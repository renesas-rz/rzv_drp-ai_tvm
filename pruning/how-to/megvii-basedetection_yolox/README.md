# How to use the AI model with the DRP-AI Extension Pack
This document explains the contents of pruning then retraining by using the DRP-AI Extension Pack with YOLOX-s provided by [Megvii-BaseDetection framework](https://github.com/Megvii-BaseDetection/YOLOX/tree/0.3.0).

Please read [README.md](./../../installing/README.md) in advance and make docker image.


This tutorial will perform 5 steps like below.

>**Note:**  
>This chapter takes <u>**a long time**</u> to retrain the AI model.  
>In our environment (Tesla V100 was used), it takes about <u>**4days**</u>.  
>If only want to validate RZ/V2H operation, please skip this chapter.

<img src=./../../../img/flow-of-pruning-then-retraining.png width=600 alt="Flow of the DRP-AI Extension Pack">


### Step1. Run docker image
Please change to the directory where this README is located and then execute the following command.

```bash
cd ${TVM_ROOT}/pruning/how-to/megvii-basedetection_yolox
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
# YOLOX/
# patch/
# setup.sh
# README.md
```

Move to the working directory with the following command.

```bash
cd YOLOX
ls
# Confirm the following folders and files.
# torch2onnx.py
# yolox_voc_s_pruning_conf.py
# onnx_voc_evaluator.py
# eval_with_pruned_model.py
# yolox/
```


### Step3. Prepare the datasets and pre-trained model
In this tutorial use the pascal VOC dataset. 
To find more details please visit [http://host.robots.ox.ac.uk/pascal/VOC/](http://host.robots.ox.ac.uk/pascal/VOC/).  
Please download the dataset present at the following links.
- http://host.robots.ox.ac.uk/pascal/VOC/voc2007/VOCtrainval_06-Nov-2007.tar
- http://host.robots.ox.ac.uk/pascal/VOC/voc2007/VOCtest_06-Nov-2007.tar 
- http://host.robots.ox.ac.uk/pascal/VOC/voc2012/VOCtrainval_11-May-2012.tar

```bash
mkdir -p datasets
tar xvf VOCtrainval_06-Nov-2007.tar -C datasets
tar xvf VOCtest_06-Nov-2007.tar -C datasets
tar xvf VOCtrainval_11-May-2012.tar -C datasets
ls datasets/VOCdevkit
# Confirm the following folders
# VOC2012/
# VOC2007/
```

To create the trained model on VOC, this chapter uses the COCO pretrained-model provided by Megvii-BaseDetection for initializing the model.
Training can be performed by executing the following command.

```bash
wget https://github.com/Megvii-BaseDetection/YOLOX/releases/download/0.1.1rc0/yolox_s.pth
python3 -m yolox.tools.train -f exps/example/yolox_voc/yolox_voc_s.py \
                             -c yolox_s.pth \
                             -d 1 \
                             -b 64 \
                             --fp16 \
                             -o
```
>**Note:**<br>
>- -f: experiment description file
>- -c: checkpoint file
>- -d: number of gpu devices
>- -b: total batch size
>- --fp16: mixed precision training
>- -o: occupy GPU memory first for training.

After training is finished, the trained weight is contained in the `YOLOX_outputs/yolox_voc_s` folder.<br>
And the trained weight is named `best_ckpt.pth`.
```bash
ls YOLOX_outputs/yolox_voc_s
# Confirm the following folders and files
# ...
# best_ckpt.pth
# train_log.txt
# tensorboard/
```

### Step4. Pruning then retraining
Pruning then retraining can be performed by executing the following command.<br>
This chapter uses the trained model on VOC which is created above for initializing the model.
```bash
PRUNING_RATE=0.7 python3 -m yolox.tools.train -f yolox_voc_s_pruning_conf.py \
                                              -c YOLOX_outputs/yolox_voc_s/best_ckpt.pth \
                                              -d 1 \
                                              -b 32 \
                                              --fp16 \
                                              -o
```
>**Note1:**<br>
>The pruning rate is set as an environment variable (PRUNING_RATE) in this tutorial.<br>
>When performing the pruning then retraining, please set `PRUNING_RATE` environment variable.

>**Note2:**<br>
> We recommend using the same parameters for pruning then retraining, such as epoch and batch size, as those that were set for initial training.<br>
> However, in this tutorial the batch size is changed from 64 to 32 because of our GPU memory limitations. <br>
> If you have a high performance GPU, please set the batch size to 64.

After retraining is finished, the `YOLOX_outputs/yolox_voc_s_pruning_conf` folder can be confirmed. <br>
And the trained weight is named `best_ckpt.pth`.

```bash
ls YOLOX_outputs/yolox_voc_s_pruning_conf
# Confirm the following folders and files
# ...
# best_ckpt.pth
# train_log.txt
# tensorboard/
```


### Step5. Test the pruned model
In this chapter, the accuracy of the pruned model is measured to ensure that its accuracy is maintained after pruning then retraining.

Convert pytorch format model to onnx format model first.
```bash
python3 torch2onnx.py -f yolox_voc_s_pruning_conf.py \
                      -c YOLOX_outputs/yolox_voc_s_pruning_conf/best_ckpt.pth \
                      --output-name pruned_model.onnx \
                      --is_pruned_weight
```

Test the pruned model with onnx format.
```bash
python3 eval_with_pruned_model.py -f yolox_voc_s_pruning_conf.py \
                                  -c pruned_model.onnx \
                                  -b 1 \
                                  -d 1 \
                                  --conf 0.001
```

The following log can be confirmed. It shows the accuracy of the trained model.
```
...
map_5095: 0.6120537790481723
map_50: 0.8182438030405812
...
```

(Option) If test the pruned model with pytorch format, please run the following command.
```bash
python3 eval_with_pruned_model.py -f yolox_voc_s_pruning_conf.py \
                                  -c YOLOX_outputs/yolox_voc_s_pruning_conf/best_ckpt.pth \
                                  -b 64 \
                                  -d 1 \
                                  --conf 0.001 \
                                  --is_pruned_weight \
                                  --use_pytorch_model
```


# FAQ
### IndexError: Caught IndexError in DataLoader worker process 0. or RuntimeError: Pin memory thread exited unexpectedly
If the above error occured during the training or retraining, please retry the training or retraining command.


### RuntimeError: cuDNN error: CUDNN_STATUS_INTERNAL_ERROR
If the above error occured during the training or retraining, please remove `-o` option from the training or retraining command.


### UserWarning: CUDA initialization: Unexpected error from cudaGetDeviceCount().
Please update nvidia driver on host pc. 


# Tips
### How to find difference between code for initial training and code for pruning then retraining
Please execute the following command to find difference between code for initial training and code for pruning then retraining. 

You can find how to add the DRP-AI Extension Pack APIs to code for initial training.

```bash
diff -u yolox/core/trainer.py yolox_voc_s_pruning_conf.py
```


### Reference: Accuracy results of pruning then retraining
When pruning then retraining was performed in our environment, the accuracy results were shown as follows.

||mAP@0.5|
|:---:|:---:|
|Baseline|82.50%|
|Pruning rate 70%|81.80%|

> Note:  
> This information is just for reference purposes because the above results are not guaranteed.


# Usage Notes
- Do not use Exponential Moving Average (EMA) with gradual pruning. It may cause incorrect pruning result.  
Note: For YOLOX, the default training setting uses EMA.