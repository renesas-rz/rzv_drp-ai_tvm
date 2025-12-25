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
# Copyright (c) OpenMMLab. All rights reserved.
from .checkpoint import CheckpointHook
from .closure import ClosureHook
from .ema import EMAHook
from .evaluation import DistEvalHook, EvalHook
from .hook import HOOKS, Hook
from .iter_timer import IterTimerHook
from .logger import (DvcliveLoggerHook, LoggerHook, MlflowLoggerHook,
                     NeptuneLoggerHook, PaviLoggerHook, SegmindLoggerHook,
                     TensorboardLoggerHook, TextLoggerHook, WandbLoggerHook)
from .lr_updater import (CosineAnnealingLrUpdaterHook,
                         CosineRestartLrUpdaterHook, CyclicLrUpdaterHook,
                         ExpLrUpdaterHook, FixedLrUpdaterHook,
                         FlatCosineAnnealingLrUpdaterHook, InvLrUpdaterHook,
                         LrUpdaterHook, OneCycleLrUpdaterHook,
                         PolyLrUpdaterHook, StepLrUpdaterHook)
from .memory import EmptyCacheHook
from .momentum_updater import (CosineAnnealingMomentumUpdaterHook,
                               CyclicMomentumUpdaterHook, MomentumUpdaterHook,
                               OneCycleMomentumUpdaterHook,
                               StepMomentumUpdaterHook)
from .optimizer import (Fp16OptimizerHook, GradientCumulativeFp16OptimizerHook,
                        GradientCumulativeOptimizerHook, OptimizerHook)
from .profiler import ProfilerHook
from .sampler_seed import DistSamplerSeedHook
from .sync_buffer import SyncBuffersHook
from .pruning_hook import PruningHook
__all__ = [
    'HOOKS', 'Hook', 'CheckpointHook', 'ClosureHook', 'LrUpdaterHook',
    'FixedLrUpdaterHook', 'StepLrUpdaterHook', 'ExpLrUpdaterHook',
    'PolyLrUpdaterHook', 'InvLrUpdaterHook', 'CosineAnnealingLrUpdaterHook',
    'FlatCosineAnnealingLrUpdaterHook', 'CosineRestartLrUpdaterHook',
    'CyclicLrUpdaterHook', 'OneCycleLrUpdaterHook', 'OptimizerHook',
    'Fp16OptimizerHook', 'IterTimerHook', 'DistSamplerSeedHook',
    'EmptyCacheHook', 'LoggerHook', 'MlflowLoggerHook', 'PaviLoggerHook',
    'TextLoggerHook', 'TensorboardLoggerHook', 'NeptuneLoggerHook',
    'WandbLoggerHook', 'DvcliveLoggerHook', 'MomentumUpdaterHook',
    'StepMomentumUpdaterHook', 'CosineAnnealingMomentumUpdaterHook',
    'CyclicMomentumUpdaterHook', 'OneCycleMomentumUpdaterHook',
    'SyncBuffersHook', 'EMAHook', 'EvalHook', 'DistEvalHook', 'ProfilerHook',
    'GradientCumulativeOptimizerHook', 'GradientCumulativeFp16OptimizerHook',
    'SegmindLoggerHook', 'PruningHook'
]
