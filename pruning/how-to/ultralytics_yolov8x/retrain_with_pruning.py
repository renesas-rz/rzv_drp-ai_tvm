#######################################################################################################################
# DISCLAIMER
# This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No
# other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
# applicable laws, including copyright laws.
# THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
# THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM
# EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES
# SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO
# THIS SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
# Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of
# this software. By using this software, you agree to the additional terms and conditions found by accessing the
# following link:
# http://www.renesas.com/disclaimer
#
# Copyright (C) 2025 Renesas Electronics Corporation. All rights reserved.
#######################################################################################################################
from ultralytics.models.yolo.detect import DetectionTrainer
# Import DRP-AI Extension Pack
from drpai_compaction_tool.pytorch import make_pruning_layer_list, \
                                                            Pruner, \
                                                            get_model_info
import torch
from ultralytics import YOLO
import argparse
                                              
class CustomDetectionTrainer(DetectionTrainer):
    def __init__(self, pruning_rate, is_pruned_weight, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.pruning_rate = pruning_rate
        self.is_pruned_weight = is_pruned_weight
    
    def get_model(self, weights=None, cfg=None, verbose=True):
        
        # To resume training from saved pruned weights
        if self.is_pruned_weight:
            model = YOLO(self.args.model)
            model = model.model
            input_data = torch.rand(1,3,self.args.imgsz,self.args.imgsz)
            pruning_layer_list = make_pruning_layer_list(model, input_data=[input_data])
            self.pruner = Pruner(model, pruning_layer_list,  final_pr=0.0)
            
            # Confirming the result of loading weights correctly
            print(get_model_info(model, input_data=[input_data]))
        else:
            model = super().get_model(weights=weights, cfg=cfg, verbose=verbose)
        
        for p in model.parameters():
            p.requires_grad = True
        input_data = torch.rand(1,3,self.args.imgsz,self.args.imgsz)
        pruning_layer_list = make_pruning_layer_list(model, input_data=[input_data])
        self.pruner = Pruner(model, pruning_layer_list,  final_pr=self.pruning_rate)
        
        # Confirming the result of pruned weights correctly
        print(get_model_info(model, input_data=[input_data]))
        
        return model
    
def on_train_batch_start(trainer):
    trainer.pruner.update()


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--weight", default= "yolov8x.pt", help=".pt file of the model")
    parser.add_argument('--resume', action='store_true',
                    help='Resume from previous weights')
    parser.add_argument('--dataset', default = "coco.yaml",
                    help='Using dataset to eval, please specify this option')
    parser.add_argument('--imgsz', type=int, default=640, help='Image size')
    parser.add_argument('--epochs', type=int, default=100, help='Training epochs')
    parser.add_argument('--device', default="0", type=str,
                    help='Device to use (e.g. "0", "cpu")')
    parser.add_argument('--batch', default=16, type=int,
                    help='Batch size for training')
    parser.add_argument("--pruning_rate", default=0.7, type=float, help="pruning rate. this parameter can be in the range [0,1).")
    parser.add_argument('--is_pruned_weight', action='store_true',
                    help='When testing the model with pruned weight, please specify this option')
    args = parser.parse_args()
    
    trainer = CustomDetectionTrainer(
        pruning_rate=args.pruning_rate,
        is_pruned_weight=args.is_pruned_weight,
        overrides={
        'model': args.weight,
        'data':  args.dataset,
        'imgsz': args.imgsz,
        'epochs': args.epochs,
        'resume': args.resume,
        'device': args.device, 
        'batch': args.batch
        }
    )   

    trainer.add_callback("on_train_batch_start", on_train_batch_start)
    trainer.train()