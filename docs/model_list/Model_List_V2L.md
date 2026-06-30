# Model list for RZ/V2L

---

Below is a list of AI models that Renesas has verified for conversion with the RUHMI[^1] and actual operation on an evaluation board.

| Item                   |RZ/V2L     |
|------------------------|-----------|
| RUHMI[^1] AI compiler  |R2026-06   |
| Evaluation Board       |RZ/V2L EVK |
| DRP-AI Translator      |v1.90      |
| RZ/V2L AI SDK          |v7.0.0     |

**[NOTE]** This benchmark shows the results when only AI inference is running. When system operations such as camera input and display output are performed simultaneously, they may affect the inference time.  


## **Representative models**

| AI model | Input Shape | inferences/s | Inference time<br>(ms) | Task | Params<br>(M) |
|:--|--:|--:|--:|:--|--:|
|[YOLOv3](./how_to_convert/How_to_convert_yolov3_onnx_model.md)|416,416|3.2|311.6|Object Detection|61.9|
|[YOLOv5n](./how_to_convert/How_to_convert_yolov5_onnx_models.md)|640,640|3.3|306.8|Object Detection|1.9|
|[YOLOv5s](./how_to_convert/How_to_convert_yolov5_onnx_models.md)|640,640|1.7|587.1|Object Detection|7.3|
|[YOLOv8n](./how_to_convert/How_to_convert_yolov8_onnx_models.md)|640,640|4.8|206.7|Object Detection|3.1|
|[YOLOX_s](./how_to_convert/How_to_convert_yolox_onnx_models.md)|640,640|5.6|178.0|Object Detection|9.0|
|[MobileNetV2](./how_to_convert/How_to_download_ONNX_models.md)|224,224|43.8|22.9|Classification|3.5|
|[ResNet50-v1](./how_to_convert/How_to_download_ONNX_models.md)|224,224|17.4|57.6|Classification|25.6|
|[HRNet](./how_to_convert/How_to_convert_hrnet_onnx_model.md)|256,192|9.5|105.4|Pose Estimation|28.5|

---

## **List of models for each task**

Please click on a task name to view its performance

<details><summary><u>Object Detection</u></summary>

| AI model | Input Shape | inferences/s | Inference time<br>(ms) | Params<br>(M) |
|:--|--:|--:|--:|--:|
|[EdgeYOLO_Tiny](./how_to_convert/How_to_convert_edgeyolo_models_onnx.md)|640,640|2.8|354.8|5.5|
|[EdgeYOLO_S](./how_to_convert/How_to_convert_edgeyolo_models_onnx.md)|640,640|2.2|462.1|9.3|
|[EdgeYOLO_M](./how_to_convert/How_to_convert_edgeyolo_models_onnx.md)|640,640|2.5|403.7|17.8|
|[YOLOv2](./how_to_convert/How_to_download_ONNX_models.md)|416,416|8.2|121.5|51.0|
|[YOLOv3](./how_to_convert/How_to_convert_yolov3_onnx_model.md)|416,416|3.2|311.6|61.9|
|[YOLOv5n](./how_to_convert/How_to_convert_yolov5_onnx_models.md)|640,640|3.3|306.8|1.9|
|[YOLOv5s](./how_to_convert/How_to_convert_yolov5_onnx_models.md)|640,640|1.7|587.1|7.3|
|[YOLOv6n](./how_to_convert/How_to_convert_yolov6_onnx_models.md)|640,640|2.4|408.8|4.7|
|[YOLOv6s](./how_to_convert/How_to_convert_yolov6_onnx_models.md)|640,640|0.7|1486.4|18.5|
|[YOLOv7](./how_to_convert/How_to_convert_yolov7_onnx_models.md)|640,640|1.9|513.8|36.9|
|[YOLOv7-w6](./how_to_convert/How_to_convert_yolov7_onnx_models.md)|640,640|2.3|432.3|70.4|
|[YOLOv7x](./how_to_convert/How_to_convert_yolov7_onnx_models.md)|640,640|1.1|878.3|71.3|
|[YOLOv7-e6](./how_to_convert/How_to_convert_yolov7_onnx_models.md)|640,640|1.5|662.2|97.2|
|[YOLOv8n](./how_to_convert/How_to_convert_yolov8_onnx_models.md)|640,640|4.8|206.7|3.1|
|[YOLOv8s](./how_to_convert/How_to_convert_yolov8_onnx_models.md)|640,640|2.6|379.5|11.2|
|[YOLOv8m](./how_to_convert/How_to_convert_yolov8_onnx_models.md)|640,640|1.3|742.7|25.9|
|[YOLOX_s](./how_to_convert/How_to_convert_yolox_onnx_models.md)|640,640|5.6|178.0|9.0|
|[YOLOX_m](./how_to_convert/How_to_convert_yolox_onnx_models.md)|640,640|2.6|387.0|25.3|
|[YOLOX_l](./how_to_convert/How_to_convert_yolox_onnx_models.md)|640,640|1.5|671.3|54.2|

</details>

<details><summary><u>Classification</u></summary>

| AI model | Input Shape | inferences/s | Inference time<br>(ms) | Params<br>(M) |   
|:--|--:|--:|--:|--:|
|[DenseNet9](./how_to_convert/How_to_download_ONNX_models.md)|224,224|5.7|176.8|8.2|
|[Inception-v1](./how_to_convert/How_to_download_ONNX_models.md)|224,224|1.7|589.7|7.0|
|[Inception-v2](./how_to_convert/How_to_download_ONNX_models.md)|224,224|9.3|108.0|11.2|
|[MobileNetV2](./how_to_convert/How_to_download_ONNX_models.md)|224,224|43.8|22.9|3.5|
|[ResNet18-v1](./how_to_convert/How_to_download_ONNX_models.md)|224,224|41.0|24.4|11.7|
|[ResNet34-v1](./how_to_convert/How_to_download_ONNX_models.md)|224,224|24.9|40.2|21.8|
|[ResNet50-v1](./how_to_convert/How_to_download_ONNX_models.md)|224,224|17.4|57.6|25.6|
|[ResNet101-v1](./how_to_convert/How_to_download_ONNX_models.md)|224,224|10.6|94.2|44.7|
|[ResNet18-v2](./how_to_convert/How_to_download_ONNX_models.md)|224,224|37.7|26.5|11.7|
|[ResNet34-v2](./how_to_convert/How_to_download_ONNX_models.md)|224,224|23.1|43.2|21.8|
|[ResNet50-v2](./how_to_convert/How_to_download_ONNX_models.md)|224,224|14.5|68.8|25.6|
|[ResNet101-v2](./how_to_convert/How_to_download_ONNX_models.md)|224,224|9.1|110.3|44.6|
|[SqueezeNet1.1-7](./how_to_convert/How_to_download_ONNX_models.md)|224,224|104.8|9.5|1.2|
|[DenseNet-121](./how_to_convert/How_to_convert_torchvision_models.md)|224,224|4.5|223.5|8.0|
|[GoogleNet](./how_to_convert/How_to_convert_torchvision_models.md)|224,224|4.6|215.2|6.6|
|[MnasNet0_5](./how_to_convert/How_to_convert_torchvision_models.md)|224,224|24.0|41.7|2.2|
|[ResNeXt-50-32x4d](./how_to_convert/How_to_convert_torchvision_models.md)|224,224|2.1|482.8|25.0|
|[SqueezeNet1_1](./how_to_convert/How_to_convert_torchvision_models.md)|224,224|16.8|59.5|1.2|

</details>

<details><summary><u>Pose Estimation</u></summary>

| AI model | Input Shape | inferences/s | Inference time<br>(ms) | Params<br>(M) |   
|:--|--:|--:|--:|--:|
|[HRNet](./how_to_convert/How_to_convert_hrnet_onnx_model.md)|256,192|9.5|105.4|28.5|
|[DeepPose](./how_to_convert/How_to_convert_mmpose_models.md)|256,192|17.3|57.8|23.6|
|[HRNetV2](./how_to_convert/How_to_convert_mmpose_models.md)|256,192|14.2|70.5|9.7|
|[HRNetV2 DarkPose](./how_to_convert/How_to_convert_mmpose_models.md)|256,192|9.0|111.2|9.7|

</details>

<details><summary><u>Semantic Segmentation</u></summary>

| AI model | Input Shape | inferences/s | Inference time<br>(ms) | Params<br>(M) |   
|:--|--:|--:|--:|--:|
|[DeepLabv3-resnet50](./how_to_convert/How_to_convert_torchvision_models.md)|224,224|3.2|312.2|42.0|
|[DeepLabv3-resnet101](./how_to_convert/How_to_convert_torchvision_models.md)|224,224|1.9|514.2|61.0|
|[FCN_resnet101](./how_to_convert/How_to_convert_torchvision_models.md)|224,224|2.0|495.4|54.3|

</details>

<details><summary><u>Depth Estimation</u></summary>

| AI model | Input Shape | inferences/s | Inference time<br>(ms) | Params<br>(M) |   
|:--|--:|--:|--:|--:|
|[MiDaS v2.1 Small](./how_to_convert/How_to_convert_MiDaS_onnx_models.md)|256,256|3.6|274.1|16.6|


</details>

----   

[^1]: RUHMI Framework is powered by EdgeCortix MERA&trade;.