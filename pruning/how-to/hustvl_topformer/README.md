[<- Installing DRP-AI Extention Package](./../../setup/README.md)

# How to use the DRP-AI Extension Pack

This document explains the contents of pruning then retraining by using the DRP-AI Extension Pack with TopFormer (Token Pyramid Transformer for Mobile Semantic Segmentation) provided by [hustvl](https://github.com/hustvl/TopFormer/tree/cc0f4369698bd813a0d8eefcdcf2d28d91080e21).

Please read [README.md](./../../setup/README.md) in advance and make docker image.

This tutorial will perform 5 steps like below.

> **Note:**  
> This chapter takes a long time to retrain the AI model.  
> If you only want to validate RZ/V2H operation, please skip this chapter.

<img src=./../../../img/flow-of-pruning-then-retraining.png width=600 alt="Flow of the DRP-AI Extension Pack">


### Step1. Run docker image
Please change to the directory where this README is located and then execute the following command.

```bash
cd $WORK/pruning-tool-examples/examples/hustvl_topformer
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
# TopFormer/
# patch/
# scripts/
# setup.sh
# README.md
# __init__.py
# pruning_hook.py
# topformer_base_512x512_160k_2x8_ade20k_pruning_conf.py
# torch2onnx.py
# onnx_validate.py
# eval_with_pruned_model.py
```

Move to the working directory with the following command.

```bash
cd TopFormer
ls tools/
# Confirm the following files.
# eval_with_pruned_model.py
# onnx_validate.py
# torch2onnx.py
# ...
```

### Step3. Prepare the datasets
ADE20K dataset: The detail datasets at https://github.com/open-mmlab/mmsegmentation/blob/master/docs/en/dataset_prepare.md
The training and validation set of ADE20K could be download from this [link](http://data.csail.mit.edu/places/ADEchallenge/ADEChallengeData2016.zip) to workspace TopFormer.
We may also download test set from [here](http://data.csail.mit.edu/places/ADEchallenge/release_test.zip).
The ADE20K dataset is split into two subsets:
```bash
├── data
│   └── ade
│       ├── ADEChallengeData2016
│       └── release_test
```

### Step3 & 4. Pruning then retraining
Pruning then retraining can be performed by executing the following command.
Downdload Topformer model. The example for TopFormer-B_512x512_2x8_160k downdload at [link ](https://drive.google.com/file/d/1pYUUB4N6FVjXt1NKygtQis5WmhGECrNt/view?usp=sharing)
```bash
PRUNING_RATE=0.7 python3 tools/train.py local_configs/topformer/topformer_base_512x512_160k_2x8_ade20k_pruning_conf.py \ # for config model
--gpus 1 \ # for gpu numbers
--options load_from=TopFormer-B_512x512_2x8_160k-38.3.pth 
```

After retraining is finished, the following files can be confirmed in the `work_dirs/topformer_base_512x512_160k_2x8_ade20k_pruning_conf` directory.
```bash
latest.pth
topformer_base_512x512_160k_2x8_ade20k_pruning_conf.py
...
```

### Step5. Test the pruned model
In this chapter, the accuracy of the pruned model is measured to ensure that its accuracy is maintained after pruning then retraining.

Convert pytorch format model to onnx format model first.
```bash
python3 ./tools/torch2onnx.py \
local_configs/topformer/topformer_base_512x512_160k_2x8_ade20k_pruning_conf.py \ # for config model
--shape 512 512 \
--is_pruned_weight \
--checkpoint <path/to>latest.pth \ # for model weights path
--output-file pruned_model.onnx
```

Test the pruned model with onnx format.
```bash
python tools/onnx_validate.py  \
local_configs/topformer/topformer_base_512x512_160k_2x8_ade20k_pruning_conf.py \ # for config model
pruned_model.onnx \ # for output model name
--backend=onnxruntime \
--shape 512 512 \
--eval mIoU
```

(Option) If test the pruned model with pytorch format, please run the following command.
```bash
python3 tools/eval_with_pruned_model.py local_configs/topformer/topformer_base_512x512_160k_2x8_ade20k_pruning_conf.py \ # for config model
<path/to>latest.pth \ # for model weights path
--eval mIoU \
--is_pruned_weight
```

The following log can be confirmed. It shows the accuracy of the trained model.
```bash
...
+-------+-------+-------+
|  aAcc |  mIoU |  mAcc |
+-------+-------+-------+
| 77.07 | 37.05 | 48.21 |
+-------+-------+-------+
...
```

# Tips
### Reference: Accuracy results of pruning then retraining
When pruning then retraining was performed in our environment, the accuracy results were shown as follows.

||mIoU|
|:---:|:---:|
|Baseline|38.3|
|Pruning rate 70%|37.05|

> Note:  
> This information is just for reference purposes because the above results are not guaranteed.

[Compiling model for DRP-AI TVM](./../../../tutorials/tutorial_RZV2H.md)[^1] [(RZ/V2H) ->](./../../../tutorials/tutorial_RZV2H.md)

[^1]: DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.