[<- Installing DRP-AI Extention Package](./../../setup/README.md)

# How to use the DRP-AI Extension Pack

This document explains the contents of pruning then retraining by using the DRP-AI Extension Pack with Yolov8x provided by [ultralytics framework](https://github.com/ultralytics/ultralytics/tree/v8.3.163).

Please read [README.md](./../../setup/README.md) in advance and make docker image.

This tutorial will perform 5 steps like below.

> **Note:**  
> This chapter takes a long time to retrain the AI model.  
> If you only want to validate RZ/V2H operation, please skip this chapter.

<img src=./../../../img/flow-of-pruning-then-retraining.png width=600 alt="Flow of the DRP-AI Extension Pack">

### Step1. Run docker image
Please change to the directory where this README is located and then execute the following command.

```bash
cd $WORK/pruning-tool-examples/examples/ultralytics_yolov8x
docker run -it --rm --shm-size=32gb --gpus all -v $(pwd):/workspace -w /workspace drpai_ext_pt_img
```

The local `$(pwd)` is mounted to `/workspace` on the Docker container by the above command option.

For example, you can use this directory to copy files created on the Docker container to your local environment.

### Step2. Setup the training environment
In this chapter, install python libraries needed to run and create codes for pruning then retraing.

```bash
./setup.sh
ls
# Confirm the following folders and files.
# ultralytics/
# patch/
# setup.sh
# README.md
# eval_with_pruned_model.py
# modified_autobackend.py
# modified_loss.py
# modified_tasks.py
# modified_torch_utils.py
# modified_trainer.py
# onnx_validate.py
# retrain_with_pruning.py
# torch2onnx.py
```

Move to the working directory with the following command.

```bash
cd ultralytics
ls
# Confirm the following files.
# eval_with_pruned_model.py
# onnx_validate.py
# retrain_with_pruning.py
# torch2onnx.py
# ...
```

### Step3. Prepare the datasets
COCO datasets: The detail datasets at https://docs.ultralytics.com/datasets/detect/coco/

**When train or evaluate the model, the datasets are automatically downloaded.**

For example, the whole datasets after being automatically downloaded is shown below.

```bash
datasets/
└── coco
    ├── LICENSE
    ├── README.txt
    ├── annotations
    ├── images
    ├── labels
    ├── person_detection_results
    ├── test-dev2017.txt
    ├── train2017
    ├── train2017.txt
    └── val2017.txt
```

### Step3 & 4. Pruning then retraining
Pruning then retraining can be performed by executing the following command.
Running the following command will automatically download the pre-trained model.

```bash
python3 retrain_with_pruning.py --weight yolov8x.pt \ # for model weights path
                                --dataset coco.yaml \ # for dataset config
                                --imgsz 640 \ # for image size
                                --epochs 100 \ # for number of training epochs
                                --pruning_rate 0.7 # for pruning rate. This parameter can be in the range [0,1)
```

After retraining is finished, the following files can be confirmed in the `runs/detect/train/weights/` directory.

```bash
best.pt  last.pt
```

### Step5. Test the pruned model
In this chapter, the accuracy of the pruned model is measured to ensure that its accuracy is maintained after pruning then retraining.

Convert pytorch format model to onnx format model first.

```bash
python3 torch2onnx.py --weight runs/detect/train/weights/best.pt \ # for pruned model weights path
                      --imgsz 640 \ # for image size
                      --is_pruned_weight # if you convert the pruned weights

```

Test the pruned model with onnx format.

```bash
python3 onnx_validate.py --weight runs/detect/train/weights/best.onnx \ # for converted model output path
--dataset coco.yaml # for dataset config
```

Or using CLI

```bash
yolo detect val model=runs/detect/train/weights/best.onnx \ # for converted model output path 
data=coco.yaml
```

(Option) If test the pruned model with pytorch format, please run the following command.

```bash
python3 eval_with_pruned_model.py --weight runs/detect/train/weights/best.pt \ # for pruned model weights path
                                --dataset coco.yaml \ # for dataset config 
                                --imgsz 640 \ # for image size
                                --is_pruned_weight # to load a pruned model's weights, specify this option.
```

The following log can be confirmed. It shows the accuracy of the trained model.

```bash
...
Class     Images  Instances      Box(P          R      mAP50  mAP50-95): 100%|█████
  all       5000      36335      0.729      0.635      0.695      0.529
...
```

# Tips
### Reference: Accuracy results of pruning then retraining
When pruning then retraining was performed in our environment, the accuracy results were shown as follows.

||mAPval50-95|
|:---:|:---:|
|Baseline|53.9|
|Pruning rate 70%|52.9|

> Note:  
> This information is just for reference purposes because the above results are not guaranteed.

[Compiling model for DRP-AI TVM](./../../../tutorials/tutorial_RZV2H.md)[^1] [(RZ/V2H) ->](./../../../tutorials/tutorial_RZV2H.md)

[^1]: DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.
