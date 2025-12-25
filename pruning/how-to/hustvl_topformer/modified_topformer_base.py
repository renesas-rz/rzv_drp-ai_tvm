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
# model settings
norm_cfg = dict(type='SyncBN', requires_grad=True)

model_cfgs = dict(
    cfg=[
        # k,  t,  c, s
        [3,   1,  16, 1], # 1/2        0.464K  17.461M
        [3,   4,  32, 2], # 1/4 1      3.44K   64.878M
        [3,   3,  32, 1], #            4.44K   41.772M
        [5,   3,  64, 2], # 1/8 3      6.776K  29.146M
        [5,   3,  64, 1], #            13.16K  30.952M
        [3,   3,  128, 2], # 1/16 5     16.12K  18.369M
        [3,   3,  128, 1], #            41.68K  24.508M
        [5,   6,  160, 2], # 1/32 7     0.129M  36.385M
        [5,   6,  160, 1], #            0.335M  49.298M
        [3,   6,  160, 1], #            0.335M  49.298M
    ],
    channels=[32, 64, 128, 160],
    out_channels=[None, 256, 256, 256],
    embed_out_indice=[2, 4, 6, 9],
    decode_out_indices=[1, 2, 3],
    num_heads=8,
    c2t_stride=2,
)

model = dict(
    type='EncoderDecoder',
    backbone=dict(
        type='Topformer',
        cfgs=model_cfgs['cfg'], 
        channels=model_cfgs['channels'],
        out_channels=model_cfgs['out_channels'], 
        embed_out_indice=model_cfgs['embed_out_indice'],
        decode_out_indices=model_cfgs['decode_out_indices'],
        depths=4,
        num_heads=model_cfgs['num_heads'],
        c2t_stride=model_cfgs['c2t_stride'],
        drop_path_rate=0.1,
        norm_cfg=norm_cfg,
        # init_cfg=dict(
        #     type='Pretrained', checkpoint='modelzoos/classification/topformer-B-224-75.3.pth')
    ),
    decode_head=dict(
        type='SimpleHead',
        in_channels=[256, 256, 256],
        in_index=[0, 1, 2],
        channels=256,
        dropout_ratio=0.1,
        num_classes=150,
        norm_cfg=norm_cfg,
        align_corners=False,
        loss_decode=dict(
            type='CrossEntropyLoss', use_sigmoid=False, loss_weight=1.0)),
    # model training and testing settings
    train_cfg=dict(),
    test_cfg=dict(mode='whole'),
    init_cfg=dict(
       type='Pretrained', checkpoint='TopFormer-B_512x512_2x8_160k-38.3.pth')
    )