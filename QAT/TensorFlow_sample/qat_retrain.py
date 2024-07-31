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
# Description : Implement QAT on CIFAR10 with trained TensorFlow RESNET50.
# Version     : 1.00

import argparse
import time
import tensorflow as tf
# Import the tfmot from TensorFlow for model quantization
import tensorflow_model_optimization as tfmot
import tf2onnx
from tensorflow.keras import layers, models, optimizers, losses, callbacks
from tensorflow.keras.datasets import cifar10
from tensorflow.keras.applications import ResNet50
from tensorflow.keras.utils import to_categorical

# Check and display GPU availability for training
print("Num GPUs Available: ", len(tf.config.experimental.list_physical_devices('GPU')))
if tf.test.is_gpu_available():
    print("Using GPU for training")
else:
    print("Using CPU for training")

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
      Too many epochs might lead to overfitting, especially in QAT. 
      It's recommended to monitor validation performance and adjust accordingly.
    - The optimal number of epochs can vary based on the specific model and dataset.
    - It is recommend that 3-5 epochs of training are sufficient at QAT, with the learning rate adjusted to 
    one-fifth to one-tenth of the initial training.

    Args:
        None

    Returns:
        args: Arguments from command line, including learning rate and max_epochs.
    """
    parser = argparse.ArgumentParser(description='TensorFlow CIFAR-10 Training')
    parser.add_argument('-lr', '--learning_rate', default=0.0001, type=float,
                        help='learning rate')
    parser.add_argument('--max_epochs', dest='max_epochs',
                        help='number of training epochs during initial training',
                        default=2, type=int)
    args = parser.parse_args()
    return args

def prepare_cifar10_dataloaders():
    """
    Prepares and returns CIFAR-10 dataloaders for training and testing.

    The function loads the CIFAR-10 dataset, normalizes the images, and converts
    the labels to one-hot encoded format.

    Returns:
        Tuple: A tuple containing training and testing data loaders.
    """
    print('==> Preparing data loader...')
    (x_train, y_train), (x_test, y_test) = cifar10.load_data()
    
    x_train = x_train.astype("float32") / 255
    x_test = x_test.astype("float32") / 255

    y_train = to_categorical(y_train, 10)
    y_test = to_categorical(y_test, 10)

    return (x_train, y_train), (x_test, y_test)

def initialize_resnet50(pretrained_model_path):
    """
    Loads the pretrained ResNet50 model from an H5 file and makes it quantization-aware.

    In QAT, it is no need to reinitialize the model but load a pretrained model.
    This function is specifically for loading the pretrained model and applying
    Quantization Aware Training (QAT) to it. It is assumed that the model was
    previously saved in an H5 file after being trained on a dataset like CIFAR-10.

    Args:
        pretrained_model_path (str): Path to the .h5 file containing the pretrained model.

    Returns:
        tensorflow.keras.Model: A quantization-aware model ready for QAT.
    """
    print('==> Loading pretrained model and applying quantization-aware training...')
    
    # Load the pretrained model from H5 file
    pretrained_model = tf.keras.models.load_model(pretrained_model_path)
    print("Pretrained Model loaded.")

    # Apply quantization-aware training to the model
    quantized_model = tfmot.quantization.keras.quantize_model(pretrained_model)
    print("Quantization applied to the model")

    return quantized_model

def convert_to_onnx(model, onnx_model_path):
    """
    Convert a given TensorFlow model to ONNX format.

    Args:
        model (tensorflow.keras.Model): The model to convert to ONNX.
        onnx_model_path (str): File path to save the ONNX model.

    Note: 
    - The function uses tfmot.quantization.keras.quantize_scope() to ensure
      the quantization parameters are correctly set during the conversion.
    """
    # Begin a new scope for quantization-aware operations.
    # 'tfmot.quantization.keras.quantize_scope()' ensures that any custom objects related to 
    # TensorFlow Model Optimization Toolkit (TF-MOT) are recognized during model loading.
    with tfmot.quantization.keras.quantize_scope():
        # Convert to ONNX
        onnx_model, _ = tf2onnx.convert.from_keras(model, opset=13)
        onnx_model_proto = onnx_model.SerializeToString()

    with open(onnx_model_path, "wb") as f:
        f.write(onnx_model_proto)
    print(f"Quantization aware model converted to ONNX format and saved as '{onnx_model_path}'")

def retrain_model(model, x_train, y_train, x_test, y_test, learning_rate, num_epochs):
    """
    Retrain the given model, save it in .h5 format, and convert it to ONNX format.

    Args:
        model (tensorflow.keras.Model): The model to be retrained.
        x_train (numpy.ndarray): Training data images.
        y_train (numpy.ndarray): Training data labels.
        x_test (numpy.ndarray): Validation data images.
        y_test (numpy.ndarray): Validation data labels.
        learning_rate (float): Learning rate for training.
        num_epochs (int): Number of epochs to train for.
        h5_model_path (str): Path to save the .h5 model file.
        onnx_model_path (str): Path to save the ONNX model file.

    The function retrains the model using the provided dataset, saves the retrained
    model in .h5 format, and then converts it to ONNX format.
    """
    print('==> Retraining quantization-aware model...')
    start_time = time.time()

    # Compile and retrain the model
    model.compile(optimizer=optimizers.SGD(learning_rate=learning_rate, momentum=0.9),
                  loss='categorical_crossentropy',
                  metrics=['accuracy'])

    history = model.fit(x_train, y_train, 
                        epochs=num_epochs,
                        batch_size=32,
                        validation_data=(x_test, y_test),
                        verbose=1)

    training_time = time.time() - start_time
    print('Retraining complete in {:.0f}m {:.0f}s'.format(training_time // 60, training_time % 60))

    # Save the retrained quantization-aware model in .h5 format
    model.save("QAT_resnet50.h5")
    print(f"Quantization-aware model saved in .h5 format as 'QAT_resnet50.h5'")

    # Convert the retrained quantization-aware model to ONNX
    convert_to_onnx(model, "QAT_resnet50.onnx")
    print(f"Model retrained and converted to ONNX format as 'QAT_resnet50.onnx'")

    return history

if __name__ == '__main__':
    args = parse_args()

    # Load data
    (x_train, y_train), (x_test, y_test) = prepare_cifar10_dataloaders()

    # Load the pretrained model and Apply the quantization aware to the pretrained model
    qat_model = initialize_resnet50("pretrained_resnet50.h5")

    # Recompile, retrain, and save the quantization-aware model, then convert it to ONNX format
    retrain_model(model=qat_model, 
                  x_train=x_train, 
                  y_train=y_train, 
                  x_test=x_test, 
                  y_test=y_test, 
                  learning_rate=args.learning_rate, 
                  num_epochs=args.max_epochs)
