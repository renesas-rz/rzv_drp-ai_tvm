# How to solve specific problems

This directory contains solutions to specific problems related to Renesas [RUHMI](https://www.renesas.com/en/software-tool/ruhmi-framework)[^1] AI compiler.

## Index
- [How to solve specific problems](#how-to-solve-specific-problems)
  - [Index](#index)
  - [AI Sample Application for RZ/V2H and RZ/V2N](#rocket-ai-sample-application-for-rzv2h-and-rzv2n)
  - [AI Sample Application for RZ/V2H and RZ/V2N under GPL](#rocket-ai-sample-application-for-rzv2h-and-rzv2n-under-gpl)
  - [AI Sample Application for RZ/V2L, RZ/V2M and RZ/V2MA](#rocket-ai-sample-application-for-rzv2l-rzv2m-and-rzv2ma)
  - [Validation Application for RZ/V2L, RZ/V2H and RZ/V2N](#gear-validation-application)
  - [Python API Sample Script](#gear-python-api-sample-script)
  - [Tips](#bulb-tips)

## :rocket: AI Sample Application for RZ/V2H and RZ/V2N

Available sample applications:   

| Application | Model | Source Code | 
|:--- |:---|:---|
| Object Detection | YOLOx-s <br>(Re-trained by VOC) | [LINK](./sample_app_v2h/app_yolox_cam/) |
| Depth Estimation | MiDaS |[LINK](./sample_app_v2h/app_midas_cam/) |
| Classification | ResNet50 |[LINK](./sample_app_v2h/app_resnet50_cam/) |
| Semantic Segmentation | TopFormer | [LINK](./sample_app_v2h/app_topformer_cam/) |
| Semantic Segmentation | DeepLabv3 |   [LINK](./sample_app_v2h/app_deeplabv3_cam/) | 
| Pose Estimation | YOLOx-S Pose |[LINK](./sample_app_v2h/app_yolox_pose_cam/) |
| Pose Estimation | YOLOx-S+HRNet | [LINK](./sample_app_v2h/app_yolox_hrnet_cam/) |
| Face Landmark Detection | Face Landmarker |[LINK](./sample_app_v2h/app_face_landmarker_cam/) |


These sample applications are compatible with RZ/V2H AI SDK v6.00, RZ/V2N AI SDK v6.30 or later versions. <br>
The RZ/V2H AI SDK v6.00 and the RZ/V2N AI SDK v6.30 are already available in the links below.

* [RZ/V2H AI SDK](https://www.renesas.com/ja/software-tool/rzv2h-ai-software-development-kit)
* [RZ/V2N AI SDK](https://www.renesas.com/ja/software-tool/rzv2n-ai-software-development-kit)

> NOTE: If you want to use the sample applications with older versions of AI SDK, please use the older version at the link below.
> * [AI Sample Applications for DRP-AI TVM v2.5.0](https://github.com/renesas-rz/rzv_drp-ai_tvm/tree/v2.5.0/how-to/sample_app_v2h)

## :rocket: AI Sample Application for RZ/V2H and RZ/V2N under GPL 

> NOTE: The applications in this section are available under the **[GNU General Public License](https://www.gnu.org/licenses/licenses.html)**. <br>
> For the details of the license for each sample application and each model, please refer to the license file included in each directory of the sample applications.

To use the sample applications, please clone the [sample_app_v2h_gpl](https://github.com/renesas-rz/sample_app_v2h_gpl.git) repository with the following command in this directory.

```sh
cd $TVM_ROOT/how-to
git clone https://github.com/renesas-rz/sample_app_v2h_gpl.git
```

Available sample applications:   

| Application | Model | Source Code | 
|:--- |:---|:---|
| Object Detection | YOLOv5 | [LINK](https://github.com/renesas-rz/sample_app_v2h_gpl/tree/main/app_yolov5_cam/) |
| Object Detection | YOLOv6 | [LINK](https://github.com/renesas-rz/sample_app_v2h_gpl/tree/main/app_yolov6_cam/) |
| Object Detection | YOLOv7 | [LINK](https://github.com/renesas-rz/sample_app_v2h_gpl/tree/main/app_yolov7_cam/) |
| Object Detection | YOLOv8 | [LINK](https://github.com/renesas-rz/sample_app_v2h_gpl/tree/main/app_yolov8_cam/) |
| Object Detection | YOLOv9 | [LINK](https://github.com/renesas-rz/sample_app_v2h_gpl/tree/main/app_yolov9_cam/) |
| Object Detection | YOLO11 | [LINK](https://github.com/renesas-rz/sample_app_v2h_gpl/tree/main/app_yolov11_cam/) |
| Pose Estimation | YOLOv8-Pose | [LINK](https://github.com/renesas-rz/sample_app_v2h_gpl/tree/main/app_yolov8_pose_cam/) |
| Instance Segmentation | YOLOv8-Seg | [LINK](https://github.com/renesas-rz/sample_app_v2h_gpl/tree/main/app_yolov8_seg_cam/) |

These sample applications are compatible with RZ/V2H AI SDK v6.00, RZ/V2N AI SDK v6.30 or later versions. <br>
The RZ/V2H AI SDK v6.00 and the RZ/V2N AI SDK v6.30 are already available in the links below.

* [RZ/V2H AI SDK](https://www.renesas.com/ja/software-tool/rzv2h-ai-software-development-kit)
* [RZ/V2N AI SDK](https://www.renesas.com/ja/software-tool/rzv2n-ai-software-development-kit)

## :rocket: AI Sample Application for RZ/V2L, RZ/V2M and RZ/V2MA

Available sample applications:

| Application | Model | Source Code | 
|:--- |:---|:---|
| Face Landmark | DeepPose | [LINK](./sample_app/docs/face_landmark_localization/deeppose/README.md) |   
| Object Detection | YOLOv2/YOLOv3 |[LINK](./sample_app/docs/object_detection/yolo/README.md) |
| Pose Estimation | HRNet |[LINK](./sample_app/docs/human_pose_estimation/hrnet/README.md) |   
| Face Detection | UltraFace |[LINK](./sample_app/docs/face_detection/ultraface/README.md) |   
| Hand Landmark | HRNetV2 |[LINK](./sample_app/docs/hand_landmark_localization/hrnetv2/README.md) |   
| Emotion Recognition | Emotion FERPlus |[LINK](./sample_app/docs/emotion_recognition/emotion_ferplus/README.md) |   
| Classification | GoogleNet | [LINK](./sample_app/docs/classification/googlenet/README.md) |   
| Semantic Segmentation | DeepLabV3  | [LINK](./sample_app/docs/semantic_segmentation/deeplabv3/README.md) |   

## :gear: Validation Application
Validation Application is a C++ application that allow users to verify the AI models compiled by RUHMI [^1] on the target board.  

- [Validation Application](./validation)

## :gear: Python API Sample Script  
Sample script to run AI models with Python API on RZ/V2H and RZ/V2N.  
Script can also be used for validating the AI models compiled by RUHMI [^1].  
- [How to install and run Python API](python/README.md)

## :bulb: Tips
- [How to read compile log](tips/how-to-read-log.md)
- [How to compare CPU vs DRP-AI TVM accuracy differences for ONNX model](tips/compare_difference)
- [How to obtain profiling data](tips/profiling)
- [How to optimize post-processing](tips/post_processing_optimization_method/README.md)
- [How to stabilize AI systems on RZ/V2H or RZ/V2N](tips/system_stabilization_method/README.md)

---
[^1]: RUHMI Framework is powered by EdgeCortix MERA&trade;.
