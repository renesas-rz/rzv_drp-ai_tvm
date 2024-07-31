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
# Description : Train CIFAR10 with TensorFlow RESNET50.
# Version     : 1.00

import argparse
import time
import tensorflow as tf
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
    Args:
        None

    Returns:
        args: Arguments from command line, including learning rate and max_epochs.
    """
    parser = argparse.ArgumentParser(description='TensorFlow CIFAR-10 Training')
    parser.add_argument('-lr', '--learning_rate', default=0.001, type=float,
                        help='learning rate')
    parser.add_argument('--max_epochs', dest='max_epochs',
                        help='number of training epochs during initial training',
                        default=20, type=int)
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

def initialize_resnet50(input_shape, num_classes, use_pretrained=True):
    """
    Initializes and returns a ResNet50 model for CIFAR-10 dataset.

    Args:
        input_shape (Tuple): The shape of the input images.
        num_classes (int): The number of classes in the dataset.
        use_pretrained (bool): If True, use pretrained weights for ResNet50.

    Returns:
        tensorflow.keras.Model: A modified ResNet50 model suitable for CIFAR-10.
    """
    print('==> Initializing model...')
    base_model = ResNet50(weights='imagenet' if use_pretrained else None,
                          include_top=False,
                          input_shape=input_shape)

    # Freeze layers of the base model
    for layer in base_model.layers:
        layer.trainable = True

    # Add custom layers for CIFAR-10 classification
    x = base_model.output
    x = layers.GlobalAveragePooling2D()(x)
    x = layers.Dense(1024, activation='relu')(x)
    predictions = layers.Dense(num_classes, activation='softmax')(x)

    model = models.Model(inputs=base_model.input, outputs=predictions)
    return model

def convert_to_onnx(model, onnx_model_path):
    """
    Converts a TensorFlow Keras model to ONNX format and saves it.

    Args:
        model (tensorflow.keras.Model): The TensorFlow Keras model to convert.
        onnx_model_path (str): Path to save the converted ONNX model file.
    """
    # Convert to ONNX
    onnx_model, _ = tf2onnx.convert.from_keras(model, opset=13)
    onnx_model_proto = onnx_model.SerializeToString()

    with open(onnx_model_path, "wb") as f:
        f.write(onnx_model_proto)
    print(f"Model converted to ONNX format and saved as '{onnx_model_path}'")

def train_model(model, x_train, y_train, x_test, y_test, learning_rate, num_epochs):
    """
    Compiles and trains the given model.

    Args:
        model (tensorflow.keras.Model): The model to train.
        x_train (numpy.ndarray): Training data images.
        y_train (numpy.ndarray): Training data labels.
        x_test (numpy.ndarray): Validation data images.
        y_test (numpy.ndarray): Validation data labels.
        learning_rate (float): Learning rate for training.
        num_epochs (int): Number of epochs to train for.

    Returns:
        History: A history object containing training and validation metrics.
    """
    print('==> Training model...')
    start_time = time.time()

    model.compile(optimizer=optimizers.SGD(learning_rate=learning_rate, momentum=0.9),
                  loss='categorical_crossentropy',
                  metrics=['accuracy'])

    history = model.fit(x_train, y_train, 
                        epochs=num_epochs,
                        batch_size=32,
                        validation_data=(x_test, y_test),
                        verbose=1)

    training_time = time.time() - start_time
    print('Training complete in {:.0f}m {:.0f}s'.format(training_time // 60, training_time % 60))

    # Save the model in .h5 format
    model.save("pretrained_resnet50.h5")
    print("Model saved in .h5 format as 'pretrained_resnet50.h5'")

    # Convert the .h5 Keras model to ONNX format
    convert_to_onnx(model, "pretrained_resnet50.onnx")
    print(f"Pretrained model has been converted to ONNX format as 'pretrained_resnet50.onnx'")

    return history

if __name__ == '__main__':
    args = parse_args()

    (x_train, y_train), (x_test, y_test) = prepare_cifar10_dataloaders()

    num_classes = 10
    input_shape = (32, 32, 3)  # CIFAR-10 images size
    model = initialize_resnet50(input_shape=input_shape, num_classes=num_classes, use_pretrained=True)

    train_model(model=model, 
                x_train=x_train, 
                y_train=y_train, 
                x_test=x_test, 
                y_test=y_test,
                learning_rate=args.learning_rate,
                num_epochs=args.max_epochs)