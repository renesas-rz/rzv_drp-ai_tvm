#
#  Original code (C) Copyright Renesas Electronics Corporation 2023
#
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.

class FORMAT():
    """
    Constant values for data format.
    """
    UNKNOWN     = 0xFFFF
    RGB         = 0x2000
    BGR         = 0x2001
    GRAY        = 0x2010
    YUYV_422    = 0x0000
    YVYU_422    = 0x0001
    UYUV_422    = 0x0002
    VUYY_422    = 0x0003
    # only YUV2RGB is allowed (not X2GRAY).
    YUYV_420    = 0x1000
    UYVY_420    = 0x1001
    YV12_420    = 0x1002
    IYUV_420    = 0x1003
    NV12_420    = 0x1004
    NV21_420    = 0x1005
    IMC1_420    = 0x1006
    IMC2_420    = 0x1007
    IMC3_420    = 0x1008
    IMC4_420    = 0x1009

    NOT_YUY2    = 0x2000
    YUV420      = 0x1000
    N_CH_3      = 3
    N_CH_2      = 2
    N_CH_1      = 1

    RGB_STR     = "RGB"
    BGR_STR     = "BGR"
    GRAY_STR    = "GRAY"
    YUY2_STR    = "YUY2"
    STR = {
        RGB: RGB_STR,
        BGR: BGR_STR,
        GRAY: GRAY_STR,
        YUYV_422: YUY2_STR,
        YVYU_422: YUY2_STR,
        UYUV_422: YUY2_STR,
        VUYY_422: YUY2_STR,
        YUYV_420: YUY2_STR,
        UYVY_420: YUY2_STR,
        YV12_420: YUY2_STR,
        IYUV_420: YUY2_STR,
        NV12_420: YUY2_STR,
        NV21_420: YUY2_STR,
        IMC1_420: YUY2_STR,
        IMC2_420: YUY2_STR,
        IMC3_420: YUY2_STR,
        IMC4_420: YUY2_STR
    }


    

class ORDER():
    """
    Constant values for data order.
    """
    UNKNOWN = 0xFFFF
    HWC     = 0x0000
    CHW     = 0x0001
    C       = 0x0010
    HWC_STR = "HWC"
    CHW_STR = "CHW"
    C_STR   = "C"
    STR = {
        HWC: HWC_STR,
        CHW: CHW_STR,
        C: C_STR
    }

class TYPE():
    """
    Constant values for data type.
    """
    UNKNOWN = 0xFFFF
    UINT8   = 0x0000
    FP16    = 0x0001
    FP32    = 0x0002
    UINT8_STR   = "uint8"
    FP16_STR    = "fp16"
    FP32_STR    = "fp32"
    STR = {
        UINT8: UINT8_STR,
        FP16: FP16_STR,
        FP32: FP32_STR
    }

class MODE():
    """
    Constant values for distinguishing pre-processing mode.
    """
    PRE     = 0x0000
    POST    = 0x0001
