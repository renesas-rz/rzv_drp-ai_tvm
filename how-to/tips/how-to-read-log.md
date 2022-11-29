# How to read the Compile log

This page explains about tips to know more about the compiled AI model from the console log.
To see how to compile the model, please refer to [Tutorial](../../tutorials).

## Necessary size of DRP-AI memory area
DRP-AI memory area is contiguous memory area which is used to store necessary data for running DRP-AI.  
If DRP-AI TVM[^1] assigned the processing to DRP-AI, it will use this DRP-AI memory area.   
The size of DRP-AI memory area is defined by RZ/V2x Linux Package.  
If the AI model needs larger size than the memory area size, DRP-AI TVM[^1] inference cannot be executed.

To check the size of DRP-AI memory area, please refer to RZ/V2x Linux Package and DRP-AI Support Package.

The necessary size of DRP-AI memory area for a model can be checked in console log when you compiled the model.  
It is shown as `required drp-ai memory` at the bottom of the log.  
For example, below is the log when compiled the ResNet18.
```sh
At sub graph name tvmgen_default_tvmgen_default_mera_drp_0, required drp-ai memory: 34(Mbyte)
[TVM compile finished]
   Please check resnet18_onnx directory
```
In this case, the model uses **34Mbyte** of DRP-AI memory area.

Note that `required drp-ai memory` may be repeated throughout the log since it is generated for every sub-graph assigned to DRP-AI.  
The bottommost value is the necessary size for the entire processing in the model.

[^1]: DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.
