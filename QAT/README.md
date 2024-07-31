# Quantization Aware Training (QAT) Workflow

This README provides a concise overview of the Quantization Aware Training (QAT) workflow using the DRP-AI TVM[^1]. For a more detailed explanation, please refer to the complete user manual.  

Check the [Introduction](#1-introduction) part to view what Quantization Aware Training(QAT) is.  
Check the [Workflow](#2-workflow) part to view DRP-AI TVM[^1]'s QAT workflow.  
Check the [Quantize Tensorflow QAT Models](#3-quantize-tensorflow-qat-models) part to view the sample of QAT a model with TensorFlow.  
Check the [Quantize PyTorch QAT Models](#4-quantize-pytorch-qat-models) part to view the sample of QAT a model with PyTorch.  
Check the [Compile QAT models and generate INT8 deployment network](#5-compile-qat-models-and-generate-int8-deployment-network) part to view the sample command of compile and quantize the QAT models with DRP-AI TVM[^1].  

**Note: We recommend to use post-training quantization (PTQ) , and try to use QAT if a large loss of accuracy is still remained after adjusting the PTQ and quantization parameters**

## 1. Introduction

Quantization Aware Training (QAT) is a powerful technique in the field of machine learning that optimizes models to use lower-precision numerical formats.  
The DRP-AI TVM[^1] supports QAT models trained using popular frameworks like PyTorch and TensorFlow. This compatibility, however, is subject to certain conditions that ensure the integrity and performance of the quantized models.  
See [Section 3](#3-prepare-tensorflow-qat-modelsrefer-to-workflow-step1-and-step2) to prepare Tensorflow QAT models for DRP-AI TVM[^1], or see [Section 4](#4-prepare-pytorch-qat-modelsrefer-to-workflow-step1-and-step2) to prepare PyTorch QAT models for DRP-AI TVM[^1].

Here are some experimental results using quantized perception after training and performing 8bit quantization.  
Quantization aware training closes the gap to floating point accuracy.  
|          Model        | 8-bit symmetric per-channel PTQ  | 8-bit symmetric per-channel QAT  |    Floating point   |
|-----------------------|----------------------------------|----------------------------------|---------------------|
|   Mobilenet-v2_1 224  |              69.8%               |                71.1%             |        71.9%        |
|   Mobilenet-v1 1 224  |              59.1%               |                70.7%             |        70.9%        |

**The above results are referenced to the paper [Quantizing deep convolutional networks for efficient inference: A whitepaper](https://arxiv.org/pdf/1806.08342.pdf).**

## 2. Workflow

The following contents and image describe the workflow of Quantization aware training, compile and quantize the QAT models with DRP-AI TVM[^1]:  

**STEP1.** Initial training: Train a model that performs well, as QAT fine-tunes the weights to maintain accuracy after quantization.  

**STEP2.** QAT: Import the quantization module from PyTorch or TensorFlow and perform Quantization Aware Training. Convert the FP32 QAT model to ONNX format.  

**STEP3.** Compile QAT models and generate INT8 deployment network: Use DRP-AI TVM[^1] to compile and quantize the FP32 ONNX format QAT models to INT8 deployment network and generate sample input/output data.  

![](./Image/QAT_workflow.png)

## 3. Prepare Tensorflow QAT Models(Refer to Workflow STEP1 and STEP2)

### 3.1 Sample scripts introduction
We have provided two sample Python scripts in the [TensorFlow_sample](./TensorFlow_sample/) folder to demonstrate initial training and quantization aware training using TensorFlow:

1. [train.py](./TensorFlow_sample/train.py): This script is designed for fine-tuning a ResNet50 model, pre-trained on the ImageNet dataset, for the CIFAR-10 dataset. It loads the pre-trained ResNet50 model from TensorFlow Keras, adapts the final layers to align with the CIFAR-10 classes, and trains the model using the CIFAR-10 data. The script saves the trained model in both .h5 and ONNX formats.

2. [qat_retrain.py](./TensorFlow_sample/qat_retrain.py): This script extends `train.py` by incorporating Quantization Aware Training (QAT) to fine-tune the pre-trained ResNet50 model for the CIFAR-10 dataset. It loads the pre-trained model saved in .h5 format from the initial training script, applies the default [tfmot.quantization.keras.quantize_model](https://www.tensorflow.org/model_optimization/api_docs/python/tfmot/quantization/keras/quantize_model) API to make the model quantization-aware, retrains the model with QAT, and exports the quantization aware model in both .h5 and ONNX formats.  

You can refer to the following image to overview of the roles of the sample scripts:
![](./Image/tensorflow_QAT_workflow.png)

### 3.2 Use the sample scripts:

#### 3.2.1 Run `train.py`(Workflow's STEP1) 
`train.py` can perform initial training and save the pre-trained model weights in .h5 and ONNX formats.(As mentioned in [Workflow's STEP1](#2-workflow)) To execute the initial training script, use the following command in your terminal or command prompt:
```
python3 train.py -lr <int:the learning rate> --max_epochs <the learning epochs>
```
- **<learning_rate>** should be replaced with the desired learning rate value (as a floating-point number). For example, 0.001
- **<max_epochs>** should be replaced with the desired number of training epochs (as an integer). For example, 50  

This command will start the training process of the model using the specified learning rate and number of epochs. After the training is completed, two files will be generated :  
<1> **pretrained_resnet50.h5** : This file is a keras model saved in a .h5 format, which contains the trained model weights.  
<2> **pretrained_resnet50.onnx** : This file is converted from ‘pretrained_resnet50.h5’.  

#### 3.2.2 Run `qat_retrain.py`(Workflow's STEP2) 
`qat_retrain.py` can perform QAT using the pre-trained model weights from the .h5 file, and export the quantization aware model in both .h5 and ONNX formats.(As mentioned in [Workflow's STEP2](#2-workflow)) To execute the retraining script, use the following command in your terminal or command prompt:
```
python3 qat_retrain.py -lr <int:the learning rate> --max_epochs <the learning epochs>
```
- <learning_rate> should be replaced with the desired learning rate value (as a floating-point number). For example, 0.0001  
- <max_epochs> should be replaced with the desired number of training epochs (as an integer). For example, 3
#### Note: Adjust the learning rate and number of epochs for QAT accordingly. QAT typically requires fewer epochs than initial training, as it is essentially a fine-tuning process. Monitor validation performance and adjust as needed.
This command will start the quantization aware training process of the model using the specified learning rate and number of epochs. After the training is completed, two files will be generated  
<1> **QAT_resnet50_fp32.h5** : This file contains the model weights of float format trained with QAT.   
<2> **QAT_resnet50_fp32.onnx** : This file is converted from ‘QAT_resnet50_fp32.h5’. It will be used in the next section to be quantized in DRP-AI TVM[^1].

## 4. Prepare PyTorch QAT Models(Refer to Workflow STEP1 and STEP2)

### 4.1 Sample scripts introduction
We have provided two sample Python scripts in the [PyTorch_sample](./PyTorch_sample/) folder to demonstrate initial training and quantization aware training using PyTorch:

1. [train.py](./PyTorch_sample/train.py): This script is designed for fine-tuning a ResNet50 model, pre-trained on the ImageNet dataset, for the FOOD-101 dataset. It loads the pre-trained ResNet50 model from PyTorch TorchVision, adapts the final layers to align with the FOOD-101 classes, and trains the model using the FOOD-101 data. The script saves the trained model in both .pth and ONNX formats.

2. [qat_retrain.py](./PyTorch_sample/qat_retrain.py): This script extends `train.py` by incorporating Quantization Aware Training (QAT) to fine-tune the pre-trained ResNet50 model for the FOOD-101 dataset. It loads the pre-trained model saved in .pth format from the initial training script, applies QAT settings using specific quantization configurations for activations and weights to make the model quantization-aware, retrains the model with QAT, and exports the quantization aware model in both .pth and ONNX formats.  

You can refer to the following image to overview of the roles of the sample scripts:
![](./Image/pytorch_QAT_workflow.png)

### 4.2 Use the sample scripts:

#### 4.2.1 Run `train.py`(Workflow's STEP1) 
`train.py` can perform initial training and save the pre-trained model weights in .pth and ONNX formats.(As mentioned in [Workflow's STEP1](#2-workflow)) To execute the initial training script, use the following command in your terminal or command prompt:
```
python3 train.py -lr <int:the learning rate> --max_epochs <the learning epochs>
```
- **<learning_rate>** should be replaced with the desired learning rate value (as a floating-point number). For example, 0.001
- **<max_epochs>** should be replaced with the desired number of training epochs (as an integer). For example, 50  

This command will start the training process of the model using the specified learning rate and number of epochs. After the training is completed, two files will be generated :  
<1> **pretrained_resnet50.pth** : This file is a pytorch model saved in a .pth format, which contains the trained model weights.  
<2> **pretrained_resnet50.onnx** : This file is converted from ‘pretrained_resnet50.pth’.  

#### 4.2.2 Run `qat_retrain.py`(Workflow's STEP2) 
`qat_retrain.py` can perform QAT using the pre-trained model weights from the .pth file, and export the quantization aware model in both .pth and ONNX formats.(As mentioned in [Workflow's STEP2](#2-workflow)) To execute the retraining script, use the following command in your terminal or command prompt:
```
python3 qat_retrain.py -lr <int:the learning rate> --max_epochs <the learning epochs>
```
- **<learning_rate>** should be replaced with the desired learning rate value (as a floating-point number). For example, 0.0001  
- **<max_epochs>** should be replaced with the desired number of training epochs (as an integer). For example, 3
#### Note: Adjust the learning rate and number of epochs for QAT accordingly. QAT typically requires fewer epochs than initial training, as it is essentially a fine-tuning process. Monitor validation performance and adjust as needed.
This command will start the quantization aware training process of the model using the specified learning rate and number of epochs. After the training is completed, two files will be generated  
<1> **QAT_resnet50_fp32.pth** : This file contains the model weights of float format trained with QAT.   
<2> **QAT_resnet50_fp32.onnx** : This file is converted from ‘QAT_resnet50_fp32.pth’. It will be used in the next section to be quantized in DRP-AI TVM[^1].

## 5. Compile and quantize QAT models(Refer to Workflow STEP3)

The last step in the quantization process is using DRP-AI TVM[^1] to compile the QAT models obtained from qat_retrain.py and perform int8 quantization on the compiled QAT ONNX models.(As mentioned in [Workflow's STEP3](#2-workflow))  
DRP-AI TVM[^1] offers the `--qat` or the `-a` command line options for processing QAT models and offers the `--qat_type` or `-m` commandline option for specifying the QAT models' source framework :

- If assign the command line option `--qat` or `-a` with the value `pytorch`,  The DRP-AI TVM[^1] will preprocess the QAT models fine-tuned with PyTorch before compile and quantization.  
- If assign the command line option `--qat` or `-a` with the value `tensorflow`,  The DRP-AI TVM[^1] will preprocess the QAT models fine-tuned with Tensorflow before compile and quantization.

**Note 1: The `--qat` or `-a` commandline option should be used with the commandline option `--qat_type` or `-m` , or it will cause parameter passing errors**  
**Note 2: The `-m` commandline option should be passed as key-value format, or it will cause parameter passing errors**  

When these commandline options has been passed, the quantization preprocessing transforms the input QAT model to let it can be comppiled and quantized by DRP-AI TVM[^1]. The following sample usage demonstrates how to compile a QAT model using the output onnx files from the sample scripts 'qat_retrain.py'.  

Sample command line when compile a QAT model which fine_tuned with PyTorch: 

```
python  compile_onnx_model_quant.py ¥                       # Execute the compile script
        <path-to>/QAT_resnet50_fp32.onnx                    # Input: FLOAT format QAT ONNX file
        -d <path-to>/drpai_quant_translator ¥               # Specify DRP-AI Translator root directory
        -q <path-to>/drpai_quant_converter ¥                # Specify Quantization tool directory
        -p "quantizer options" ¥                            # Adding Command Line Options for Quantization tool, example -p "az True"
        -v 091 ¥                                            # Specify DRP-AI Translator version (091 or 100)
        -a ¥                                                # Specify to compile QAT models
        -m pytorch                                          # Specify the QAT models' source framework as PyTorch
```  
Sample command line when compile a QAT model which fine_tuned with TensorFlow:  
```
python  compile_onnx_model_quant.py ¥                       # Execute the compile script
        <path-to>/QAT_resnet50_fp32.onnx                    # Input: FLOAT format QAT ONNX file
        -d <path-to>/drpai_quant_translator ¥               # Specify DRP-AI Translator root directory
        -q <path-to>/drpai_quant_converter ¥                # Specify Quantization tool directory
        -p "quantizer options" ¥                            # Adding Command Line Options for Quantization tool, example -p "az True"
        -v 091 ¥                                            # Specify DRP-AI Translator version (091 or 100)
        -a ¥                                                # Specify to compile QAT models
        -m Tensorflow                                       # Specify the QAT models' source framework as Tensorflow
```  

[^1]: DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.
