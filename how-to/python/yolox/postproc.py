#!/usr/bin/env python3
# -*- coding:utf-8 -*-
# Copyright (c) Megvii, Inc. and its affiliates.
# Modified by Renesas Electronics Corporation, 2026.
#  - ONNX Runtime inference processing has been replaced with binary data loading.


import argparse
import os
import time

import cv2

from visualize import vis


import numpy as np
from coco_classes import COCO_CLASSES
from voc_classes import VOC_CLASSES
from demo_utils import demo_postprocess, multiclass_nms

IMAGE_EXT = [".jpg", ".jpeg", ".webp", ".bmp", ".png"]

def get_args():
    """
    Get Arguments
      model_path : path for runtime model data
      input_shape : input shape of model
      input_img_file : input image file to draw bounding boxes
      classes: dataset used for model training
      with_cut: flag to distinguish whether YOLOX contains decoding
    """
    parser = argparse.ArgumentParser()
    parser.add_argument("--model_path", default="yolox_s_onnx/",  help="Directory that stores output binary data.")
    parser.add_argument("--input_shape", default="640,640", help="User specified input shape of model. e.g. 640,640")
    parser.add_argument("--input_img_file", default="None", help="Input image file.")
    parser.add_argument("--classes", default="COCO", help="Dataset classes. [ COCO, VOC ]")
    parser.add_argument("--with_cut", action="store_true", help="Add this flag if the YOLOX contained post-processing is cut.")

    args = parser.parse_args()
    
    return args

if __name__ == "__main__":
    args = get_args()
    dataset = {
        "COCO": COCO_CLASSES,
        "VOC": VOC_CLASSES,
    }
    output_img_file = "result.jpg"

    print("Sample script for YOLOX")

    # Get model object directory and get output data files.
    model_path = args.model_path
    output_files = []
    # Check model_path exists
    if os.path.isdir(model_path):
        for name in os.listdir(model_path):
            # Store "model_path/output_*.bin" to output_files
            if name.startswith("output_") and name.endswith(".bin"):
                full_path = os.path.join(model_path, name)
                if os.path.isfile(full_path):
                    output_files.append(full_path)
    else:
        raise FileNotFoundError(f"Error: Model_path does not exist or is not a directory: {model_path}")
    # Output data files must be sorted from output_0*.bin ~
    output_files.sort()

    print(f"Load input data")
    # Check input image
    input_img_file = args.input_img_file
    if(input_img_file != "None"):
        print(f"  Load input image file : {input_img_file}")
        img_filename = input_img_file
    else:
        print(f"Error: Please specify input image file.")
        exit()

    # Get input shape of YOLOX
    _input_shape = args.input_shape
    model_in_shape = tuple([int(dim) for dim in _input_shape.split(',')])

    # Get classes information
    if args.classes not in dataset :
        raise KeyError(f"Key '{args.classes}' is not defined in classes")
    class_names = dataset[args.classes]
    num_classes = len(class_names)
    print(f"  Dataset               : {args.classes} with {num_classes} classes")

    # Read input image for display bounding box.
    img = cv2.imread(img_filename)
    img = cv2.resize(img, model_in_shape, interpolation=cv2.INTER_LINEAR)
    ratio = min(model_in_shape[0] / img.shape[0], model_in_shape[1] / img.shape[1]) # H, W

    model_out_shape = []
    if (not args.with_cut):
        # YOLOX repository
        model_out_shape.append((1, 8400, num_classes+5))
        print(f"  YOLOX output shape    : (1, 8400, {num_classes + 5})")
    else:
        model_out_shape.append((1, num_classes+5, 80*80)) # (1, num_classes+5, 80, 80)
        model_out_shape.append((1, num_classes+5, 40*40)) # (1, num_classes+5, 40, 40)
        model_out_shape.append((1, num_classes+5, 20*20)) # (1, num_classes+5, 20, 20)
        print(f"  YOLOX output 0 shape  : (1, {num_classes + 5}, 80, 80)")
        print(f"  YOLOX output 1 shape  : (1, {num_classes + 5}, 40, 40)")
        print(f"  YOLOX output 2 shape  : (1, {num_classes + 5}, 20, 20)")
    
    outputs = []
    for i, f in enumerate(output_files) :
        output_data = np.fromfile(f, np.float16).astype(np.float32)
        outputs.append(output_data.reshape(model_out_shape[i]))
        print(f"  AI inference output binary data: {f}")
    output = np.concatenate(outputs, 2)
    
    if (args.with_cut):
        output = output.transpose(0,2,1)       # Transpose to (1, 8400, num_classes+5)

    # Run post-processing
    predictions = demo_postprocess(output[0], model_in_shape)
    boxes = predictions[:, :4]
    scores = predictions[:, 4:5] * predictions[:, 5:]

    # Center coordinates to x0y0 , x1y1
    boxes_xyxy = np.ones_like(boxes)
    boxes_xyxy[:, 0] = boxes[:, 0] - boxes[:, 2]/2.
    boxes_xyxy[:, 1] = boxes[:, 1] - boxes[:, 3]/2.
    boxes_xyxy[:, 2] = boxes[:, 0] + boxes[:, 2]/2.
    boxes_xyxy[:, 3] = boxes[:, 1] + boxes[:, 3]/2.
    boxes_xyxy /= ratio

    dets = multiclass_nms(boxes_xyxy, scores, nms_thr=0.45, score_thr=0.1)
    result_img = img
    if dets is not None:
        final_boxes, final_scores, final_cls_inds = dets[:, :4], dets[:, 4], dets[:, 5]
        result_img = vis(img, final_boxes, final_scores, final_cls_inds,
                         conf=0.3, class_names=class_names)

    cv2.imwrite(output_img_file, result_img)
    print(f"Save output data")
    print(f"  Saved result image: {output_img_file}")

    exit()