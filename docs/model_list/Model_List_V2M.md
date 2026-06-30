# Model list for RZ/V2M

---

Below is a list of AI models that Renesas has verified for conversion with the RUHMI[^1] and actual operation on an evaluation board.

| Item                   | RZ/V2M     |
|------------------------|------------|
| RUHMI[^1] AI compiler  | R2026-06   |
| Evaluation Board       | RZ/V2M EVK |
| DRP-AI Translator      | v1.90      |
| Linux Package          | v3.0.7     |

**[NOTE]** This benchmark shows the results when only AI inference is running. When system operations such as camera input and display output are performed simultaneously, they may affect the inference time.   


## **Representative models**

| AI model | Input Shape | Inferences/s | Inference time<br>(ms) | Task | Params<br>(M) |
|:--|--:|--:|--:|:--|--:|
|[YOLOv3](./how_to_convert/How_to_convert_yolov3_onnx_model.md)|416,416|5.0|198.2|Object Detection|61.9|
|[YOLOv5n](./how_to_convert/How_to_convert_yolov5_onnx_models.md)|640,640|1.6|623.0|Object Detection|1.9|
|[YOLOv8n](./how_to_convert/How_to_convert_yolov8_onnx_models.md)|640,640|5.5|181.9|Object Detection|3.1|
|[YOLOX_s](./how_to_convert/How_to_convert_yolox_onnx_models.md)|640,640|8.2|121.6|Object Detection|9.0|
|[YOLOX_m](./how_to_convert/How_to_convert_yolox_onnx_models.md)|640,640|4.0|251.1|Object Detection|25.3|
|[MobileNetV2](./how_to_convert/How_to_download_ONNX_models.md)|224,224|70.1|14.3|Classification|3.5|
|[ResNet50-v1](./how_to_convert/How_to_download_ONNX_models.md)|224,224|28.4|35.1|Classification|25.6|
|[HRNet](./how_to_convert/How_to_convert_hrnet_onnx_model.md)|256,192|16.0|62.4|Pose Estimation|28.5|

---

## **List of models for each task**

Please click on a task name to view its performance

<details><summary><u>Object Detection</u></summary>

| AI model | Input Shape | Inferences/s | Inference time<br>(ms)| Params<br>(M) |
|:--|--:|--:|--:|--:|
|[EdgeYOLO_Tiny](./how_to_convert/How_to_convert_edgeyolo_models_onnx.md)|640,640|3.2|316.8|5.5|
|[EdgeYOLO_S](./how_to_convert/How_to_convert_edgeyolo_models_onnx.md)|640,640|2.3|432.5|9.3|
|[EdgeYOLO_M](./how_to_convert/How_to_convert_edgeyolo_models_onnx.md)|640,640|3.7|270.0|17.8|
|[YOLOv2](./how_to_convert/How_to_download_ONNX_models.md)|416,416|12.7|78.4|51.0|
|[YOLOv3](./how_to_convert/How_to_convert_yolov3_onnx_model.md)|416,416|5.0|198.2|61.9|
|[YOLOv5n](./how_to_convert/How_to_convert_yolov5_onnx_models.md)|640,640|1.6|623.0|1.9|
|[YOLOv5s](./how_to_convert/How_to_convert_yolov5_onnx_models.md)|640,640|0.8|1200.3|7.3|
|[YOLOv6n](./how_to_convert/How_to_convert_yolov6_onnx_models.md)|640,640|1.2|853.5|4.7|
|[YOLOv7](./how_to_convert/How_to_convert_yolov7_onnx_models.md)|640,640|2.9|339.7|36.9|
|[YOLOv7-w6](./how_to_convert/How_to_convert_yolov7_onnx_models.md)|640,640|3.7|271.7|70.4|
|[YOLOv8n](./how_to_convert/How_to_convert_yolov8_onnx_models.md)|640,640|5.5|181.9|3.1|
|[YOLOv8s](./how_to_convert/How_to_convert_yolov8_onnx_models.md)|640,640|3.0|333.2|11.2|
|[YOLOX_s](./how_to_convert/How_to_convert_yolox_onnx_models.md)|640,640|8.2|121.6|9.0|
|[YOLOX_m](./how_to_convert/How_to_convert_yolox_onnx_models.md)|640,640|4.0|251.1|25.3|
|[YOLOX_l](./how_to_convert/How_to_convert_yolox_onnx_models.md)|640,640|2.3|434.0|54.2|

</details>

<details><summary><u>Classification</u></summary>

| AI model | Input Shape | Inferences/s | Inference time<br>(ms)| Params<br>(M) |
|:--|--:|--:|--:|--:|
|[DenseNet9](./how_to_convert/How_to_download_ONNX_models.md)|224,224|6.5|155.0|8.2|
|[Inception-v2](./how_to_convert/How_to_download_ONNX_models.md)|224,224|10.1|99.2|11.2|
|[MobileNetV2](./how_to_convert/How_to_download_ONNX_models.md)|224,224|70.1|14.3|3.5|
|[ResNet18-v1](./how_to_convert/How_to_download_ONNX_models.md)|224,224|61.7|16.2|11.7|
|[ResNet34-v1](./how_to_convert/How_to_download_ONNX_models.md)|224,224|38.6|25.9|21.8|
|[ResNet50-v1](./how_to_convert/How_to_download_ONNX_models.md)|224,224|28.4|35.1|25.6|
|[ResNet101-v1](./how_to_convert/How_to_download_ONNX_models.md)|224,224|18.0|55.4|44.7|
|[ResNet18-v2](./how_to_convert/How_to_download_ONNX_models.md)|224,224|56.6|17.7|11.7|
|[ResNet34-v2](./how_to_convert/How_to_download_ONNX_models.md)|224,224|35.6|28.1|21.8|
|[ResNet50-v2](./how_to_convert/How_to_download_ONNX_models.md)|224,224|22.5|44.5|25.6|
|[ResNet101-v2](./how_to_convert/How_to_download_ONNX_models.md)|224,224|14.4|69.4|44.6|
|[SqueezeNet1.1-7](./how_to_convert/How_to_download_ONNX_models.md)|224,224|156.5|6.4|1.2|
|[DenseNet-121](./how_to_convert/How_to_convert_torchvision_models.md)|224,224|3.5|284.5|8.0|
|[GoogleNet](./how_to_convert/How_to_convert_torchvision_models.md)|224,224|4.2|237.6|6.6|
|[MnasNet0_5](./how_to_convert/How_to_convert_torchvision_models.md)|224,224|20.1|49.9|2.2|
|[ResNeXt-50-32x4d](./how_to_convert/How_to_convert_torchvision_models.md)|224,224|1.3|759.5|25.0|
|[SqueezeNet1_1](./how_to_convert/How_to_convert_torchvision_models.md)|224,224|16.4|61.1|1.2|

</details>

<details><summary><u>Pose Estimation</u></summary>

| AI model | Input Shape | Inferences/s | Inference time<br>(ms)| Params<br>(M) |
|:--|--:|--:|--:|--:|
|[HRNet](./how_to_convert/How_to_convert_hrnet_onnx_model.md)|256,192|16.0|62.4|28.5|
|[DeepPose](./how_to_convert/How_to_convert_mmpose_models.md)|256,192|28.8|34.7|23.6|
|[HRNetV2](./how_to_convert/How_to_convert_mmpose_models.md)|256,192|28.5|35.0|9.7|
|[HRNetV2 DarkPose](./how_to_convert/How_to_convert_mmpose_models.md)|256,192|20.3|49.2|9.7|

</details>

<details><summary><u>Semantic Segmentation</u></summary>

| AI model | Input Shape | Inferences/s | Inference time<br>(ms)| Params<br>(M) |
|:--|--:|--:|--:|--:|
|[DeepLabv3-resnet50](./how_to_convert/How_to_convert_torchvision_models.md)|224,224|5.9|169.3|42.0|
|[DeepLabv3-resnet101](./how_to_convert/How_to_convert_torchvision_models.md)|224,224|3.7|271.5|61.0|
|[FCN_resnet101](./how_to_convert/How_to_convert_torchvision_models.md)|224,224|3.8|262.8|54.3|

</details>

<details><summary><u>Depth Estimation</u></summary>

| AI model | Input Shape | Inferences/s | Inference time<br>(ms)| Params<br>(M) |
|:--|--:|--:|--:|--:|
|[MiDaS v2.1 Small](./how_to_convert/How_to_convert_MiDaS_onnx_models.md)|256,256|2.6|387.6|16.6|


</details>

----   

[^1]: RUHMI Framework is powered by EdgeCortix MERA&trade;.