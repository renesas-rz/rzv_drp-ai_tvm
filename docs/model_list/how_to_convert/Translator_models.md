# The onnx model list included with DRP-AI Translator i8
<!-- Below is a list of AI models supported by this manual. -->
DRP-AI Translator i8 includes the following sample onnx models. Please use them for the evaluation. 


| AI model | file name | Input Shape | Task | Pruning ratio |    
|---- | ---- | ----- | ---- | ---- |   
| Resnet50     | Resnetr50_v1_sparse90.onnx           | (224, 224)  | Classification          | 90%   |     
| YoloV2       | YoloV2_sparse90.onnx                 | (416, 416)  | Object Detection        | 90%   |    
| YoloX S      | YoloX-S_VOC_sparse.onnx              | (640, 640)  | Object Detection        | 70%   |
| DeepLabV3    | DeepLabV3_sparse90.onnx              | (513, 513)  | Semantic Segmentation   | 90%   |
| HRnet        | hrnet_w32_coco_256x192_sparse90.onnx | (256, 192)  | Pose Estimation (Single)| 90%   |
| YoloX-Pose S | yolox-pose_s_sample.onnx             |(640, 640)   | Pose Estimation (Multi) | Dense | 

Please check below folder:   
{DRP-AI_Translator_i8}/onnx_models/*

----