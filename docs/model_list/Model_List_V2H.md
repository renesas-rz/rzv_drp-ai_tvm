# Model list for RZ/V2H

---

Below is a list of AI models that Renesas has verified for conversion with the RUHMI[^1] and actual operation on an evaluation board.

| Item                       | RZ/V2H        |
| -------------------------- | ------------- |
| RUHMI[^1] AI compiler      | R2026-06      |
| Evaluation Board           | RZ/V2H EVK    |
| DRP-AI Translator i8       | v1.12         |
| RZ/V2H AI SDK              | v6.00         |

**[NOTE]** This benchmark shows the results when only AI inference is running. When system operations such as camera input and display output are performed simultaneously, they may affect the inference time.   

## **Representative models**

| AI model | Input Shape | Inferences/s | Inference time<br>(ms) | Task | Params<br>(M) |
|:--|--:|--:|--:|:--|--:|
|[YOLOv5s](./how_to_convert/How_to_convert_yolov5_onnx_models.md)|640,640|51.8|19.3|Object Detection|7.3|
|[YOLOv5m](./how_to_convert/How_to_convert_yolov5_onnx_models.md)|640,640|32.6|30.7|Object Detection|21.4|
|[YOLOv8n](./how_to_convert/How_to_convert_yolov8_onnx_models.md)|640,640|69.2|14.4|Object Detection|3.1|
|[YOLOv8s](./how_to_convert/How_to_convert_yolov8_onnx_models.md)|640,640|49.5|20.2|Object Detection|11.2|
|[YOLOv8m](./how_to_convert/How_to_convert_yolov8_onnx_models.md)|640,640|26.5|37.8|Object Detection|25.9|
|[YOLO11n](./how_to_convert/How_to_convert_yolov11_onnx_models.md)|640,640|63.7|15.7|Object Detection|2.6|
|[YOLO11s](./how_to_convert/How_to_convert_yolov11_onnx_models.md)|640,640|44.0|22.7|Object Detection|9.4|
|[YOLO11m](./how_to_convert/How_to_convert_yolov11_onnx_models.md)|640,640|23.7|42.3|Object Detection|20.1|
|[YOLO26n](./how_to_convert/How_to_convert_yolo26_o2m_onnx_models.md)|640,640|56.3|17.8|Object Detection|2.4|
|[YOLO26s](./how_to_convert/How_to_convert_yolo26_o2m_onnx_models.md)|640,640|35.8|27.9|Object Detection|9.5|
|[YOLO26m](./how_to_convert/How_to_convert_yolo26_o2m_onnx_models.md)|640,640|18.6|53.7|Object Detection|20.4|
|[YOLOX_s](./how_to_convert/How_to_convert_yolox_onnx_models.md)|640,640|60.2|16.6|Object Detection|9.0|
|[YOLOX_m](./how_to_convert/How_to_convert_yolox_onnx_models.md)|640,640|31.3|31.9|Object Detection|25.3|
|[MobileNetV2](./how_to_convert/How_to_download_ONNX_models.md)|224,224|800.0|1.2|Classification|3.5|
|[ResNet50-v1](./how_to_convert/How_to_download_ONNX_models.md)|224,224|243.9|4.1|Classification|25.6|
|[HRNet](./how_to_convert/How_to_convert_hrnet_onnx_model.md)|256,192|113.8|8.8|Pose Estimation|28.5|
|[YOLOv8n-pose](./how_to_convert/How_to_convert_yolov8pose_onnx_models.md)|640,640|70.5|14.2|Multi Person Pose Estimation|3.3|
|[YOLOv8s-pose](./how_to_convert/How_to_convert_yolov8pose_onnx_models.md)|640,640|49.3|20.3|Multi Person Pose Estimation|11.6|
|[YOLOv8m-pose](./how_to_convert/How_to_convert_yolov8pose_onnx_models.md)|640,640|26.3|38.0|Multi Person Pose Estimation|26.4|
|[YOLOv8n-seg](./how_to_convert/How_to_convert_yolov8-seg_onnx_models.md)|640,640|50.0|20.0|Instance Segmentation|3.4|
|[YOLOv8s-seg](./how_to_convert/How_to_convert_yolov8-seg_onnx_models.md)|640,640|36.6|27.3|Instance Segmentation|11.8|
|[YOLOv8m-seg](./how_to_convert/How_to_convert_yolov8-seg_onnx_models.md)|640,640|20.6|48.5|Instance Segmentation|27.3|
|[Topformer tiny](./how_to_convert/How_to_download_ONNX_models.md)|512,512|13.8|72.5|Semantic Segmentation|1.4|
|[face_landmark](./how_to_convert/How_to_download_ONNX_models.md)|192,192|613.5|1.6|Face Landmark Detection|0.6|
|[hand_landmark](./how_to_convert/How_to_download_ONNX_models.md)|256,256|363.6|2.8|Hand Landmark Detection|2.0|
|[MiDaS v2.1 Small](./how_to_convert/How_to_convert_MiDaS_onnx_models.md)|256,256|123.5|8.1|Depth Estimation|16.6|
|[Paddle OCR V3 det](./how_to_convert/How_to_convert_paddleOCR_V3_onnx_models.md)|640,640|43.3|23.1|Text Detection　|1.2|
|[Paddle OCR V3 rec](./how_to_convert/How_to_convert_paddleOCR_V3_onnx_models.md)|48,480|27.7|36.1|Text Recognition|4.1|

---
## **List of models for each task**

Please click on a task name to view its performance

<details><summary><u>Object Detection</u></summary>

| AI model | Input Shape | Inferences/s | Inference time<br>(ms) | Params<br>(M) |
|:--|--:|--:|--:|--:|
|[EdgeYOLO_Tiny](./how_to_convert/How_to_convert_edgeyolo_models_onnx.md)|640,640|41.8|23.9|5.5|
|[EdgeYOLO_S](./how_to_convert/How_to_convert_edgeyolo_models_onnx.md)|640,640|33.9|29.5|9.3|
|[EdgeYOLO_M](./how_to_convert/How_to_convert_edgeyolo_models_onnx.md)|640,640|27.5|36.4|17.8|
|[YOLOv2](./how_to_convert/How_to_download_ONNX_models.md)|416,416|71.8|13.9|51.0|
|[YOLOv3](./how_to_convert/How_to_convert_yolov3_onnx_model.md)|416,416|37.1|26.9|61.9|
|[YOLOv5s](./how_to_convert/How_to_convert_yolov5_onnx_models.md)|640,640|51.8|19.3|7.3|
|[YOLOv5m](./how_to_convert/How_to_convert_yolov5_onnx_models.md)|640,640|32.6|30.7|21.4|
|[YOLOv5l](./how_to_convert/How_to_convert_yolov5_onnx_models.md)|640,640|23.5|42.6|47.0|
|[YOLOv5x](./how_to_convert/How_to_convert_yolov5_onnx_models.md)|640,640|13.6|73.7|87.7|
|[YOLOv6n](./how_to_convert/How_to_convert_yolov6_onnx_models.md)|640,640|99.5|10.1|4.7|
|[YOLOv6s](./how_to_convert/How_to_convert_yolov6_onnx_models.md)|640,640|62.5|16.0|18.5|
|[YOLOv6m](./how_to_convert/How_to_convert_yolov6_onnx_models.md)|640,640|27.6|36.2|34.9|
|[YOLOv6l](./how_to_convert/How_to_convert_yolov6_onnx_models.md)|640,640|20.4|49.0|59.6|
|[YOLOv7](./how_to_convert/How_to_convert_yolov7_onnx_models.md)|640,640|20.5|48.7|36.9|
|[YOLOv7-w6](./how_to_convert/How_to_convert_yolov7_onnx_models.md)|640,640|23.2|43.1|70.4|
|[YOLOv7x](./how_to_convert/How_to_convert_yolov7_onnx_models.md)|640,640|14.0|71.3|71.3|
|[YOLOv7-e6](./how_to_convert/How_to_convert_yolov7_onnx_models.md)|640,640|15.8|63.1|97.2|
|[YOLOv7-d6](./how_to_convert/How_to_convert_yolov7_onnx_models.md)|640,640|13.0|77.0|133.8|
|[YOLOv7-e6e](./how_to_convert/How_to_convert_yolov7_onnx_models.md)|640,640|10.6|94.4|151.7|
|[YOLOv8n](./how_to_convert/How_to_convert_yolov8_onnx_models.md)|640,640|69.2|14.4|3.1|
|[YOLOv8s](./how_to_convert/How_to_convert_yolov8_onnx_models.md)|640,640|49.5|20.2|11.2|
|[YOLOv8m](./how_to_convert/How_to_convert_yolov8_onnx_models.md)|640,640|26.5|37.8|25.9|
|[YOLOv8l](./how_to_convert/How_to_convert_yolov8_onnx_models.md)|640,640|18.4|54.4|43.7|
|[YOLOv8x](./how_to_convert/How_to_convert_yolov8_onnx_models.md)|640,640|12.3|81.5|68.2|
|[YOLOv9-t](./how_to_convert/How_to_convert_yolov9_onnx_models.md)|640,640|61.5|16.3|2.0|
|[YOLOv9-s](./how_to_convert/How_to_convert_yolov9_onnx_models.md)|640,640|45.0|22.2|7.1|
|[YOLOv9-m](./how_to_convert/How_to_convert_yolov9_onnx_models.md)|640,640|23.3|42.9|20.0|
|[YOLOv10n](./how_to_convert/How_to_convert_yolov10_onnx_models.md)|640,640|21.2|47.1|2.3|
|[YOLOv10s](./how_to_convert/How_to_convert_yolov10_onnx_models.md)|640,640|12.8|78.0|7.2|
|[YOLOv10m](./how_to_convert/How_to_convert_yolov10_onnx_models.md)|640,640|10.2|97.9|15.4|
|[YOLOv10b](./how_to_convert/How_to_convert_yolov10_onnx_models.md)|640,640|9.3|107.9|19.1|
|[YOLOv10l](./how_to_convert/How_to_convert_yolov10_onnx_models.md)|640,640|8.6|116.0|24.4|
|[YOLOv10x](./how_to_convert/How_to_convert_yolov10_onnx_models.md)|640,640|6.4|156.9|29.5|
|[YOLO11n](./how_to_convert/How_to_convert_yolov11_onnx_models.md)|640,640|63.7|15.7|2.6|
|[YOLO11s](./how_to_convert/How_to_convert_yolov11_onnx_models.md)|640,640|44.0|22.7|9.4|
|[YOLO11m](./how_to_convert/How_to_convert_yolov11_onnx_models.md)|640,640|23.7|42.3|20.1|
|[YOLO11l](./how_to_convert/How_to_convert_yolov11_onnx_models.md)|640,640|19.2|52.1|25.3|
|[YOLO26n](./how_to_convert/How_to_convert_yolo26_o2m_onnx_models.md)|640,640|56.3|17.8|2.4|
|[YOLO26s](./how_to_convert/How_to_convert_yolo26_o2m_onnx_models.md)|640,640|35.8|27.9|9.5|
|[YOLO26m](./how_to_convert/How_to_convert_yolo26_o2m_onnx_models.md)|640,640|18.6|53.7|20.4|
|[YOLO26l](./how_to_convert/How_to_convert_yolo26_o2m_onnx_models.md)|640,640|15.2|65.7|24.8|
|[YOLOX_s](./how_to_convert/How_to_convert_yolox_onnx_models.md)|640,640|60.2|16.6|9.0|
|[YOLOX_m](./how_to_convert/How_to_convert_yolox_onnx_models.md)|640,640|31.3|31.9|25.3|
|[YOLOX_l](./how_to_convert/How_to_convert_yolox_onnx_models.md)|640,640|19.7|50.8|54.2|
|[YOLOX_x](./how_to_convert/How_to_convert_yolox_onnx_models.md)|640,640|11.7|85.1|99.0|

</details>

<details><summary><u>Classification</u></summary>

| AI model | Input Shape | Inferences/s | Inference time<br>(ms) | Params<br>(M) |
|:--|--:|--:|--:|--:|
|[DenseNet9](./how_to_convert/How_to_download_ONNX_models.md)|224,224|157.7|6.3|8.2|
|[MobileNetV2](./how_to_convert/How_to_download_ONNX_models.md)|224,224|800.0|1.2|3.5|
|[ResNet18-v1](./how_to_convert/How_to_download_ONNX_models.md)|224,224|448.4|2.2|11.7|
|[ResNet34-v1](./how_to_convert/How_to_download_ONNX_models.md)|224,224|278.6|3.6|21.8|
|[ResNet50-v1](./how_to_convert/How_to_download_ONNX_models.md)|224,224|243.9|4.1|25.6|
|[ResNet101-v1](./how_to_convert/How_to_download_ONNX_models.md)|224,224|154.8|6.5|44.7|
|[ResNet18-v2](./how_to_convert/How_to_download_ONNX_models.md)|224,224|411.5|2.4|11.7|
|[ResNet34-v2](./how_to_convert/How_to_download_ONNX_models.md)|224,224|263.9|3.8|21.8|
|[ResNet50-v2](./how_to_convert/How_to_download_ONNX_models.md)|224,224|157.5|6.3|25.6|
|[ResNet101-v2](./how_to_convert/How_to_download_ONNX_models.md)|224,224|98.1|10.2|44.6|
|[SqueezeNet1.1-7](./how_to_convert/How_to_download_ONNX_models.md)|224,224|704.2|1.4|1.2|
|[DenseNet-121](./how_to_convert/How_to_convert_torchvision_models.md)|224,224|181.5|5.5|8.0|
|[DenseNet-161](./how_to_convert/How_to_convert_torchvision_models.md)|224,224|47.1|21.2|28.7|
|[GoogleNet](./how_to_convert/How_to_convert_torchvision_models.md)|224,224|203.7|4.9|6.6|
|[MnasNet0_5](./how_to_convert/How_to_convert_torchvision_models.md)|224,224|653.6|1.5|2.2|
|[ResNeXt-50-32x4d](./how_to_convert/How_to_convert_torchvision_models.md)|224,224|83.5|12.0|25.0|
|[SqueezeNet1_1](./how_to_convert/How_to_convert_torchvision_models.md)|224,224|704.2|1.4|1.2|
|[Swin Transformer(tiny)](./how_to_convert/How_to_convert_swin_onnx_model.md)|224,224|2.2|458.6|28.8|
|[Vision Transformer(tiny)](./how_to_convert/How_to_convert_VIT_onnx_models.md)|224,224|6.8|147.9|5.7|
|[Vision Transformer(small)](./how_to_convert/How_to_convert_VIT_onnx_models.md)|224,224|3.7|270.4|22.1|

</details>

<details><summary><u>Pose Estimation</u></summary>

| AI model | Input Shape | Inferences/s | Inference time<br>(ms) | Params<br>(M) |
|:--|--:|--:|--:|--:|
|[HRNet](./how_to_convert/How_to_convert_hrnet_onnx_model.md)|256,192|113.8|8.8|28.5|
|[DeepPose](./how_to_convert/How_to_convert_mmpose_models.md)|256,192|253.8|3.9|23.6|
|[HRNetV2](./how_to_convert/How_to_convert_mmpose_models.md)|256,192|193.1|5.2|9.7|
|[HRNetV2 DarkPose](./how_to_convert/How_to_convert_mmpose_models.md)|256,192|136.6|7.3|9.7|

</details>

<details><summary><u>Multi Person Pose Estimation</u></summary>

| AI model | Input Shape | Inferences/s | Inference time<br>(ms) | Params<br>(M) |
|:--|--:|--:|--:|--:|
|[YOLOv8n-pose](./how_to_convert/How_to_convert_yolov8pose_onnx_models.md)|640,640|70.5|14.2|3.3|
|[YOLOv8s-pose](./how_to_convert/How_to_convert_yolov8pose_onnx_models.md)|640,640|49.3|20.3|11.6|
|[YOLOv8m-pose](./how_to_convert/How_to_convert_yolov8pose_onnx_models.md)|640,640|26.3|38.0|26.4|
|[YOLOv8l-pose](./how_to_convert/How_to_convert_yolov8pose_onnx_models.md)|640,640|22.1|45.2|33.3|
|[YOLOv8x-pose](./how_to_convert/How_to_convert_yolov8pose_onnx_models.md)|640,640|14.5|68.9|52.0|
|[Yolo11n-Pose](./how_to_convert/How_to_convert_yolo11pose_onnx_models.md)|640,640|64.4|15.5|2.9|
|[Yolo11s-Pose](./how_to_convert/How_to_convert_yolo11pose_onnx_models.md)|640,640|43.8|22.9|9.9|
|[Yolo11m-Pose](./how_to_convert/How_to_convert_yolo11pose_onnx_models.md)|640,640|23.4|42.7|20.9|
|[Yolo11l-Pose](./how_to_convert/How_to_convert_yolo11pose_onnx_models.md)|640,640|19.0|52.6|26.1|
|[YoloX_s-Pose](./how_to_convert/Translator_models.md)|640,640|55.8|17.9|10.8|

</details>

<details><summary><u>Instance Segmentation</u></summary>

| AI model | Input Shape | Inferences/s | Inference time<br>(ms) | Params<br>(M) |
|:--|--:|--:|--:|--:|
|[YOLOv8n-seg](./how_to_convert/How_to_convert_yolov8-seg_onnx_models.md)|640,640|50.0|20.0|3.4|
|[YOLOv8s-seg](./how_to_convert/How_to_convert_yolov8-seg_onnx_models.md)|640,640|36.6|27.3|11.8|
|[YOLOv8m-seg](./how_to_convert/How_to_convert_yolov8-seg_onnx_models.md)|640,640|20.6|48.5|27.3|
|[YOLOv8l-seg](./how_to_convert/How_to_convert_yolov8-seg_onnx_models.md)|640,640|14.8|67.5|46.0|
|[YOLOv8x-seg](./how_to_convert/How_to_convert_yolov8-seg_onnx_models.md)|640,640|10.1|99.3|71.8|


</details>

<details><summary><u>Semantic Segmentation</u></summary>

| AI model | Input Shape | Inferences/s | Inference time<br>(ms) | Params<br>(M) |   
|:--|--:|--:|--:|--:|
|[UNET-mobilenetV2](./how_to_convert/How_to_convert_UNET_onnx_models.md)|256,256|87.1|11.5|6.6|
|[DeepLabv3-resnet50](./how_to_convert/How_to_convert_torchvision_models.md)|224,224|38.9|25.7|42.0|
|[DeepLabv3-resnet101](./how_to_convert/How_to_convert_torchvision_models.md)|224,224|33.3|30.1|61.0|
|[FCN_resnet101](./how_to_convert/How_to_convert_torchvision_models.md)|224,224|36.4|27.5|54.3|
|[Topformer tiny](./how_to_convert/How_to_download_ONNX_models.md)|512,512|13.8|72.5|1.4|

</details>

<details><summary><u>Face Landmark Detection</u></summary>

| AI model | Input Shape | Inferences/s | Inference time<br>(ms) | Params<br>(M) |   
|:--|--:|--:|--:|--:|
|[face_landmark](./how_to_convert/How_to_download_ONNX_models.md)|192,192|613.5|1.6|0.6|

</details>

<details><summary><u>Face Recognition</u></summary>

| AI model | Input Shape | Inferences/s | Inference time<br>(ms) | Params<br>(M) |   
|:--|--:|--:|--:|--:|
|[InceptionResnetV1_vggface2](./how_to_convert/How_to_convert_facenet_pytorch_onnx_models.md)|160,160|232.6|4.3|23.5|

</details>

<details><summary><u>Hand Landmark Detection</u></summary>

| AI model | Input Shape | Inferences/s | Inference time<br>(ms) | Params<br>(M) |   
|:--|--:|--:|--:|--:|
|[hand_landmark](./how_to_convert/How_to_download_ONNX_models.md)|256,256|363.6|2.8|2.0|

</details>

<details><summary><u>Depth Estimation</u></summary>

| AI model | Input Shape | Inferences/s | Inference time<br>(ms) | Params<br>(M) |   
|:--|--:|--:|--:|--:|
|[MiDaS v2.1 Small](./how_to_convert/How_to_convert_MiDaS_onnx_models.md)|256,256|123.5|8.1|16.6|

</details>

<details><summary><u>Text Detection</u></summary>

| AI model | Input Shape | Inferences/s | Inference time<br>(ms) | Params<br>(M) |   
|:--|--:|--:|--:|--:|
|[Paddle OCR V3 det](./how_to_convert/How_to_convert_paddleOCR_V3_onnx_models.md)|640,640|43.3|23.1|1.2|

</details>

<details><summary><u>Text Recognition</u></summary>

| AI model | Input Shape | Inferences/s | Inference time<br>(ms) | Params<br>(M) |   
|:--|--:|--:|--:|--:|
|[Paddle OCR V3 rec](./how_to_convert/How_to_convert_paddleOCR_V3_onnx_models.md)|48,480|27.7|36.1|4.1|

</details>

:information_source:　The evaluation of this model list was performed by extending the DRP-AI memory area.
Please refer to the [guide](https://github.com/renesas-rz/rzv_drp-ai_tvm/blob/main/docs/model_list/how_to_expand_drpai_memory.md) for instructions on how to extend the DRP-AI memory area.

----   

[^1]: RUHMI Framework is powered by EdgeCortix MERA&trade;.