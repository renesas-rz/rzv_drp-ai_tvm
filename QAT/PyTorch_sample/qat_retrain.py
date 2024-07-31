#######################################################################################################################
# DISCLAIMER
# This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No
# other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
# applicable laws, including copyright laws.
# THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
# THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM
# EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES
# SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO
# THIS SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
# Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of
# this software. By using this software, you agree to the additional terms and conditions found by accessing the
# following link:
# http://www.renesas.com/disclaimer
#
# Copyright (C) 2024 Renesas Electronics Corporation. All rights reserved.
#######################################################################################################################
# Copyright 2015 The TensorFlow Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# =============================================================================
# Description : Implement QAT on FOOD-101 with trained PyTorch RESNET50.
# Version     : 1.00

import argparse
import copy
import time

import torch
import torch.nn as nn
import torch.optim as optim
import torchvision
import torchvision.transforms as transforms
import torchvision.datasets as datasets
from torch.utils.data import DataLoader
# Import the quantization module from PyTorch for model quantization
import torch.quantization

# Detect if we have a GPU available
device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")

def parse_args():
    """
    Parse command line arguments for training settings.

    This function parses arguments related to the training process of a PyTorch model, 
    including learning rate and the number of epochs. 
    For Quantization Aware Training (QAT), 
    the number of epochs is crucial as it determines the extent of training with quantization 
    simulation. Setting an appropriate number of epochs for QAT is important to ensure the 
    model adequately learns quantization effects without overfitting.

    Note:
    - QAT typically requires less epochs than initial training since QAT is essentially a fine-tuning process.
      Too many epochs might lead to overfitting, especially in a QAT context. 
      It's recommended to monitor validation performance and adjust accordingly.
    - The optimal number of epochs can vary based on the specific model and dataset.
    - It is recommend that 3-5 epochs of training are sufficient at QAT, with the learning rate adjusted to 
    one-fifth to one-tenth of the initial training.

    Args:
        None

    Returns:
        args: Arguments from command line, including learning rate and max_epochs.
    """
    parser = argparse.ArgumentParser(description='PyTorch FOOD-101 Training')
    parser.add_argument('-lr', '--learning_rate', default=0.0001, type=float,
                        help='learning rate')
    parser.add_argument('--max_epochs', dest='max_epochs',
                        help='number of training epochs during QAT',
                        default=2, type=int)
    args = parser.parse_args()
    return args

def prepare_food101_dataloaders(data_root_path="./data"):
    """Prepare Food-101 dataloader for training and testing

    Args:
        data_root_path: Path to store/download Food-101 dataset.

    Returns:
        dict: Dataloader dictionary with `train` and `test` keys.
    """
    print('==> Preparing data loader...')
    transform = transforms.Compose([
        transforms.Resize(256),
        transforms.CenterCrop(224),
        transforms.ToTensor(),
        transforms.Normalize(mean=[0.485, 0.456, 0.406],                              
                             std=[0.229, 0.224, 0.225]),
    ])

    trainset = datasets.Food101(root=data_root_path, split='train', download=True, transform=transform)
    trainloader = DataLoader(trainset, batch_size=32, shuffle=True, num_workers=4)

    testset = datasets.Food101(root=data_root_path, split='test', download=True, transform=transform)
    testloader = DataLoader(testset, batch_size=32, shuffle=False, num_workers=4)

    return {"train": trainloader, "test": testloader}

def initialize_resnet50(num_classes, use_pretrained=True, pretrained_path=None):
    """Initialize ResNet50

    Args:
        num_classes: Number of classes
        use_pretrained: When True, use torchvision pretrained model weight.
        pretrained_path: Path to the pretrained model weights.

    Returns:
        model: torchvision ResNet50 for finetuning
    """
    print('==> Initializing model for QAT...')
    model_ft = torchvision.models.resnet50(pretrained=use_pretrained)

    # Modify the final layer for the number of classes   
    num_ftrs = model_ft.fc.in_features
    model_ft.fc = nn.Linear(num_ftrs, num_classes)
    # Load the pre-trained weights
    if pretrained_path is not None:
        model_ft.load_state_dict(torch.load(pretrained_path))

    # Prepare the model for Quantization Aware Training (QAT)
    # It is crucial to set the quantization configuration as follows to simulate the 
    # quantization specification of DRP-AI_Quantizer. These settings are essential 
    # for ensuring the model accurately simulates quantization effects during QAT.
    # Improper configuration may lead to errors or suboptimal quantization results.

    # Activation Quantization Configuration must be set as follows so that it can aligning with drp-ai_quantizer specifications:
    # - Uses Fake Quantization to simulate quantization effects during training.
    # - The observer must be set as 'MovingAverageMinMaxObserver'.
    # - The 'quant_min' and 'quant_max' must be set as 0 and 255.
    # - 'dtype' must be set as 'torch.quint8' to represent unsigned 8-bit integers.
    # - 'qscheme' must be set as 'per_tensor_affine' for scale and zero_point settings per tensor.
    # - 'reduce_range' must be set as True to reduce the quantization range by 1 bit.
    qact = torch.quantization.FakeQuantize.with_args(observer=torch.quantization.MovingAverageMinMaxObserver, \
                                                    quant_min=0, quant_max=255, dtype=torch.quint8, qscheme=torch.per_channel_symmetric, reduce_range=True)

    # Weight Quantization Configuration must be set as follows so that it can aligning with drp-ai_quantizer specifications:
    # - Also uses Fake Quantization for weight quantization simulation.
    # - The observer must be set as 'MovingAveragePerChannelMinMaxObserver'.
    # - The 'quant_min' and 'quant_max' must be set as -128 and 127.
    # - 'dtype' must be set as 'torch.qint8' for signed 8-bit integers.
    # - 'qscheme' must be set as 'per_channel_symmetric', applying symmetric quantization per channel.
    # - 'ch_axis' must be set as 0 in order to indicate the channel is the first dimension in the weight tensor.
    # - 'reduce_range' must be set as False.
    qwe = torch.quantization.FakeQuantize.with_args(observer=torch.quantization.MovingAveragePerChannelMinMaxObserver, \
                                                    quant_min=-128, quant_max=127, dtype=torch.qint8, qscheme=torch.per_channel_symmetric, reduce_range=False, ch_axis=0)
    
    # Create a quantization configuration (QConfig) with the defined activation and weight quantization settings.
    qat_config = torch.quantization.QConfig(activation=qact,weight=qwe)
    model_ft.qconfig = qat_config
    # Prepare the model for quantization-aware training (QAT).
    # This step modifies the model to simulate quantization during training.
    # 'inplace=True' modifies the model in place without creating a copy.
    model_ft = torch.quantization.prepare_qat(model_ft, inplace=True)
    
    return model_ft

def train_model(model, dataloaders, criterion, optimizer, scheduler, num_epochs=25):
    """Train the model

    Args:
        model: PyTorch model.
        dataloaders: Dataloader dictionary. This has `train` and `test` keys.
        Each key has the following data loaders.
        - `train`: training data loader.
        - `test`: test data loader.
        criterion: The criterion used to compute the loss for training.
        optimizer: Optimizer for training the model.
        scheduler: Scheduler to adjust the learning rate.
        num_epochs: Number of training epochs

    Returns:
        test_acc_history: The accuracy list for each epoch.
    """
    print('==> Training model...')
    since = time.time()

    test_acc_history = []

    best_model_wts = copy.deepcopy(model.state_dict())
    best_acc = 0.0

    for epoch in range(num_epochs):
        print('Epoch {}/{}'.format(epoch, num_epochs - 1))
        print('-' * 10)

        # Each epoch has a training and validation phase
        for phase in ['train', 'test']:
            if phase == 'train':
                model.train()  # Set model to training mode
            else:
                model.eval()   # Set model to evaluate mode

            running_loss = 0.0
            running_corrects = 0

            # Iterate over data.
            for inputs, labels in dataloaders[phase]:
                inputs = inputs.to(device)
                labels = labels.to(device)

                # zero the parameter gradients
                optimizer.zero_grad()

                # forward
                # track history if only in train
                with torch.set_grad_enabled(phase == 'train'):
                    # Get model outputs and calculate loss
                    outputs = model(inputs)
                    loss = criterion(outputs, labels)

                    _, preds = torch.max(outputs, 1)

                    # backward + optimize only if in training phase
                    if phase == 'train':
                        loss.backward()
                        optimizer.step()

                # statistics
                running_loss += loss.item() * inputs.size(0)
                running_corrects += torch.sum(preds == labels.data)

            epoch_loss = running_loss / len(dataloaders[phase].dataset)
            epoch_acc = running_corrects.double(
            ) / len(dataloaders[phase].dataset)

            print('{} Loss: {:.4f} Acc: {:.4f}'.format(
                phase, epoch_loss, epoch_acc*100))

            # deep copy the model
            if phase == 'test' and epoch_acc > best_acc:
                best_acc = epoch_acc
                # Prepare the best model for export as onnx format
                # Set model to evaluation mode
                print('Switching best model to evaluation mode and disabling quantization observers.')
                # Create a copy of the current model for export
                copied_model = copy.deepcopy(model) 
                # Disable the observer modules in the quantized model.
                # Observers are used during training to gather information about the distributions of the model's weights and activations.
                # Disabling them is necessary before exporting the model as they are not needed during inference or exported them into onnx format.
                copied_model.eval()
                copied_model.apply(torch.quantization.disable_observer)  
                best_model_wts = copy.deepcopy(copied_model.state_dict())
            if phase == 'test':
                test_acc_history.append(epoch_acc)

        scheduler.step()

        print()

    time_elapsed = time.time() - since
    print('Training complete in {:.0f}m {:.0f}s'.format(
        time_elapsed // 60, time_elapsed % 60))
    print('Best test Acc: {:4f}'.format(best_acc*100))

    # Load the best model weights
    print("Loading best model weights for ONNX export.")
    model.load_state_dict(best_model_wts)
    model.eval()  
    # Move model to CPU
    model.cpu()  


    # Save the retrained QAT FP32 model
    torch.save(model.state_dict(), "QAT_resnet50_fp32.pth")
    # Export to ONNX
    print('Exporting model to ONNX format.')
    dummy_input = torch.randn(1, 3, 224, 224)
    torch.onnx.export(model, 
                      dummy_input,
                      "QAT_resnet50_fp32.onnx", 
                      opset_version=13)  # opset_version must be 13.

    return test_acc_history


if __name__ == '__main__':
    # Parse command line arguments
    args = parse_args()

    # Create Food-101 dataloaders
    dataloaders = prepare_food101_dataloaders()

    # Initialize ResNet50 model for QAT
    num_classes = 101 
    pretrained_path = './pretrained_resnet50.pth'
    model = initialize_resnet50(num_classes=num_classes, 
                                use_pretrained=True,
                                pretrained_path=pretrained_path)
    model.to(device)

    # Setup the loss function and create the optimizer and the scheduler
    criterion = nn.CrossEntropyLoss()
    optimizer = optim.SGD(model.parameters(),
                          lr=args.learning_rate,
                          momentum=0.9)
    scheduler = torch.optim.lr_scheduler.CosineAnnealingLR(
        optimizer, T_max=200)

    # Train model
    train_model(model=model,
                dataloaders=dataloaders,
                criterion=criterion,
                optimizer=optimizer,
                scheduler=scheduler,
                num_epochs=args.max_epochs)