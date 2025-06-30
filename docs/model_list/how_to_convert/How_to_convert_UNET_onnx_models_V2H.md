# How to convert UNET onnx models for V2H
<!-- Below is a list of AI models supported by this manual. -->
The AI models in the table below should be converted according to the following procedure and then entered into the compilation script.

| AI model    | Model name   |Input shape    | Task              |
|---------|------------|---------------|-------------------|
| [UNET](https://smp.readthedocs.io/en/latest/index.html)  |UNET_Mobilenet_v2  | (256,256)  | Segmentation  |
---

## 1. Setup Segmentation models Pytorch env..

Please prepare new docker environment to make onnx model. See the detail the guid in [Segmentation Models Pytroch(SMP) ](https://smp.readthedocs.io/en/latest/index.html). SMP provides a framework for  segmentation models training with PyTorch, supporting various architectures as backbones.

## 2. Make ONNX(.onnx) files
The following describes how to save ONNX models for benchmarking with DRP-AI TVM.   
**[NOTE]** The following step saves a untrained model as a benchmark model for inference time measurement. If you want to perform inference correctly, please follow the SMP guide to train the model.

```py
import segmentation_models_pytorch as smp
import torch

model = smp.Unet(
    encoder_name="mobilenet_v2",
    in_channels=3, 
    classes=21)

# Dummy input 256x256
tsr_din = torch.ones(1, 3, 256, 256)
# Export
torch.onnx.export(model, tsr_din, "./Unet_mobilenet_v2.onnx")
```

## 3. Next Step

To compile the models, enter the ONNX (.onnx) files into the compilation script in the tutorials.[See [tutorials](../../../tutorials/)]


----