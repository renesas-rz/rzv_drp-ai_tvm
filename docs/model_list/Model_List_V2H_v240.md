# Model list for RZ/V2H

Below is a list of AI models that Renesas has verified for conversion with the DRP-AI TVM[^1] and actual operation on an evaluation board.

| Item                   | RZ/V2H      |
|------------------------|-------------|
| DRP-AI TVM[^1]         | v2.4.0      |
| Evaluation Board       | RZ/V2H EVK  |
| DRP-AI Translator      | i8 v1.03    |
| Linux Package          | AISDK v5.00 |
| DRP-AI Support Package | AISDK v5.00 |

[Note] Below Transformer models are compiled with "-f float32" option.

| AI model | Input Shape | Task | Format | Inference time<br>(CPU only) | Inference time<br>(CPU+DRP-AI) |
|-----|----|----|----|----|----|
| [ViT tiny ](./how_to_convert/How_to_convert_VIT_onnx_models_V2H.md)   |(224, 224)    | Classification    | ONNX  | 4,350.6 | 202.5 |  
| [ViT small ](./how_to_convert/How_to_convert_VIT_onnx_models_V2H.md)  |(224, 224)    | Classification    | ONNX  | 17,328.3 | 383.5 |
| [ViT base ](./how_to_convert/How_to_convert_VIT_onnx_models_V2H.md)   |(224, 224)    | Classification    | ONNX  | 60,311.4 | 769.4 |  

*1

[^1]: DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.  

