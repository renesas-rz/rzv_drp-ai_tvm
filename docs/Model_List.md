# Model list

Below is a list of AI models that Renesas has verified for conversion with the DRP-AI TVM[^1] and actual operation on an evaluation board.
*  DRP-AI TVM[^1] Version: v1.0.0
*  Evaluation Board: RZ/V2MA EVK
*  Software Version:
    * DRP-AI Translator v1.80
    * RZ/V2MA Linux Package v1.0.0
    * RZ/V2MA DRP-AI Support Package v7.20


| AI model | Task | Format | Inference time<br>(CPU only) | Inference time<br>(CPU+DRP-AI) |
|----------|------|--------|---------------------------|-----------------------------|
| ResNet18-v1 | Classification | ONNX | 488ms | 17ms |
| ResNet18-v2 | Classification | ONNX | 487ms | 19ms |
| ResNet34-v1 | Classification | ONNX | 870ms | 27ms |
| ResNet34-v2 | Classification | ONNX | 890ms | 29ms |
| ResNet50-v1 | Classification | ONNX | 1358ms | 36ms |
| ResNet50-v2 | Classification | ONNX | 1662ms | 46ms |
| ResNet101-v1 | Classification | ONNX | 2479ms | 56ms |
| ResNet101-v2 | Classification | ONNX | 2777ms | 70ms |
| MobileNetV2 | Classification | ONNX | 224ms | 21ms |
| SqueezeNet1.1-7 | Classification | ONNX | 142ms | 8ms |
| DenseNet9 | Classification | ONNX | 1345ms | 149ms |
| Inception-v1 | Classification | ONNX | 738ms | 649ms |
| Inception-v2 | Classification | ONNX | 1165ms | 128ms |
| YOLOv2 | Object Detection | ONNX | 6688ms | 81ms |
| YOLOv3 | Object Detection | ONNX | 15507ms | 222ms |
| YOLOv5l | Object Detection | ONNX | 13575ms | 222ms |
| HRNet | Body Keypiont 2D | ONNX | 3639ms | 61ms |
| ResNet18 | Classification | PyTorch | 488ms | 18ms |
| ResNet34 | Classification | PyTorch | 897ms | 27ms |
| ResNet50 | Classification | PyTorch | 1619ms | 38ms |
| ResNet101 | Classification | PyTorch | 2760ms | 58ms |
| ResNeXt-50-32x4d | Classification | PyTorch | 2038ms | 504ms |
| MobileNetV2 | Classification | PyTorch | 226ms | 21ms |
| SqueezeNet1_1 | Classification | PyTorch | 142ms | 41ms |
| DenseNet-121 | Classification | PyTorch | 1436ms | 307ms |
| DenseNet-161 | Classification | PyTorch | 4072ms | 1172ms |
| GoogleNet | Classification | PyTorch | 758ms | 153ms |
| MnasNet0_5 | Classification | PyTorch | 102ms | 37ms |
| DeepLabv3-resnet50 | Segmentation | PyTorch | 15467ms | 172ms |
| DeepLabv3-resnet101 | Segmentation | PyTorch | 21524ms | 274ms |
| FCN_resnet101 | Segmentation | PyTorch | 18151ms | 265ms |
| DeepPose | Body Keypoint 2D | PyTorch | 2239ms | 36ms |
| HRNetV2 | Face Detection 2D | PyTorch | 1936ms | 52ms |
| HRNetV2 DarkPose | Face Detection 2D | PyTorch | 3215ms | 67ms |
---
[^1]: DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.