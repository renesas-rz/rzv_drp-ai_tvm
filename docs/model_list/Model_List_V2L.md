# Model list for RZ/V2L  

Below is a list of AI models that Renesas has verified for conversion with the DRP-AI TVM[^1] and actual operation on an evaluation board.

| Item                   |RZ/V2L     |
|------------------------|-----------|
| DRP-AI TVM[^1]         |v2.6.0     |
| Evaluation Board       |RZ/V2L EVK |
| DRP-AI Translator      |v1.90      |
| Linux Package          |v3.0.6     |
| DRP-AI Support Package |v7.50      |

| AI model                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            |Input Shape    | Task              | Format               | Inference time<br>(CPU only) | Inference time<br>(CPU+DRP-AI)  |
|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|---------------|-------------------|----------------------|------------------------------|----------------------------------|
|[ResNet18-v1](https://github.com/onnx/models/blob/main/validated/vision/classification/resnet/model/resnet18-v1-7.onnx)|(224,224)|Classification|ONNX|3724ms|24ms|
|[ResNet18-v2](https://github.com/onnx/models/blob/main/validated/vision/classification/resnet/model/resnet18-v2-7.onnx)|(224,224)|Classification|ONNX|3706ms|26ms|
|[ResNet34-v1](https://github.com/onnx/models/blob/main/validated/vision/classification/resnet/model/resnet34-v1-7.onnx)|(224,224)|Classification|ONNX|8160ms|40ms|
|[ResNet34-v2](https://github.com/onnx/models/blob/main/validated/vision/classification/resnet/model/resnet34-v2-7.onnx)|(224,224)|Classification|ONNX|8162ms|43ms|
|[ResNet50-v1](https://github.com/onnx/models/blob/main/validated/vision/classification/resnet/model/resnet50-v1-7.onnx)|(224,224)|Classification|ONNX|7841ms|57ms|
|[ResNet50-v2](https://github.com/onnx/models/blob/main/validated/vision/classification/resnet/model/resnet50-v2-7.onnx)|(224,224)|Classification|ONNX|8442ms|68ms|
|[ResNet101-v1](https://github.com/onnx/models/blob/main/validated/vision/classification/resnet/model/resnet101-v1-7.onnx)|(224,224)|Classification|ONNX|16893ms|94ms|
|[ResNet101-v2](https://github.com/onnx/models/blob/main/validated/vision/classification/resnet/model/resnet101-v2-7.onnx)|(224,224)|Classification|ONNX|17518ms|110ms|
|[MobileNetV2](https://github.com/onnx/models/blob/main/validated/vision/classification/mobilenet/model/mobilenetv2-7.onnx)|(224,224)|Classification|ONNX|646ms|22ms|
|[SqueezeNet1.1-7](https://github.com/onnx/models/blob/main/validated/vision/classification/squeezenet/model/squeezenet1.1-7.onnx)|(224,224)|Classification|ONNX|609ms|9ms|
|[DenseNet9](https://github.com/onnx/models/blob/main/validated/vision/classification/densenet-121/model/densenet-9.onnx)|(224,224)|Classification|ONNX|5070ms|188ms|
|[Inception-v1](https://github.com/onnx/models/blob/main/validated/vision/classification/inception_and_googlenet/inception_v1/model/inception-v1-9.onnx)|(224,224)|Classification|ONNX|2812ms|609ms|
|[Inception-v2](https://github.com/onnx/models/blob/main/validated/vision/classification/inception_and_googlenet/inception_v2/model/inception-v2-9.onnx)|(224,224)|Classification|ONNX|3537ms|118ms|
|[YOLOv2](https://github.com/onnx/models/blob/main/validated/vision/object_detection_segmentation/yolov2-coco/model/yolov2-coco-9.onnx)|(416,416)|Object Detection|ONNX|31036ms|121ms|
|[YOLOv3](./how_to_convert/How_to_convert_yolov3_onnx_model_V2L_V2M_V2MA.md)|(416,416)|Object Detection|ONNX|67869ms|409ms|
|[YOLOv5l](./how_to_convert/How_to_convert_yolov5_onnx_models_V2L_V2M_V2MA.md)|(320,320)|Object Detection|ONNX|26255ms|431ms|
|[YOLOv5m](./how_to_convert/How_to_convert_yolov5_onnx_models_V2L_V2M_V2MA.md)|(320,320)|Object Detection|ONNX|7237ms|450ms|
|[YOLOv5n](./how_to_convert/How_to_convert_yolov5_onnx_models_V2L_V2M_V2MA.md)|(320,320)|Object Detection|ONNX|966ms|140ms|
|[YOLOv5s](./how_to_convert/How_to_convert_yolov5_onnx_models_V2L_V2M_V2MA.md)|(320,320)|Object Detection|ONNX|4025ms|211ms|
|[YOLOv5x](./how_to_convert/How_to_convert_yolov5_onnx_models_V2L_V2M_V2MA.md)|(320,320)|Object Detection|ONNX|23722ms|609ms|
|[YOLOv6N](./how_to_convert/How_to_convert_yolov6_onnx_models_V2L_V2M_V2MA.md)|(640,640)|Object Detection|ONNX|9093ms|449ms|
|[YOLOv6S](./how_to_convert/How_to_convert_yolov6_onnx_models_V2L_V2M_V2MA.md)|(640,640)|Object Detection|ONNX|35571ms|1567ms|
|[YOLOv6M](./how_to_convert/How_to_convert_yolov6_onnx_models_V2L_V2M_V2MA.md)|(640,640)|Object Detection|ONNX|65631ms|3426ms|
|[YOLOv6L](./how_to_convert/How_to_convert_yolov6_onnx_models_V2L_V2M_V2MA.md)|(320,320)|Object Detection|ONNX|36894ms|1573ms|
|[YOLOv7](./how_to_convert/How_to_convert_yolov7_onnx_models_V2L_V2M_V2MA.md)|(320,320)|Object Detection|ONNX|23081ms|209ms|
|[YOLOv7X](./how_to_convert/How_to_convert_yolov7_onnx_models_V2L_V2M_V2MA.md)|(320,320)|Object Detection|ONNX|35193ms|314ms|
|[YOLOv7W6](./how_to_convert/How_to_convert_yolov7_onnx_models_V2L_V2M_V2MA.md)|(320,320)|Object Detection|ONNX|18118ms|1611ms|
|[YOLOv7E6](./how_to_convert/How_to_convert_yolov7_onnx_models_V2L_V2M_V2MA.md)|(320,320)|Object Detection|ONNX|22875ms|2401ms|
|[YOLOv7D6](./how_to_convert/How_to_convert_yolov7_onnx_models_V2L_V2M_V2MA.md)|(320,320)|Object Detection|ONNX|T.B.D|3126ms|
|[YOLOv7E6E](./how_to_convert/How_to_convert_yolov7_onnx_models_V2L_V2M_V2MA.md)|(320,320)|Object Detection|ONNX|T.B.D|3714ms|
|[YOLOv8n](./how_to_convert/How_to_convert_yolov8_onnx_models_V2L_V2M_V2MA.md)|(640,640)|Object Detection|ONNX|5875ms|351ms|
|[YOLOv8s](./how_to_convert/How_to_convert_yolov8_onnx_models_V2L_V2M_V2MA.md)|(640,640)|Object Detection|ONNX|21827ms|554ms|
|[YOLOv8m](./how_to_convert/How_to_convert_yolov8_onnx_models_V2L_V2M_V2MA.md)|(640,640)|Object Detection|ONNX|55289ms|1033ms|
|[YOLOv8l](./how_to_convert/How_to_convert_yolov8_onnx_models_V2L_V2M_V2MA.md)|(320,320)|Object Detection|ONNX|39103ms|443ms|
|[YOLOv8x](./how_to_convert/How_to_convert_yolov8_onnx_models_V2L_V2M_V2MA.md)|(320,320)|Object Detection|ONNX|29143ms|607ms|
|[YOLOX_s](./how_to_convert/How_to_convert_yolox_onnx_models_V2L_V2M_V2MA.md)|(640,640)|Object Detection|ONNX|20367ms|229ms|
|[YOLOX_m](./how_to_convert/How_to_convert_yolox_onnx_models_V2L_V2M_V2MA.md)|(640,640)|Object Detection|ONNX|54496ms|437ms|
|[YOLOX_l](./how_to_convert/How_to_convert_yolox_onnx_models_V2L_V2M_V2MA.md)|(320,320)|Object Detection|ONNX|38733ms|199ms|
|[YOLOX_x](./how_to_convert/How_to_convert_yolox_onnx_models_V2L_V2M_V2MA.md)|(320,320)|Object Detection|ONNX|32114ms|355ms|
|[EdgeYOLO](./how_to_convert/How_to_convert_edgeyolo_models_onnx_V2L_V2M_V2MA.md)|(320,320)|Object Detection|ONNX|28377ms|177ms|
|[EdgeYOLO_M](./how_to_convert/How_to_convert_edgeyolo_models_onnx_V2L_V2M_V2MA.md)|(320,320)|Object Detection|ONNX|9076ms|129ms|
|[EdgeYOLO_S](./how_to_convert/How_to_convert_edgeyolo_models_onnx_V2L_V2M_V2MA.md)|(640,640)|Object Detection|ONNX|25818ms|584ms|
|[EdgeYOLO_Tiny](./how_to_convert/How_to_convert_edgeyolo_models_onnx_V2L_V2M_V2MA.md)|(640,640)|Object Detection|ONNX|19746ms|475ms|
|[HRNet](./how_to_convert/How_to_convert_hrnet_onnx_model_V2L_V2M_V2MA.md)|(256,192)|Body Keypiont 2D|ONNX|7581ms|105ms|
|[ResNet18](./how_to_convert/How_to_convert_torchvision_models_V2L_V2M_V2MA.md)|(224,224)|Classification|PyTorch|3707ms|24ms|
|[ResNet34](./how_to_convert/How_to_convert_torchvision_models_V2L_V2M_V2MA.md)|(224,224)|Classification|PyTorch|8150ms|40ms|
|[ResNet50](./how_to_convert/How_to_convert_torchvision_models_V2L_V2M_V2MA.md)|(224,224)|Classification|PyTorch|8389ms|59ms|
|[ResNet101](./how_to_convert/How_to_convert_torchvision_models_V2L_V2M_V2MA.md)|(224,224)|Classification|PyTorch|17433ms|96ms|
|[ResNeXt-50-32x4d](./how_to_convert/How_to_convert_torchvision_models_V2L_V2M_V2MA.md)|(224,224)|Classification|PyTorch|7371ms|445ms|
|[MobileNetV2](./how_to_convert/How_to_convert_torchvision_models_V2L_V2M_V2MA.md)|(224,224)|Classification|PyTorch|524ms|29ms|
|[SqueezeNet1_1](./how_to_convert/How_to_convert_torchvision_models_V2L_V2M_V2MA.md)|(227,227)|Classification|PyTorch|694ms|61ms|
|[DenseNet-121](./how_to_convert/How_to_convert_torchvision_models_V2L_V2M_V2MA.md)|(224,224)|Classification|PyTorch|5479ms|272ms|
|[DenseNet-161](./how_to_convert/How_to_convert_torchvision_models_V2L_V2M_V2MA.md)|(224,224)|Classification|PyTorch|12633ms|1085ms|
|[GoogleNet](./how_to_convert/How_to_convert_torchvision_models_V2L_V2M_V2MA.md)|(224,224)|Classification|PyTorch|2524ms|226ms|
|[MnasNet0_5](./how_to_convert/How_to_convert_torchvision_models_V2L_V2M_V2MA.md)|(224,224)|Classification|PyTorch|191ms|38ms|
|[DeepLabv3-resnet50](./how_to_convert/How_to_convert_torchvision_models_V2L_V2M_V2MA.md)|(224,224)|Segmentation|PyTorch|53187ms|312ms|
|[DeepLabv3-resnet101](./how_to_convert/How_to_convert_torchvision_models_V2L_V2M_V2MA.md)|(224,224)|Segmentation|PyTorch|81240ms|513ms|
|[FCN_resnet101](./how_to_convert/How_to_convert_torchvision_models_V2L_V2M_V2MA.md)|(224,224)|Segmentation|PyTorch|63854ms|494ms|
|[DeepPose](./how_to_convert/How_to_convert_mmpose_models_V2L_V2M_V2MA.md)|(256,192)|Body Keypoint 2D|PyTorch|5717ms|57ms|
|[HRNetV2](./how_to_convert/How_to_convert_mmpose_models_V2L_V2M_V2MA.md)|(256,192)|Face Detection 2D|PyTorch|3549ms|111ms|
|[HRNetV2 DarkPose](./how_to_convert/How_to_convert_mmpose_models_V2L_V2M_V2MA.md)|(256,192)|PyTorch|Face Detection 2D|3542ms|111ms|
|[Monodepth2 mono_640x192 encoder](./how_to_convert/How_to_convert_monodepth2_model_V2L_V2M_V2MA.md)|(640,192)|Depth|PyTorch|6532ms|65ms|
|[SC-Depth resnet18_depth_256 dispnet](./how_to_convert/How_to_convert_sc_depth_resnet_models_V2L_V2M_V2MA.md)|(832,256)|Depth|PyTorch|18277ms|884ms|
|[SC-Depth resnet50_depth_256 dispnet](./how_to_convert/How_to_convert_sc_depth_resnet_models_V2L_V2M_V2MA.md)|(416,128)|Depth|PyTorch|9807ms|1034ms|
|[ConvNeXt atto](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|1094ms|319ms|
|[ConvNeXt femto](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|1375ms|399ms|
|[ConvNeXt femto ols](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|1414ms|392ms|
|[CSP-Darknet](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(256,256)|Classification|pytorch-image-models|7502ms|156ms|
|[CSP-ResNet](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(256,256)|Classification|pytorch-image-models|5554ms|106ms|
|[CSP-ResNeXt](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|6142ms|425ms|
|[Darknet-53](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(228,228)|Classification|pytorch-image-models|12719ms|86ms|
|[Darknet-aa53](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(228,228)|Classification|pytorch-image-models|13182ms|108ms|
|[DenseNet121](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|5479ms|272ms|
|[DenseNet161](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|12633ms|1085ms|
|[DenseNet169](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|6962ms|412ms|
|[DenseNet201](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|9804ms|677ms|
|[DenseNet Blur 121d](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|5746ms|284ms|
|[DLA(Dense Layer Aggregation)102x](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|13925ms|774ms|
|[DLA102x2](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|17984ms|1318ms|
|[DLA46x_c](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|725ms|110ms|
|[DLA60x_c](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|871ms|125ms|
|[DPN(Dual Path Network)107](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|21598ms|T.B.D|
|[DPN68](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|4671ms|578ms|
|[DPN68b](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|5021ms|547ms|
|[ECA-ResNet101d](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|17666ms|378ms|
|[ECA-ResNet26t](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(320,320)|Classification|pytorch-image-models|8527ms|255ms|
|[ECA-ResNet50d](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|8618ms|229ms|
|[ECA-ResNet50t](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(320,320)|Classification|pytorch-image-models|15335ms|435ms|
|[ECA-ResNetlight](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|9352ms|182ms|
|[EfficientNet Edge Large](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(300,300)|Classification|pytorch-image-models|13520ms|468ms|
|[pruned EfficientNet Edge Large](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(240,240)|Classification|pytorch-image-models|7273ms|233ms|
|[EfficientNet Edge Medium](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(300,300)|Classification|pytorch-image-models|7139ms|322ms|
|[EfficientNet Edge Small](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|2979ms|124ms|
|[pruned EfficientNet Edge Small](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|2968ms|123ms|
|[EfficientNet Lite0](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|605ms|85ms|
|[Ensemble Adversarial Inception ResNet v2](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|9902ms|1594ms|
|[ESE-VoVNet 19-dw](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|2327ms|103ms|
|[ESE-VoVNet 39b](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|12653ms|153ms|
|[FBNet-C](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|739ms|102ms|
|[FBNetV3-B](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(256,256)|Classification|pytorch-image-models|1178ms|265ms|
|[FBNetV3-D](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(256,256)|Classification|pytorch-image-models|1354ms|195ms|
|[FBNetV3-G](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(288,288)|Classification|pytorch-image-models|3457ms|578ms|
|[Global Context Resnet50t (gcresnet50t)](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(256,256)|Classification|pytorch-image-models|6634ms|218ms|
|[GPU-Efficient ResNet Large (gernet_l)](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(256,256)|Classification|pytorch-image-models|5851ms|62ms|
|[GPU-Efficient ResNet Middle (gernet_m)](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|4514ms|40ms|
|[GPU-Efficient ResNet Small (gernet_s)](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|895ms|16ms|
|[GhostNet-1.0x](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|259ms|98ms|
|[(Gluon) ResNet101 v1b](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|17403ms|96ms|
|[(Gluon) ResNet101 v1c](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|17608ms|96ms|
|[(Gluon) ResNet101 v1d](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|17607ms|124ms|
|[(Gluon) ResNet101 v1s](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|18610ms|102ms|
|[(Gluon) ResNet152 v1b](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|25827ms|133ms|
|[(Gluon) ResNet152 v1c](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|26038ms|133ms|
|[(Gluon) ResNet152 v1d](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|26109ms|160ms|
|[(Gluon) ResNet152 v1s](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|27039ms|139ms|
|[(Gluon) ResNet18 v1b](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|3707ms|24ms|
|[(Gluon) ResNet34 v1b](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|8144ms|40ms|
|[(Gluon) ResNet50 v1b](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|8387ms|59ms|
|[(Gluon) ResNet50 v1c](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|8599ms|59ms|
|[(Gluon) ResNet50 v1d](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|8587ms|88ms|
|[(Gluon) ResNet50 v1s](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|9598ms|65ms|
|[(Gluon) ResNeXt101 32x4d](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|14195ms|818ms|
|[(Gluon) ResNeXt101 64x4d](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|18239ms|1477ms|
|[(Gluon) SENet154](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|31492ms|T.B.D|
|[(Gluon) SE-ResNeXt101 32-4d](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|14330ms|993ms|
|[(Gluon) SE-ResNeXt101 64-4d](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|18394ms|1695ms|
|[(Gluon) SE-ResNeXt50 32-4d](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|7401ms|546ms|
|[HardcoreNAS_A](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|334ms|113ms|
|[HardcoreNAS_B](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|379ms|90ms|
|[HardcoreNAS_C](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|396ms|109ms|
|[HardcoreNAS_D](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|439ms|120ms|
|[HardcoreNAS_E](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|500ms|153ms|
|[HardcoreNAS_F](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|500ms|144ms|
|[HRNet w18](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|7041ms|152ms|
|[HRNet w18small](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|2891ms|33ms|
|[HRNet w18small V2](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|4516ms|77ms|
|[HRNet w30](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|10331ms|167ms|
|[HRNet w32](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|14818ms|123ms|
|[HRNet w40](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|14158ms|188ms|
|[HRNet w44](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|15815ms|228ms|
|[HRNet w48](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|22893ms|200ms|
|[Instagram ResNeXt101 32x8 WSL](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|19170ms|2415ms|
|[Inception ResNet v2](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|9906ms|1510ms|
|[PP-LCNet-0.5x](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|126ms|26ms|
|[PP-LCNet-0.75x](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|215ms|35ms|
|[PP-LCNet-1x](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|423ms|42ms|
|[(Legacy) SENet-154](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|31511ms|T.B.D|
|[(Legacy) SE-ResNet-152](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|25396ms|573ms|
|[(Legacy) SE-ResNet-18](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|3702ms|77ms|
|[(Legacy) SE-ResNet-34](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|8158ms|118ms|
|[(Legacy) SE-ResNet-50](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|7862ms|244ms|
|[(Legacy) SE-ResNeXt-26](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|4080ms|318ms|
|[MnasNet-B1 depth multiplier 1.0](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|516ms|64ms|
|[MnasNet-Small depth multiplier 1.0](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classifications|pytorch-image-models|166ms|32ms|
|[MobileNet V2 with channel multiplier of 0.5](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|220ms|20ms|
|[MobileNet V2 with channel multiplier of 1.0](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|524ms|29ms|
|[MobileNet V2 with channel multiplier of 1.1](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|795ms|38ms|
|[MobileNet V2 with channel multiplier of 1.2](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|1106ms|50ms|
|[MobileNet V2 with channel multiplier of 1.4](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|1100ms|41ms|
|[MobileNet V3 Large 1.0](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|316ms|65ms|
|[MobileNet V3 Large 1.0,  21k pretraining](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|467ms|73ms|
|[MobileNet V3 (RW variant)](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|325ms|65ms|
|[MobileNet V3 Small 0.5](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|57ms|33ms|
|[MobileNet V3 Small 0.75](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|86ms|40ms|
|[MobileNet V3 Small 1.0](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|103ms|44ms|
|[RegNetX 200MF](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|436ms|63ms|
|[RegNetX 400MF](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|424ms|153ms|
|[RegNetX 600MF](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|894ms|239ms|
|[RegNetX 800MF](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|1226ms|234ms|
|[RegNetX 1.6GF](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|1829ms|537ms|
|[RegNetX 3.2GF](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|5203ms|1448ms|
|[RegNetX 4.0GF](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|4412ms|1335ms|
|[RegNetX 6.4GF](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|7254ms|2280ms|
|[RegNetX 8.0GF](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|11745ms|3633ms|
|[RegNetX 16GF](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|25715ms|8128ms|
|[RegNetY 200MF](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|448ms|65ms|
|[RegNetY 400MF](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|647ms|121ms|
|[RegNetY 600MF](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|1327ms|202ms|
|[RegNetY 800MF](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|1017ms|254ms|
|[RegNetY 1.6GF](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|1836ms|601ms|
|[RegNetY 4.0GF](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|7150ms|1981ms|
|[RegNetY 8.0GF](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|15244ms|2537ms|
|[RegNetY 16GF](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|18903ms|6256ms|
|[RegNetY 32GF](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|T.B.D|14068ms|
|[RepVGG-A2](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|6553ms|96ms|
|[RepVGG-B0](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|8004ms|77ms|
|[RepVGG-B1](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|34080ms|160ms|
|[RepVGG-B1g4](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|18501ms|1918ms|
|[RepVGG-B2](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|44078ms|233ms|
|[RepVGG-B2g4](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|24694ms|2999ms|
|[RepVGG-B3](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|34368ms|288ms|
|[RepVGG-B3g4](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|20862ms|4191ms|
|[Res2Net-101 26w×4s](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|9509ms|407ms|
|[Res2Net-50 14w×8s](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|5939ms|318ms|
|[Res2Net-50 26w×4s](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|5760ms|248ms|
|[Res2Net-50 26w×6s](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|7572ms|356ms|
|[Res2Net-50 48w×2s](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|5827ms|207ms|
|[Res2Next-50](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|7355ms|466ms|
|[ResNeSt-101](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(256,256)|Classification|pytorch-image-models|14359ms|6432ms|
|[ResNeSt-14](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|3621ms|1655ms|
|[ResNeSt-200](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|47594ms|T.B.D|
|[ResNeSt-269](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(416,416)|Classification|pytorch-image-models|103200ms|T.B.D|
|[ResNeSt-26](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|4920ms|2184ms|
|[ResNeSt-50](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|7476ms|3203ms|
|[ResNeSt-50 1s4×24d](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|5693ms|1284ms|
|[ResNeSt-50 4s2×40d](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|5638ms|1749ms|
|[ResNet-101](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|17433ms|96ms|
|[ResNet-101-D](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(320,320)|Classification|pytorch-image-models|28752ms|222ms|
|[ResNet-10-T](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|1900ms|22ms|
|[ResNet-14-T](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|2988ms|53ms|
|[ResNet-152](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|25868ms|133ms|
|[ResNet-152-D](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(320,320)|Classification|pytorch-image-models|43279ms|288ms|
|[ResNet-18](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|3707ms|24ms|
|[ResNet-18-D](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|3891ms|32ms|
|[ResNet-200-D](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(320,320)|Classification|pytorch-image-models|60449ms|360ms|
|[ResNet-26](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|4604ms|38ms|
|[ResNet-26-D](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|4798ms|66ms|
|[ResNet-26-T](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(256,256)|Classification|pytorch-image-models|4209ms|82ms|
|[ResNet-34](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|8150ms|40ms|
|[ResNet-34-D](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|8335ms|47ms|
|[ResNet-50](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|8389ms|59ms|
|[ResNet-50-D](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|8582ms|87ms|
|[ResNet-50 avgpool anti-aliasing](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(288,288)|Classification|pytorch-image-models|11829ms|136ms|
|[ResNet-50 blur anti-aliasing](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|10892ms|98ms|
|[ResNet-RS-101](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(288,288)|Classification|pytorch-image-models|15100ms|618ms|
|[ResNet-RS-152](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(320,320)|Classification|pytorch-image-models|43568ms|T.B.D|
|[ResNet-RS-200](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(320,320)|Classification|pytorch-image-models|60704ms|T.B.D|
|[ResNet-RS-50](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|8706ms|240ms|
|[ResNet101-v2](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|17488ms|110ms|
|[ResNet50-v2](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|8409ms|68ms|
|[ResNeXt-101(32x8d)](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|19159ms|2445ms|
|[ResNeXt-101(64x4d)](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(288,288)|Classification|pytorch-image-models|27564ms|2312ms|
|[ResNeXt-50(32x4d)](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|7371ms|445ms|
|[ResNeXt-50d(32x4d)](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|7555ms|454ms|
|[SelecSLS42_B](./how_to_convert/How_to_convert_timm_models_V2L_V2M_V2MA.md)|(224,224)|Classification|pytorch-image-models|3156ms|46ms|



[^1]: DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.  
