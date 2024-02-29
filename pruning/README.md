[<- Overview](./../README.md)  
### What is Pruning and Quantization?
Nodes are interconnected in a neural network as shown in the figure below. Methods of reducing the number of parameters by removing weights between nodes or removing nodes are referred to as “pruning”. A neural network to which pruning has not been applied is generally referred to as a dense neural network.
Applying pruning to a neural network lead to a slight deterioration in the accuracy of the model but can reduce the power required by hardware and accelerate the inference process.

<img src=./../img/pruning_desc.png width=600 alt="Schematic View of the Pruning of a Neural Network">

**Note:** In the use of pruning tool for DRP-AI, we recommend pruning by at least 70% to improve the processing performance of the DRP-AI.

Quantization is the process of reducing the sizes of models by representing parameters of networks such as weights with a lower bit width.

<img src=./../img/quantization_desc.png width=600 alt="Schematic View of the Effect of Quantization">


[Installing DRP-AI Extention Package ->](./setup/README.md)