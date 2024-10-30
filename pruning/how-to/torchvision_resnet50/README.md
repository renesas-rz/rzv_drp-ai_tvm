[<- Installing DRP-AI Extention Package](./../../setup/README.md)
# How to use the DRP-AI Extension Pack
This document explains the contents of pruning then retraining by using the DRP-AI Extension Pack with ResNet50 provided by [torchvision framework](https://pytorch.org/vision/stable/index.html).

Please read [README.md](./../../setup/README.md) in advance and make docker image.

This tutorial will perform 5 steps like below.

> **Note:**  
> This chapter takes a long time to retrain the AI model.  
> If you only want to validate RZ/V2H operation, please skip this chapter.

<img src=./../../../img/flow-of-pruning-then-retraining.png width=600 alt="Flow of the DRP-AI Extension Pack">


### Step1. Run docker image
Please change to the directory where this README is located and then execute the following command.

```bash
cd ${TVM_ROOT}/pruning/how-to/torchvision_resnet50/
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
# vision/
# patch/
# setup.sh
# README.md
```

Move to the working directory with the following command.

```bash
cd vision/references/classification
ls
# Confirm the following files.
# retrain_with_pruning.py
# onnx_inference.py
# torch2onnx.py
# ...
```


### Step3. Prepare the datasets
The torchvision resnet50 was pretrained on Imagenet, which has an output layer of size 1000.
So this tutorial is supposed to use the Imagenet like dataset.  
For example, the whole dataset is as below.

```
classification/
|-data/
    |- train
    |    |- cat
    |    |    |- xxx.jpg
    |    |    |- xyz.jpg
    |    |    |- ....
    |    |- horse
    |    |    |- horse1.jpg
    |    |    |- horse2.jpg
    |    |    |- ....
    |    |- dog
    |         |- 123.jpg
    |         |- 456.jpg
    |         |- ....
    |- val
    |    |- cat
    |    |    |- 1xxx.jpg
    |    |    |- 2xyz.jpg
    |    |    |- ....
    |    |- horse
    |    |    |- 1horse1.jpg
    |    |    |- 2horse2.jpg
    |    |    |- ....
    |------ dog
            |- 1123.jpg
            |- 2456.jpg
            |- ....
```

In this case, you don't need provide annotation file. 
All images in the directory of `cat` will be recognized as `cat` class.

### Step3 & 4. Pruning then retraining
Pruning then retraining can be performed by executing the following command.
Running the following command will automatically download the pre-trained model.
```bash
python3 retrain_with_pruning.py --data-path data \
                                --weights ResNet50_Weights.DEFAULT \
                                --pruning_rate 0.7 \
                                -b 256
```

After retraining is finished, the following files can be confirmed in the current directory.
```
model_0.pth
model_1.pth
...
model_89.pth
```


### Step5. Test the pruned model
In this chapter, the accuracy of the pruned model is measured to ensure that its accuracy is maintained after pruning then retraining.

Convert pytorch format model to onnx format model first.
```bash
python3 torch2onnx.py --weight <path/to>model_89.pth \
                      --is_pruned_weight \
                      --output_onnx_filepath pruned_model.onnx
```

Test the pruned model with onnx format.
```bash
python3 onnx_inference.py --input_model pruned_model.onnx \
                          --data_path <path/to/data>test_dataset
```

The following log can be confirmed. It shows the accuracy of the trained model.
```
...
Test[50000/50000] ValidAcc=72.670%    Test Finish
```

(Option) If test the pruned model with pytorch format, please run the following command.

```bash
python3 retrain_with_pruning.py --data-path data \
                                --resume <path/to>model_89.pth \
                                --is_pruned_weight \
                                --test-only
```

# FAQ

### How to retrain with multi gpus

The script does not support retraining with multi gpus.

# Tips

### How to find difference between code for initial training and code for pruning then retraining

Please execute the following command to find difference between code for initial training and code for pruning then retraining. 

You can find how to add the DRP-AI Extension Pack APIs to code for initial training.

```bash
diff -u train.py retrain_with_pruning.py
```

[Compiling model for DRP-AI TVM](./../../../tutorials/tutorial_RZV2H.md)[^1] [(RZ/V2H) ->](./../../../tutorials/tutorial_RZV2H.md)

[^1]: DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.
