# Model list

Below is a list of AI models that Renesas has verified for conversion with the DRP-AI TVM[^1] and actual operation on an evaluation board.

| Item                   | RZ/V2L     | RZ/V2M     | RZ/V2MA     |
|------------------------|------------|------------|-------------|
| DRP-AI TVM[^1]         | v1.1.1     | v1.1.1     | v1.1.1      |
| Evaluation Board       | RZ/V2L EVK | RZ/V2M EVK | RZ/V2MA EVK |
| DRP-AI Translator      | v1.82      | v1.82      | v1.82       |
| Linux Package          | v3.0.2     | v1.3.0     | v1.0.0      |
| DRP-AI Support Package | v7.30      | v7.30      | v7.20       |

| AI model                                                                                                                  | Task              | Format                                                                   | Inference time<br>(CPU only@V2L) | Inference time<br>(CPU+DRP-AI@V2L) | Inference time<br>(CPU only@V2M) | Inference time<br>(CPU+DRP-AI@V2M) | Inference time<br>(CPU only@V2MA) | Inference time<br>(CPU+DRP-AI@V2MA) |
| ------------------------------------------------------------------------------------------------------------------------- | ----------------- | ------------------------------------------------------------------------ |----------------------------------|------------------------------------|----------------------------------|------------------------------------| --------------------------------- | ----------------------------------- |
|ResNet18-v1                                |Classification   |ONNX                |12078ms |28ms  |29135ms |18ms  |14579ms |18ms  |
|ResNet18-v2                                |Classification   |ONNX                |12080ms |30ms  |29219ms |19ms  |14619ms |19ms  |
|ResNet34-v1                                |Classification   |ONNX                |27174ms |44ms  |65626ms |27ms  |32818ms |27ms  |
|ResNet34-v2                                |Classification   |ONNX                |27202ms |47ms  |65909ms |30ms  |32941ms |29ms  |
|ResNet50-v1                                |Classification   |ONNX                |24584ms |61ms  |59407ms |36ms  |29741ms |36ms  |
|ResNet50-v2                                |Classification   |ONNX                |26321ms |72ms  |63818ms |46ms  |31954ms |46ms  |
|ResNet101-v1                               |Classification   |ONNX                |52645ms |98ms  |127350ms|57ms  |63697ms |57ms  |
|ResNet101-v2                               |Classification   |ONNX                |54097ms |114ms |131248ms|71ms  |65654ms |71ms  |
|MobileNetV2                                |Classification   |ONNX                |1453ms  |26ms  |3479ms  |16ms  |1747ms  |16ms  |
|SqueezeNet1.1-7                            |Classification   |ONNX                |1836ms  |42ms  |4450ms  |34ms  |2252ms  |36ms  |
|DenseNet9                                  |Classification   |ONNX                |14570ms |197ms |35207ms |183ms |17682ms |186ms |
|Inception-v1                               |Classification   |ONNX                |8842ms  |595ms |21373ms |1123ms|10840ms |650ms |
|Inception-v2                               |Classification   |ONNX                |9919ms  |148ms |23902ms |147ms |12067ms |149ms |
|YOLOv2                                     |Object Detection |ONNX                |117936ms|139ms |292045ms|87ms  |146062ms|87ms  |
|YOLOv3                                     |Object Detection |ONNX                |250694ms|360ms |618007ms|250ms |308293ms|242ms |
|YOLOv5l                                    |Object Detection |ONNX                |253066ms|516ms |621655ms|803ms |310838ms|538ms |
|YOLOv5m                                    |Object Detection |ONNX                |84428ms |1945ms|202625ms|3629ms|101566ms|2214ms|
|YOLOv5n                                    |Object Detection |ONNX                |9191ms  |656ms |22130ms |1046ms|11298ms |771ms |
|YOLOv5s                                    |Object Detection |ONNX                |41747ms |1008ms|102031ms|1704ms|51231ms |1163ms|
|YOLOv5x                                    |Object Detection |ONNX                |47531ms |749ms |115046ms|1149ms|57517ms |756ms |
|YOLOv6N                                    |Object Detection |ONNX                |27273ms |552ms |67314ms |1074ms|33668ms |607ms |
|YOLOv6S                                    |Object Detection |ONNX                |109878ms|1809ms|271105ms|3932ms|135391ms|2073ms|
|YOLOv6M                                    |Object Detection |ONNX                |188406ms|4119ms|461916ms|8877ms|230713ms|4704ms|
|YOLOv6L                                    |Object Detection |ONNX                |129326ms|1814ms|318325ms|3913ms|158958ms|2061ms|
|YOLOv7                                     |Object Detection |ONNX                |72876ms |382ms |178492ms|361ms |89287ms |364ms |
|YOLOv7X                                    |Object Detection |ONNX                |108063ms|525ms |264456ms|469ms |132316ms|470ms |
|YOLOv7W6                                   |Object Detection |ONNX                |T.B.D.  |T.B.D.|T.B.D.  |T.B.D.|T.B.D.  |T.B.D.|
|YOLOv7E6                                   |Object Detection |ONNX                |T.B.D.  |T.B.D.|T.B.D.  |T.B.D.|T.B.D.  |T.B.D.|
|YOLOv7D6                                   |Object Detection |ONNX                |T.B.D.  |T.B.D.|T.B.D.  |T.B.D.|T.B.D.  |T.B.D.|
|YOLOv7E6E                                  |Object Detection |ONNX                |T.B.D.  |T.B.D.|T.B.D.  |T.B.D.|T.B.D.  |T.B.D.|
|YOLOv8n                                    |Object Detection |ONNX                |14073ms |369ms |34307ms |393ms |17269ms |380ms |
|YOLOv8s                                    |Object Detection |ONNX                |64495ms |569ms |158456ms|585ms |79292ms |577ms |
|YOLOv8m                                    |Object Detection |ONNX                |147706ms|1020ms|356004ms|987ms |177888ms|971ms |
|YOLOv8l                                    |Object Detection |ONNX                |143141ms|447ms |352785ms|398ms |176179ms|399ms |
|YOLOv8x                                    |Object Detection |ONNX                |60873ms |594ms |147215ms|524ms |73570ms |522ms |
|YOLOX_s                                    |Object Detection |ONNX                |56540ms |402ms |138831ms|368ms |69598ms |373ms |
|YOLOX_m                                    |Object Detection |ONNX                |150285ms|686ms |361990ms|580ms |180987ms|587ms |
|YOLOX_l                                    |Object Detection |ONNX                |139775ms|289ms |344315ms|228ms |172028ms|229ms |
|YOLOX_x                                    |Object Detection |ONNX                |65489ms |463ms |157959ms|341ms |78918ms |344ms |
|EdgeYOLO                                   |Object Detection |ONNX                |94265ms |350ms |231173ms|312ms |115618ms|316ms |
|EdgeYOLO_M                                 |Object Detection |ONNX                |18092ms |253ms |43522ms |226ms |21921ms |228ms |
|EdgeYOLO_S                                 |Object Detection |ONNX                |51995ms |821ms |120844ms|855ms |62259ms |787ms |
|EdgeYOLO_Tiny                              |Object Detection |ONNX                |49574ms |661ms |115891ms|660ms |59519ms |630ms |
|HRNet                                      |Body Keypiont 2D |ONNX                |15207ms |106ms |36706ms |62ms  |18374ms |62ms  |
|ResNet18                                   |Classification   |PyTorch             |12069ms |28ms  |29074ms |17ms  |14549ms |17ms  |
|ResNet34                                   |Classification   |PyTorch             |27173ms |44ms  |65625ms |27ms  |32808ms |27ms  |
|ResNet50                                   |Classification   |PyTorch             |26325ms |63ms  |63799ms |38ms  |31952ms |38ms  |
|ResNet101                                  |Classification   |PyTorch             |54262ms |100ms |131499ms|58ms  |65818ms |58ms  |
|ResNeXt-50-32x4d                           |Classification   |PyTorch             |21555ms |472ms |52117ms |823ms |26111ms |496ms |
|MobileNetV2                                |Classification   |PyTorch             |1233ms  |32ms  |2919ms  |21ms  |1482ms  |21ms  |
|SqueezeNet1_1                              |Classification   |PyTorch             |1931ms  |89ms  |4662ms  |95ms  |2364ms  |92ms  |
|DenseNet-121                               |Classification   |PyTorch             |14760ms |326ms |35197ms |429ms |17976ms |431ms |
|DenseNet-161                               |Classification   |PyTorch             |30645ms |1563ms|72801ms |2459ms|37325ms |2061ms|
|GoogleNet                                  |Classification   |PyTorch             |7301ms  |269ms |29234ms |286ms |8866ms  |285ms |
|MnasNet0_5                                 |Classification   |PyTorch             |443ms   |41ms  |1004ms  |51ms  |528ms   |36ms  |
|DeepLabv3-resnet50                         |Segmentation     |PyTorch             |181872ms|340ms |443894ms|197ms |221695ms|198ms |
|DeepLabv3-resnet101                        |Segmentation     |PyTorch             |276868ms|541ms |677082ms|302ms |338204ms|301ms |
|FCN_resnet101                              |Segmentation     |PyTorch             |189265ms|498ms |463248ms|264ms |231282ms|264ms |
|DeepPose                                   |Body Keypoint 2D |PyTorch             |16974ms |61ms  |41038ms |36ms  |20542ms |36ms  |
|HRNetV2                                    |Face Detection 2D|PyTorch             |6841ms  |133ms |15735ms |71ms  |8117ms  |72ms  |
|HRNetV2 DarkPose                           |Face Detection 2D|PyTorch             |6831ms  |132ms |15742ms |72ms  |8123ms  |72ms  |
|Monodepth2 mono_640x192 encoder            |Depth            |PyTorch             |21259ms |74ms  |51774ms |55ms  |25892ms |51ms  |
|SC-Depth resnet18_depth_256 dispnet        |Depth            |PyTorch             |49419ms |885ms |121715ms|1313ms|61011ms |925ms |
|SC-Depth resnet50_depth_256 dispnet        |Depth            |PyTorch             |26831ms |1081ms|65763ms |2300ms|32908ms |1237ms|
|FaceNet                                    |Face Detection   |Tensorflow          |27875ms |422ms |80234ms |917ms |45130ms |481ms |
|ConvNeXt atto                              |Classification   |pytorch-image-models|12005ms |151ms |29235ms |118ms |14650ms |112ms |
|ConvNeXt femto                             |Classification   |pytorch-image-models|12015ms |151ms |29227ms |118ms |14665ms |113ms |
|ConvNeXt femto ols                         |Classification   |pytorch-image-models|12004ms |152ms |29228ms |118ms |14649ms |112ms |
|CSP-Darknet                                |Classification   |pytorch-image-models|11840ms |189ms |28880ms |146ms |14472ms |141ms |
|CSP-ResNet                                 |Classification   |pytorch-image-models|11847ms |189ms |28889ms |147ms |14461ms |141ms |
|CSP-ResNeXt                                |Classification   |pytorch-image-models|12005ms |152ms |29224ms |117ms |14651ms |112ms |
|Darknet-53                                 |Classification   |pytorch-image-models|28517ms |178ms |68345ms |147ms |34480ms |133ms |
|Darknet-aa53                               |Classification   |pytorch-image-models|28531ms |177ms |68345ms |147ms |34452ms |132ms |
|DenseNet121                                |Classification   |pytorch-image-models|12017ms |152ms |29231ms |119ms |14647ms |111ms |
|DenseNet161                                |Classification   |pytorch-image-models|12002ms |152ms |29231ms |118ms |14656ms |113ms |
|DenseNet169                                |Classification   |pytorch-image-models|12004ms |152ms |29227ms |117ms |14653ms |112ms |
|DenseNet201                                |Classification   |pytorch-image-models|11998ms |152ms |29228ms |119ms |14655ms |112ms |
|DenseNet Blur 121d                         |Classification   |pytorch-image-models|11999ms |152ms |29229ms |117ms |14658ms |112ms |
|DLA(Dense Layer Aggregation)102x           |Classification   |pytorch-image-models|12006ms |152ms |29227ms |118ms |14648ms |111ms |
|DLA102x2                                   |Classification   |pytorch-image-models|12001ms |152ms |29225ms |117ms |14658ms |112ms |
|DLA46x_c                                   |Classification   |pytorch-image-models|12001ms |152ms |29228ms |117ms |14651ms |112ms |
|DLA60x_c                                   |Classification   |pytorch-image-models|12001ms |152ms |29263ms |119ms |14649ms |112ms |
|DPN(Dual Path Network)107                  |Classification   |pytorch-image-models|12003ms |151ms |29230ms |117ms |14661ms |112ms |
|DPN68                                      |Classification   |pytorch-image-models|12003ms |152ms |29226ms |117ms |14650ms |112ms |
|DPN68b                                     |Classification   |pytorch-image-models|12001ms |152ms |29232ms |118ms |14650ms |112ms |
|ECA-ResNet101d                             |Classification   |pytorch-image-models|12036ms |151ms |29222ms |118ms |14652ms |112ms |
|ECA-ResNet26t                              |Classification   |pytorch-image-models|22295ms |286ms |53528ms |224ms |26863ms |218ms |
|ECA-ResNet50d                              |Classification   |pytorch-image-models|12003ms |151ms |29229ms |118ms |14654ms |111ms |
|ECA-ResNet50t                              |Classification   |pytorch-image-models|22303ms |285ms |53528ms |224ms |26851ms |213ms |
|ECA-ResNet light                           |Classification   |pytorch-image-models|12002ms |152ms |29237ms |118ms |14655ms |112ms |
|EfficientNet Edge Large                    |Classification   |pytorch-image-models|26540ms |281ms |65111ms |230ms |32618ms |207ms |
|pruned EfficientNet Edge Large             |Classification   |pytorch-image-models|31062ms |178ms |74408ms |145ms |37524ms |134ms |
|EfficientNet Edge Medium                   |Classification   |pytorch-image-models|26527ms |281ms |65116ms |230ms |32599ms |208ms |
|EfficientNet Edge Small                    |Classification   |pytorch-image-models|12009ms |152ms |29233ms |117ms |14653ms |112ms |
|pruned EfficientNet Edge Small             |Classification   |pytorch-image-models|12011ms |152ms |29229ms |117ms |14651ms |112ms |
|EfficientNet Lite0                         |Classification   |pytorch-image-models|12000ms |152ms |29229ms |117ms |14663ms |112ms |
|Ensemble Adversarial Inception ResNet v2   |Classification   |pytorch-image-models|26554ms |285ms |65110ms |231ms |32612ms |207ms |
|ESE-VoVNet 19-dw                           |Classification   |pytorch-image-models|11998ms |152ms |29226ms |117ms |14651ms |112ms |
|ESE-VoVNet 39b                             |Classification   |pytorch-image-models|11998ms |151ms |29229ms |117ms |14646ms |112ms |
|FBNet-C                                    |Classification   |pytorch-image-models|11996ms |152ms |29225ms |118ms |14667ms |112ms |
|FBNetV3-B                                  |Classification   |pytorch-image-models|11854ms |190ms |28867ms |146ms |14488ms |141ms |
|FBNetV3-D                                  |Classification   |pytorch-image-models|11841ms |189ms |28870ms |147ms |14471ms |140ms |
|FBNetV3-G                                  |Classification   |pytorch-image-models|21263ms |241ms |51182ms |187ms |25660ms |178ms |
|Global Context Resnet50t (gcresnet50t)     |Classification   |pytorch-image-models|11832ms |189ms |28877ms |146ms |14472ms |140ms |
|GPU-Efficient ResNet Large (gernet_l)      |Classification   |pytorch-image-models|11847ms |189ms |29227ms |146ms |14470ms |141ms |
|GPU-Efficient ResNet Middle (gernet_m)     |Classification   |pytorch-image-models|12011ms |151ms |65111ms |117ms |14687ms |112ms |
|GPU-Efficient ResNet Small (gernet_s)      |Classification   |pytorch-image-models|12002ms |152ms |17495ms |119ms |14666ms |112ms |
|GhostNet-1.0x                              |Classification   |pytorch-image-models|12001ms |152ms |29226ms |117ms |14651ms |111ms |
|(Gluon) ResNet101 v1b                      |Classification   |pytorch-image-models|11995ms |152ms |29234ms |117ms |14671ms |111ms |
|(Gluon) ResNet101 v1c                      |Classification   |pytorch-image-models|12002ms |152ms |29228ms |118ms |14662ms |112ms |
|(Gluon) ResNet101 v1d                      |Classification   |pytorch-image-models|12018ms |151ms |29235ms |117ms |14679ms |111ms |
|(Gluon) ResNet101 v1s                      |Classification   |pytorch-image-models|12008ms |152ms |29231ms |117ms |14654ms |112ms |
|(Gluon) ResNet152 v1b                      |Classification   |pytorch-image-models|12011ms |152ms |29229ms |117ms |14653ms |112ms |
|(Gluon) ResNet152 v1c                      |Classification   |pytorch-image-models|12001ms |152ms |29232ms |118ms |14646ms |112ms |
|(Gluon) ResNet152 v1d                      |Classification   |pytorch-image-models|12004ms |152ms |29226ms |117ms |14662ms |111ms |
|(Gluon) ResNet152 v1s                      |Classification   |pytorch-image-models|12008ms |152ms |29235ms |117ms |14658ms |113ms |
|(Gluon) ResNet18 v1b                       |Classification   |pytorch-image-models|12006ms |152ms |29226ms |118ms |14647ms |112ms |
|(Gluon) ResNet34 v1b                       |Classification   |pytorch-image-models|11995ms |152ms |29232ms |119ms |14650ms |111ms |
|(Gluon) ResNet50 v1b                       |Classification   |pytorch-image-models|12001ms |152ms |29228ms |117ms |14663ms |112ms |
|(Gluon) ResNet50 v1c                       |Classification   |pytorch-image-models|12004ms |152ms |29221ms |118ms |14654ms |112ms |
|(Gluon) ResNet50 v1d                       |Classification   |pytorch-image-models|12008ms |152ms |29225ms |117ms |14665ms |112ms |
|(Gluon) ResNet50 v1s                       |Classification   |pytorch-image-models|12005ms |152ms |29228ms |119ms |14654ms |111ms |
|(Gluon) ResNeXt101 32x4d                   |Classification   |pytorch-image-models|12000ms |152ms |29229ms |118ms |14663ms |112ms |
|(Gluon) ResNeXt101 64x4d                   |Classification   |pytorch-image-models|12001ms |152ms |29227ms |117ms |14665ms |112ms |
|(Gluon) SENet154                           |Classification   |pytorch-image-models|12005ms |152ms |29234ms |118ms |14647ms |112ms |
|(Gluon) SE-ResNeXt101 32-4d                |Classification   |pytorch-image-models|12002ms |151ms |29227ms |118ms |14651ms |111ms |
|(Gluon) SE-ResNeXt101 64-4d                |Classification   |pytorch-image-models|12002ms |152ms |29228ms |118ms |14665ms |112ms |
|(Gluon) SE-ResNeXt50 32-4d                 |Classification   |pytorch-image-models|12014ms |152ms |29232ms |117ms |14663ms |111ms |
|(Gluon) Xception65                         |Classification   |pytorch-image-models|26519ms |284ms |28879ms |230ms |32591ms |208ms |
|HardcoreNAS_A                              |Classification   |pytorch-image-models|12003ms |152ms |29237ms |119ms |14647ms |112ms |
|HardcoreNAS_B                              |Classification   |pytorch-image-models|12002ms |152ms |29229ms |117ms |14661ms |112ms |
|HardcoreNAS_C                              |Classification   |pytorch-image-models|12002ms |152ms |29227ms |117ms |14646ms |111ms |
|HardcoreNAS_D                              |Classification   |pytorch-image-models|12000ms |152ms |29230ms |119ms |14662ms |111ms |
|HardcoreNAS_E                              |Classification   |pytorch-image-models|12012ms |152ms |29231ms |118ms |14659ms |112ms |
|HardcoreNAS_F                              |Classification   |pytorch-image-models|12010ms |151ms |29228ms |117ms |14654ms |112ms |
|HRNet w18                                  |Classification   |pytorch-image-models|12008ms |152ms |29236ms |118ms |14657ms |112ms |
|HRNet w18 small                            |Classification   |pytorch-image-models|12007ms |152ms |29233ms |118ms |14655ms |112ms |
|HRNet w18 small V2                         |Classification   |pytorch-image-models|12005ms |152ms |29227ms |118ms |14648ms |111ms |
|HRNet w30                                  |Classification   |pytorch-image-models|12000ms |152ms |29227ms |118ms |14663ms |112ms |
|HRNet w32                                  |Classification   |pytorch-image-models|12010ms |152ms |29232ms |117ms |14664ms |111ms |
|HRNet w40                                  |Classification   |pytorch-image-models|12013ms |151ms |29234ms |117ms |14658ms |111ms |
|HRNet w44                                  |Classification   |pytorch-image-models|11999ms |152ms |29224ms |117ms |14653ms |112ms |
|HRNet w48                                  |Classification   |pytorch-image-models|12016ms |152ms |29233ms |117ms |14649ms |113ms |
|Instagram ResNeXt101 32x8 WSL              |Classification   |pytorch-image-models|12007ms |152ms |29234ms |117ms |14662ms |112ms |
|Inception ResNet v2                        |Classification   |pytorch-image-models|11998ms |152ms |29226ms |117ms |14665ms |111ms |
|PP-LCNet-0.5x                              |Classification   |pytorch-image-models|12012ms |151ms |29232ms |118ms |14649ms |112ms |
|PP-LCNet-0.75x                             |Classification   |pytorch-image-models|12000ms |152ms |29231ms |118ms |14648ms |112ms |
|PP-LCNet-1x                                |Classification   |pytorch-image-models|11997ms |152ms |29227ms |119ms |14657ms |112ms |
|(Legacy) SENet-154                         |Classification   |pytorch-image-models|12002ms |152ms |29228ms |118ms |14652ms |112ms |
|(Legacy) SE-ResNet-152                     |Classification   |pytorch-image-models|11998ms |152ms |29228ms |118ms |14648ms |115ms |
|(Legacy) SE-ResNet-18                      |Classification   |pytorch-image-models|12002ms |152ms |29227ms |118ms |14645ms |112ms |
|(Legacy) SE-ResNet-34                      |Classification   |pytorch-image-models|12002ms |151ms |29227ms |117ms |14650ms |112ms |
|(Legacy) SE-ResNet-50                      |Classification   |pytorch-image-models|12000ms |152ms |29234ms |117ms |14657ms |111ms |
|(Legacy) SE-ResNeXt-26                     |Classification   |pytorch-image-models|12019ms |152ms |29235ms |118ms |14648ms |112ms |
|MnasNet-B1 depth multiplier 1.0            |Classification   |pytorch-image-models|12004ms |152ms |29229ms |117ms |14664ms |111ms |
|MnasNet-Small depth multiplier 1.0         |Classification   |pytorch-image-models|12005ms |152ms |29231ms |117ms |14652ms |112ms |
|MobileNet V2 with channel multiplier of 0.5|Classification   |pytorch-image-models|11999ms |151ms |29229ms |118ms |14647ms |112ms |
|MobileNet V2 with channel multiplier of 1.0|Classification   |pytorch-image-models|12010ms |152ms |29227ms |117ms |14660ms |113ms |
|MobileNet V2 with channel multiplier of 1.1|Classification   |pytorch-image-models|12014ms |152ms |29226ms |118ms |14648ms |111ms |
|MobileNet V2 with channel multiplier of 1.2|Classification   |pytorch-image-models|12001ms |151ms |29229ms |117ms |14650ms |111ms |
|MobileNet V2 with channel multiplier of 1.4|Classification   |pytorch-image-models|12003ms |152ms |29227ms |117ms |14648ms |111ms |
|MobileNet V3 Large 1.0                     |Classification   |pytorch-image-models|12022ms |152ms |29225ms |117ms |14654ms |113ms |
|MobileNet V3 Large 1.0, 21k pretraining    |Classification   |pytorch-image-models|12009ms |151ms |29233ms |117ms |14653ms |112ms |
|MobileNet V3 (RW variant)                  |Classification   |pytorch-image-models|12005ms |151ms |29250ms |117ms |14650ms |112ms |
|MobileNet V3 Small 0.5                     |Classification   |pytorch-image-models|12010ms |152ms |29224ms |117ms |14660ms |112ms |
|MobileNet V3 Small 0.75                    |Classification   |pytorch-image-models|11999ms |152ms |29228ms |118ms |14654ms |111ms |
|MobileNet V3 Small 1.0                     |Classification   |pytorch-image-models|12004ms |152ms |29225ms |117ms |14650ms |112ms |
|RegNetX 200MF                              |Classification   |pytorch-image-models|12000ms |152ms |29229ms |119ms |14662ms |113ms |
|RegNetX 400MF                              |Classification   |pytorch-image-models|12002ms |152ms |29228ms |119ms |14658ms |111ms |
|RegNetX 600MF                              |Classification   |pytorch-image-models|12005ms |151ms |29232ms |118ms |14647ms |112ms |
|RegNetX 800MF                              |Classification   |pytorch-image-models|12003ms |151ms |29231ms |117ms |14652ms |112ms |
|RegNetX 1.6GF                              |Classification   |pytorch-image-models|12009ms |152ms |29230ms |117ms |14665ms |113ms |
|RegNetX 3.2GF                              |Classification   |pytorch-image-models|12030ms |152ms |29230ms |117ms |14649ms |112ms |
|RegNetX 4.0GF                              |Classification   |pytorch-image-models|12014ms |152ms |29230ms |118ms |14660ms |111ms |
|RegNetX 6.4GF                              |Classification   |pytorch-image-models|11997ms |151ms |29230ms |118ms |14658ms |112ms |
|RegNetX 8.0GF                              |Classification   |pytorch-image-models|11998ms |152ms |29227ms |117ms |14679ms |112ms |
|RegNetX 16GF                               |Classification   |pytorch-image-models|12001ms |152ms |29225ms |117ms |14656ms |112ms |
|RegNetY 200MF                              |Classification   |pytorch-image-models|12002ms |152ms |29224ms |118ms |14649ms |112ms |
|RegNetY 400MF                              |Classification   |pytorch-image-models|12013ms |152ms |29234ms |119ms |14667ms |111ms |
|RegNetY 600MF                              |Classification   |pytorch-image-models|12001ms |152ms |29226ms |118ms |14646ms |112ms |
|RegNetY 800MF                              |Classification   |pytorch-image-models|12014ms |152ms |29227ms |118ms |14657ms |111ms |
|RegNetY 1.6GF                              |Classification   |pytorch-image-models|12006ms |151ms |29233ms |118ms |14645ms |111ms |
|RegNetY 4.0GF                              |Classification   |pytorch-image-models|12002ms |152ms |29241ms |117ms |14657ms |112ms |
|RegNetY 8.0GF                              |Classification   |pytorch-image-models|11997ms |151ms |29226ms |118ms |14650ms |111ms |
|RegNetY 16GF                               |Classification   |pytorch-image-models|11997ms |152ms |29232ms |117ms |14650ms |111ms |
|RegNetY 32GF                               |Classification   |pytorch-image-models|12009ms |152ms |29227ms |118ms |14652ms |111ms |
|RepVGG-A2                                  |Classification   |pytorch-image-models|12000ms |151ms |29233ms |117ms |14659ms |112ms |
|RepVGG-B0                                  |Classification   |pytorch-image-models|12000ms |152ms |29231ms |118ms |14649ms |112ms |
|RepVGG-B1                                  |Classification   |pytorch-image-models|12000ms |152ms |29229ms |118ms |14650ms |112ms |
|RepVGG-B1g4                                |Classification   |pytorch-image-models|12011ms |151ms |29228ms |117ms |14652ms |112ms |
|RepVGG-B2                                  |Classification   |pytorch-image-models|12003ms |152ms |29230ms |118ms |14662ms |112ms |
|RepVGG-B2g4                                |Classification   |pytorch-image-models|12002ms |152ms |29227ms |117ms |14650ms |112ms |
|RepVGG-B3                                  |Classification   |pytorch-image-models|12008ms |152ms |29229ms |117ms |14659ms |112ms |
|RepVGG-B3g4                                |Classification   |pytorch-image-models|12010ms |152ms |29240ms |117ms |14650ms |112ms |
|Res2Net-101 26w×4s                         |Classification   |pytorch-image-models|12006ms |151ms |29230ms |117ms |14664ms |111ms |
|Res2Net-50 14w×8s                          |Classification   |pytorch-image-models|12007ms |151ms |29226ms |117ms |14651ms |113ms |
|Res2Net-50 26w×4s                          |Classification   |pytorch-image-models|11999ms |152ms |29236ms |118ms |14657ms |111ms |
|Res2Net-50 26w×6s                          |Classification   |pytorch-image-models|12008ms |152ms |29231ms |118ms |14651ms |112ms |
|Res2Net-50 48w×2s                          |Classification   |pytorch-image-models|12011ms |151ms |29229ms |118ms |14645ms |113ms |
|Res2Next-50                                |Classification   |pytorch-image-models|11997ms |152ms |29228ms |117ms |14660ms |112ms |
|ResNeSt-101                                |Classification   |pytorch-image-models|11840ms |189ms |28879ms |148ms |14439ms |140ms |
|ResNeSt-14                                 |Classification   |pytorch-image-models|12008ms |151ms |29233ms |117ms |14655ms |112ms |
|ResNeSt-200                                |Classification   |pytorch-image-models|22295ms |286ms |53526ms |224ms |26848ms |213ms |
|ResNeSt-269                                |Classification   |pytorch-image-models|42102ms |468ms |102750ms|375ms |51451ms |353ms |
|ResNeSt-26                                 |Classification   |pytorch-image-models|11998ms |152ms |29228ms |118ms |14651ms |113ms |
|ResNeSt-50                                 |Classification   |pytorch-image-models|12001ms |152ms |29224ms |118ms |14651ms |112ms |
|ResNeSt-50 1s4×24d                         |Classification   |pytorch-image-models|12002ms |152ms |29229ms |118ms |14650ms |112ms |
|ResNeSt-50 4s2×40d                         |Classification   |pytorch-image-models|12000ms |152ms |29242ms |117ms |14648ms |112ms |
|ResNet-101                                 |Classification   |pytorch-image-models|12002ms |151ms |29227ms |119ms |14659ms |112ms |
|ResNet-101-D                               |Classification   |pytorch-image-models|22317ms |286ms |53533ms |224ms |26851ms |212ms |
|ResNet-10-T                                |Classification   |pytorch-image-models|11999ms |152ms |29231ms |118ms |14655ms |113ms |
|ResNet-14-T                                |Classification   |pytorch-image-models|11997ms |152ms |29226ms |118ms |14654ms |112ms |
|ResNet-152                                 |Classification   |pytorch-image-models|12004ms |152ms |29229ms |118ms |14645ms |112ms |
|ResNet-152-D                               |Classification   |pytorch-image-models|22304ms |287ms |53528ms |224ms |26859ms |214ms |
|ResNet-18                                  |Classification   |pytorch-image-models|12004ms |152ms |29229ms |119ms |14648ms |112ms |
|ResNet-18-D                                |Classification   |pytorch-image-models|11996ms |152ms |29227ms |117ms |14648ms |112ms |
|ResNet-200-D                               |Classification   |pytorch-image-models|22301ms |287ms |53535ms |224ms |26867ms |213ms |
|ResNet-26                                  |Classification   |pytorch-image-models|12005ms |152ms |29233ms |119ms |14666ms |112ms |
|ResNet-26-D                                |Classification   |pytorch-image-models|12013ms |152ms |29224ms |117ms |14657ms |112ms |
|ResNet-26-T                                |Classification   |pytorch-image-models|11838ms |189ms |28882ms |146ms |14474ms |141ms |
|ResNet-34                                  |Classification   |pytorch-image-models|12003ms |151ms |29227ms |118ms |14648ms |112ms |
|ResNet-34-D                                |Classification   |pytorch-image-models|12007ms |152ms |29230ms |117ms |14650ms |112ms |
|ResNet-50                                  |Classification   |pytorch-image-models|12009ms |152ms |29228ms |118ms |14648ms |111ms |
|ResNet-50-D                                |Classification   |pytorch-image-models|12007ms |151ms |29226ms |117ms |14646ms |112ms |
|ResNet-50 avgpool anti-aliasing            |Classification   |pytorch-image-models|21266ms |241ms |51172ms |186ms |25661ms |177ms |
|ResNet-50 blur anti-aliasing               |Classification   |pytorch-image-models|12001ms |151ms |29223ms |118ms |14648ms |111ms |
|ResNet-RS-101                              |Classification   |pytorch-image-models|21271ms |241ms |51171ms |186ms |25644ms |178ms |
|ResNet-RS-152                              |Classification   |pytorch-image-models|22316ms |287ms |53529ms |224ms |26859ms |214ms |
|ResNet-RS-200                              |Classification   |pytorch-image-models|22291ms |286ms |53520ms |226ms |26855ms |214ms |
|ResNet-RS-50                               |Classification   |pytorch-image-models|12002ms |152ms |29227ms |117ms |14658ms |112ms |
|ResNet101-v2                               |Classification   |pytorch-image-models|12000ms |152ms |29234ms |117ms |14667ms |120ms |
|ResNet50-v2                                |Classification   |pytorch-image-models|12005ms |152ms |29220ms |117ms |14661ms |112ms |
|ResNeXt-101(32x8d)                         |Classification   |pytorch-image-models|11999ms |152ms |29226ms |118ms |14649ms |112ms |
|ResNeXt-101(64x4d)                         |Classification   |pytorch-image-models|21258ms |242ms |51177ms |186ms |25651ms |176ms |
|ResNeXt-50(32x4d)                          |Classification   |pytorch-image-models|12011ms |151ms |29229ms |118ms |14658ms |112ms |
|ResNeXt-50d(32x4d)                         |Classification   |pytorch-image-models|12005ms |152ms |29235ms |118ms |14649ms |112ms |
|SelecSLS42_B                               |Classification   |pytorch-image-models|11998ms |152ms |29226ms |117ms |14658ms |112ms |





---

[^1]: DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.  
[^2]: Linux Package Version 1.1.0, DRP-AI Support Package Version 7.30 