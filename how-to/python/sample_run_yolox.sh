#!/bin/sh

echo "Python API YOLOX sample script"
echo "[ Pre-processing ]"

python3 yolox/preproc.py --input_img yolox/dog.jpg \
    --bin_name yolox/input_0.bin

echo ""
echo "[ Inference ]"

python3 inference.py --model_path yolox/yolox_onnx \
    --input_shape 1,3,640,640 \
    --input_bin_file yolox/input_0.bin 

echo ""
echo "[ Post-processing ]" 

if [ "$1" = "with_cut" ]; then
    python3 yolox/postproc.py \
        --model_path yolox/yolox_onnx \
        --input_img_file yolox/dog.jpg  \
        --classes VOC \
        --with_cut
else
    python3 yolox/postproc.py \
        --model_path yolox/yolox_onnx \
        --input_img_file yolox/dog.jpg  \
        --classes COCO
fi

echo ""
