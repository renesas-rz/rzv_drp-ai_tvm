[<- Installing DRP-AI Extention Package](./../../setup/README.md)

# How to use the DRP-AI Extension Pack

This document explains the contents of pruning then retraining by using the DRP-AI Extension Pack with Vision Transformer (hereinafter referred to as ViT) provided by [huggingface framework](https://github.com/huggingface/pytorch-image-models/tree/v1.0.9).

Please read [README.md](./../../setup/README.md) in advance and make docker image.

This tutorial will perform 5 steps like below.

> **Note:**  
> This chapter takes a long time to retrain the AI model.  
> If you only want to validate RZ/V2H operation, please skip this chapter.

<img src=./../../../img/flow-of-pruning-then-retraining.png width=600 alt="Flow of the DRP-AI Extension Pack">


### Step1. Run docker image
Please change to the directory where this README is located and then execute the following command.

```bash
cd ${TVM_ROOT}/pruning/how-to/huggingface_vit/
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
# pytorch-image-models/
# patch/
# setup.sh
# README.md
```

Move to the working directory with the following command.

```bash
cd pytorch-image-models
ls
# Confirm the following files.
# retrain_with_pruning.py
# distributed_retrain_with_pruning.sh
# torch2onnx.py
# ...
```


### Step3. Prepare the datasets
The huggingface vision transformer was pretrained on ImageNet-21k, which has an output layer of size 21,841.
But this tutorial is supposed to use the Imagenet-1k like dataset because of GPU limitations.
Therefore, prepare a dataset with 1000 classes for this tutorial.
For example, the whole dataset is as below.

```bash
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
    |- validation
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
./distributed_retrain_with_pruning.sh <Number of GPUs to be used in training e.g. 2> \
                                      <path/to/dataset> \
                                      --model vit_base_patch16_224.orig_in21k  \
                                      --num-classes 1000 \  # Since a dataset with 1000 classes is used, 1000 is specified here.
                                      --epochs 8  \   # For the sake of time, this tutorial uses small values.
                                      --lr-base 0.025  \
                                      -b 128  \
                                      --warmup-epochs 1 \
                                      --pruning_rate 0.7 # For pruning a model with a pruning rate 70%
```

After retraining is finished, the following files can be confirmed in the `output/train/YYYYMMDD-hhmmss-vit_base_patch16_224_orig_in21k-224` directory.
```bash
checkpoint-0.pth.tar
checkpoint-1.pth.tar
...
checkpoint-7.pth.tar
model_best.pth.tar
```


### Step5. Test the pruned model
In this chapter, the accuracy of the pruned model is measured to ensure that its accuracy is maintained after pruning then retraining.

Convert pytorch format model to onnx format model first.
```bash
python3 torch2onnx.py pruned_model.onnx \
                      --model vit_base_patch16_224.orig_in21k \
                      --num-classes 1000 \  # Since a dataset with 1000 classes is used, 1000 is specified here.
                      --checkpoint <path/to>model_best.pth.tar \
                      --img-size 224 \
                      --is_pruned_weight # To load a pruned model's weights, specify this option.
```

Test the pruned model with onnx format.
```bash
python3 onnx_validate.py <path/to/dataset> \
                         --onnx-input pruned_model.onnx
```

The following log can be confirmed. It shows the accuracy of the trained model.
```bash
...
Test: [20/196]  Time 8.857 (8.876, 28.843/s, 3.467 ms/sample)   Prec@1 75.781 (76.488)  Prec@5 91.016 (92.634)
...
```

(Option) If test the pruned model with pytorch format, please run the following command.
```bash
python3 eval_with_pruned_model.py <path/to/dataset> \
                                  --model vit_base_patch16_224.orig_in21k  \
                                  --num-classes 1000 \  # Since a dataset with 1000 classes is used, 1000 is specified here.
                                  --checkpoint <path/to>model_best.pth.tar \
                                  --is_pruned_weight # To load a pruned model's weights, specify this option.
```


# Tips
### How to find difference between code for initial training and code for pruning then retraining
Please execute the following command to find difference between code for initial training and code for pruning then retraining. 

You can find how to add the DRP-AI Extension Pack APIs to code for initial training.

```bash
diff -u train.py retrain_with_pruning.py
```

[Compiling model for DRP-AI TVM](./../../../tutorials/tutorial_RZV2H.md)[^1] [(RZ/V2H) ->](./../../../tutorials/tutorial_RZV2H.md)

[^1]: DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.
