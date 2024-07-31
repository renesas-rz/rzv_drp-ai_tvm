[<- Overview](./../README.md)  
### What is Pruning and Quantization?

### Pruning

Nodes are interconnected in a neural network as shown in the figure below. Methods of reducing the number of parameters by removing weights between nodes or removing nodes are referred to as “pruning”. A neural network to which pruning has not been applied is generally referred to as a dense neural network.
Applying pruning to a neural network lead to a slight deterioration in the accuracy of the model but can reduce the power required by hardware and accelerate the inference process.

<img src=./../img/pruning_desc.png width=600 alt="Schematic View of the Pruning of a Neural Network">

**Note:** In the use of pruning tool for DRP-AI, we recommend pruning by at least 70% to improve the processing performance of the DRP-AI.

### Quantization

Typically, AI models use floating-point arithmetic, which can be computationally intensive and memory-heavy. Quantization converts these floating-point numbers into integers, which can significantly decrease the model size and improve computational efficiency. By converting high-precision (e.g., 32-bit floating point) numbers to lower-precision (e.g., 8-bit integer) representations, quantization enables faster and more efficient inference on hardware with limited computational resources. In the implementaion of quantization, There are two common types of quantization methods: Post-Training Quantization (PTQ) and Quantization-Aware Training (QAT).

<img src=./../img/quantization_desc.png width=600 alt="Schematic View of the Effect of Quantization">

#### Post-Training Quantization (PTQ)

Post-Training Quantization (PTQ) involves converting a pre-trained model from 32-bit floating point format to 8-bit integer format after the training process is completed. This method does not require retraining the model but may result in a little bit loss of accuracy. PTQ is generally easier and faster to apply. It is suitable for most cases in which a little accuracy loss is acceptable. This trade-off results in a much smaller model size and faster processing. We recommend prioritizing the use of this method to implement quantization to the model. <br>
For how to compile and quantize the onnx model with PTQ, Please refer to [compile and PTQ onnx models](./../tutorials/tutorial_RZV2H.md) 

#### Quantization-Aware Training (QAT)

Quantization-Aware Training (QAT) is a more complex quantization technique where the model is trained with quantization in mind. As a result, QAT may produce models that achieve higher accuracy compared to PTQ, even when quantized to 8 bit. However, QAT requires more computational resources and time during training since it involves modifying the training process to account for quantization effects. Therefore, we recommend using QAT only after experiencing a significant accuracy loss with PTQ and being unable to restore the accuracy loss.<br>
For how to compile and quantize the onnx model with QAT, Please refer to [compile and QAT onnx models](./../QAT/README.md) 


[Installing DRP-AI Extention Package ->](./setup/README.md)