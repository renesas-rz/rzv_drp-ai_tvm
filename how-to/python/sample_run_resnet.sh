#!/bin/sh
echo "Python API ResNet sample script"
echo "[ Pre-processing ]"

python3 resnet/preproc.py --input_img resnet/sample.bmp \
    --bin_name resnet/input_0.bin

echo ""
echo "[ Inference ]"

python3 inference.py --model_path resnet/resnet18_onnx \
    --input_shape 1,3,224,224 \
    --input_bin_file resnet/input_0.bin 

echo ""
echo "[ Post-processing ]" 
python3 resnet/postproc.py \
    --output_path resnet/resnet18_onnx/output_0_fp16.bin

echo ""