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
# Description : Train FOOD-101 with PyTorch RESNET50.
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

# Detect if we have a GPU available
device = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")

def parse_args():
    """
    Parse command line arguments for training settings.

    This function parses arguments related to the training process of a PyTorch model, 
    including learning rate and the number of epochs. 
    Args:
        None

    Returns:
        args: Arguments from command line, including learning rate and max_epochs.
    """
    parser = argparse.ArgumentParser(description='PyTorch FOOD-101 Training')
    parser.add_argument('-lr', '--learning_rate', default=0.001, type=float,
                        help='learning rate')
    parser.add_argument('--max_epochs', dest='max_epochs',
                        help='number of training epochs during initial training',
                        default=5, type=int)
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

def initialize_resnet50(num_classes, use_pretrained=True):
    """Initialize ResNet50

    Args:
        num_classes: Number of classes
        use_pretrained: When True, use torchvision pretrained model weight.

    Returns:
        model: torchvision ResNet50 for finetuning
    """
    print('==> Initializing model...')
    model_ft = torchvision.models.resnet50(pretrained=use_pretrained)
        
    # Modify the final layer for the number of classes 
    num_ftrs = model_ft.fc.in_features
    model_ft.fc = nn.Linear(num_ftrs, num_classes)
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
                best_model_wts = copy.deepcopy(model.state_dict())
            if phase == 'test':
                test_acc_history.append(epoch_acc)

        scheduler.step()

        print()

    time_elapsed = time.time() - since
    print('Training complete in {:.0f}m {:.0f}s'.format(
        time_elapsed // 60, time_elapsed % 60))
    print('Best test Acc: {:4f}'.format(best_acc*100))

    # Save best model weights
    torch.save(best_model_wts, "pretrained_resnet50.pth")
    # Save best model as onnx format
    model.load_state_dict(best_model_wts)
    torch.onnx.export(model,
                      torch.randn(1, 3, 224, 224, device = device),
                      "pretrained_resnet50.onnx", 
                      opset_version=13)  # opset_version must be 13.

    return test_acc_history


if __name__ == '__main__':
    # Parse command line arguments
    args = parse_args()

    # Create Food-101 dataloaders
    dataloaders = prepare_food101_dataloaders()

    # Initilize Resnet50 model
    num_classes = 101 
    model = initialize_resnet50(num_classes=num_classes, 
                                use_pretrained=True)
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