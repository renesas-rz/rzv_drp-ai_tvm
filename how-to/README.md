# How to solve specific problems

This directory contains the solution to specific problems related to DRP-AI TVM[^1].  

## Index
- [How to solve specific problems](#how-to-solve-specific-problems)
  - [Index](#index)
  - [AI Sample Application for RZ/V2L, RZ/V2M and RZ/V2MA](#ai-sample-application-for-rzv2l-rzv2m-and-rzv2ma)
  - [AI Sample Application for RZ/V2H and RZ/V2N](#ai-sample-application-for-rzv2h-and-rzv2n)
  - [AI Sample Application for RZ/V2H and RZ/V2N under GPL](#ai-sample-application-for-rzv2h-and-rzv2n-under-gpl)
  - [Tips](#tips)


## AI Sample Application for RZ/V2L, RZ/V2M and RZ/V2MA
<table>
<tr>
    <td align="center"><a href="sample_app/docs/face_landmark_localization/deeppose"><img src="img/face_landmark_localization.png"></a></td>
    <td align="center"><a href="sample_app/docs/object_detection/yolo"><img src="img/object_detection.png"></a></td>
    <td align="center"><a href="sample_app/docs/human_pose_estimation/hrnet"><img src="img/2d_pose_estimation.png"></a></td>
</tr>
<tr>
    <td align="center"><a href="sample_app/docs/face_landmark_localization/deeppose">Facial Landmark Localization</a></td>
    <td align="center"><a href="sample_app/docs/object_detection/yolo">Object Detection</a></td>
    <td align="center"><a href="sample_app/docs/human_pose_estimation/hrnet">2D Pose Estimation</a></td>
</tr>
<tr>
    <td align="center"><a href="sample_app/docs/face_detection/ultraface"><img src="img/face_detection.png"></a></td>
    <td align="center"><a href="sample_app/docs/hand_landmark_localization/hrnetv2"><img src="img/2d_hand_estimation.png"></a></td>
    <td align="center"><a href="sample_app/docs/emotion_recognition/emotion_ferplus"><img src="img/emotion.png"></a></td>
</tr>
<tr>
    <td align="center"><a href="sample_app/docs/face_detection/ultraface">Face Detection</a></td>
    <td align="center"><a href="sample_app/docs/hand_landmark_localization/hrnetv2">Hand Landmark Localization</a></td>
    <td align="center"><a href="sample_app/docs/emotion_recognition/emotion_ferplus">Emotion Recognition</a></td>
</tr>
<tr>
    <td align="center"><a href="sample_app/docs/classification/googlenet"><img src="img/classification.png"></a></td>
    <td align="center"><a href="sample_app/docs/semantic_segmentation/deeplabv3"><img src="img/semantic_segmentation.png"></a></td>
    <td align="center"><a href=""><img src="img/age_classification_dev.png"></a></td>
</tr>
<tr>
    <td align="center"><a href="sample_app/docs/classification/googlenet">Classification</a></td>
    <td align="center"><a href="sample_app/docs/semantic_segmentation/deeplabv3">Semantic Segmentation</a></td>
    <td align="center">Age Classification </td>
</tr>
</table>

## AI Sample Application for RZ/V2H and RZ/V2N

The applications for the following models are available.

- [Semantic Image Segmentation: DeepLabv3](./sample_app_v2h/app_deeplabv3_cam/)
- [Depth Estimation: MiDaS](./sample_app_v2h/app_midas_cam/)
- [Classification: Resnet50](./sample_app_v2h/app_resnet50_cam/)
- [Semantic Image Segmentation: TopFormer](./sample_app_v2h/app_topformer_cam/)
- [Object Detection: Re-trained YOLOX-s by VOC dataset](./sample_app_v2h/app_yolox_cam/)
- [Pose Estimation: YOLOX-S Pose](./sample_app_v2h/app_yolox_pose_cam)
- [Pose Estimation: YOLOX-S+HRNet](./sample_app_v2h/app_yolox_hrnet_cam)

These sample applications are compatible with RZ/V2H AI SDK v6.00, RZ/V2N AI SDK v6.00 or later versions. <br>
The RZ/V2H AI SDK v6.00 will be available in January 2026, and the RZ/V2N AI SDK v6.00 is already available in the links below.

* [RZ/V2H AI SDK](https://www.renesas.com/ja/software-tool/rzv2h-ai-software-development-kit)
* [RZ/V2N AI SDK](https://www.renesas.com/ja/software-tool/rzv2n-ai-software-development-kit)

> NOTE: If you want to use the sample applications with older versions of AI SDK, please use the older version in the link below.
> * [AI Sample Applications for DRP-AI TVM v2.5.0](https://github.com/renesas-rz/rzv_drp-ai_tvm/tree/v2.5.0/how-to/sample_app_v2h)

## AI Sample Application for RZ/V2H and RZ/V2N under GPL 

> NOTE: The applications in this section are available under a **[GNU General public license](https://www.gnu.org/licenses/licenses.html)**. <br>
> For the details of the license for each sample application and each model, please refer to the license file included in each directory of the sample applications.

To use the sample applications, please clone the [sample_app_v2h_gpl](https://github.com/renesas-rz/sample_app_v2h_gpl.git) repository with the following command in this directory.

```
cd $TVM_ROOT/how-to
git clone https://github.com/renesas-rz/sample_app_v2h_gpl.git
```

The applications for the following models are available.

- [Object Detection: YOLOv5](https://github.com/renesas-rz/sample_app_v2h_gpl/app_yolov5_cam/)
- [Object Detection: YOLOv6](https://github.com/renesas-rz/sample_app_v2h_gpl/app_yolov6_cam/)
- [Object Detection: YOLOv7](https://github.com/renesas-rz/sample_app_v2h_gpl/app_yolov7_cam/)
- [Object Detection: YOLOv8](https://github.com/renesas-rz/sample_app_v2h_gpl/app_yolov8_cam/)
- [Object Detection: YOLOv9](https://github.com/renesas-rz/sample_app_v2h_gpl/app_yolov9_cam/)
- [Object Detection: YOLOv11](https://github.com/renesas-rz/sample_app_v2h_gpl/app_yolov11_cam/)

These sample applications are compatible with RZ/V2H AI SDK v6.00, RZ/V2N AI SDK v6.00 or later versions. <br>
The RZ/V2H AI SDK v6.00 will be available in January 2026, and the RZ/V2N AI SDK v6.00 is already available in the links below.

* [RZ/V2H AI SDK](https://www.renesas.com/ja/software-tool/rzv2h-ai-software-development-kit)
* [RZ/V2N AI SDK](https://www.renesas.com/ja/software-tool/rzv2n-ai-software-development-kit)

## Tips
- [How to read compile log](tips/how-to-read-log.md)
- [How to compare CPU vs DRP-AI TVM accuracy differences for ONNX model](tips/compare_difference)
- [How to obtain profiling data](tips/profiling)
- [How to optimize post-processing](tips/post_processing_optimization_method/README.md)

---
[^1]: DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.
