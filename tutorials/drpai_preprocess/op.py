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

import os
import sys
from . import drpai_param
TRANSLATOR= os.getenv("TRANSLATOR")
if(TRANSLATOR == None):
    print("[ERROR] No environment variable")
    print("        Before running this script,")
    print("        Please set environment variable(TRANSLATOR)")
    print("        to the directory where you installed DRP-AI Translator")
    print("        e.g. $export TRANSLATOR=/home/user/drp-ai_translator_release/")
    sys.exit(-1)
DRP_TOOLS = TRANSLATOR + "/DRP-AI_translator"
sys.path.append(DRP_TOOLS)
from python_api import *

def in_list(param, list, param_name, param_str):
    """
    Method to check the parameter is in the list.
    Print error log if false
    Args:
        param : User input parameter
        list (list) : list to check the parameter is included
        log (string): string to display on the error log.
    Returns:
        True if param is in the list. 
        False if not.
    """
    if (param not in list):
        print("[ERROR]",param_name, "not supported:", param, "=", param_str)
        return False
    return True

def check_param_supported(op_io, f_in, f_out, o_in, o_out, t_in, t_out, mode=drpai_param.MODE.PRE):
    """
    Method to check the parameter is supported by the operator.
    Print error log if false
    Args:
        op_io (Op_IO) : Operator In/Out parameter
        f_in (list) : format_in supported list
        f_out (list) : format_out supported list
        o_in (list) : order_in supported list
        o_out (list) : order_out supported list
        t_in (list) : type_in supported list
        t_out (list) : type_out supported list
        mode (int): pre or post mode.
    Returns:
        True if param is supported by the operator. 
        False if not.
    """
    if (not mode):
        # Check format is supported
        if (not in_list(op_io.format_in, f_in, "format_in", drpai_param.FORMAT.STR[op_io.format_in])):
            return False
        if (not in_list(op_io.format_out, f_out, "format_out", drpai_param.FORMAT.STR[op_io.format_out])):
            return False
    # Check order is supported
    if (not in_list(op_io.order_in, o_in, "order_in", drpai_param.ORDER.STR[op_io.order_in])):
        return False
    if (not in_list(op_io.order_out, o_out, "order_out", drpai_param.ORDER.STR[op_io.order_out])):
        return False
    # Check type is supported
    if (not in_list(op_io.type_in, t_in, "type_in", drpai_param.TYPE.STR[op_io.type_in])):
        return False
    if (not in_list(op_io.type_out, t_out, "type_out", drpai_param.TYPE.STR[op_io.type_out])):
        return False
    return True

class Op_IO():
    def __init__(self):
        self.shape_in_w = 0;
        self.shape_in_h = 0;
        self.shape_in_c = 0;
        self.format_in = 0;
        self.order_in = 0;
        self.type_in = 0;
        self.shape_out_w = 0;
        self.shape_out_h = 0;
        self.shape_out_c = 0;
        self.format_out = 0;
        self.order_out = 0;
        self.type_out = 0;
    def copy(self, src):
        self.shape_in_w = src.shape_in_w;
        self.shape_in_h = src.shape_in_h;
        self.shape_in_c = src.shape_in_c;
        self.format_in = src.format_in;
        self.order_in = src.order_in;
        self.type_in = src.type_in;
        self.shape_out_w = src.shape_out_w;
        self.shape_out_h = src.shape_out_h;
        self.shape_out_c = src.shape_out_c;
        self.format_out = src.format_out;
        self.order_out = src.order_out;
        self.type_out = src.type_out;
    def refresh(self):
        self.shape_out_w = self.shape_in_w;
        self.shape_out_h = self.shape_in_h;
        self.shape_out_c = self.shape_in_c;
        self.format_out = self.format_in;
        self.order_out = self.order_in;
        self.type_out = self.type_in;

    def cp_prev(self, prev):
        self.shape_in_w = prev.shape_out_w;
        self.shape_in_h = prev.shape_out_h;
        self.shape_in_c = prev.shape_out_c;
        self.format_in = prev.format_out;
        self.order_in = prev.order_out;
        self.type_in = prev.type_out;
        self.refresh()

    def print(self, mode=drpai_param.MODE.PRE):
        """
        For debug.
        Method to print Op_IO class
        Args:
            mode (int): Preruntime mode.
        Returns:
        """
        print("  Shape_in (HWC):", self.shape_in_h, self.shape_in_w, self.shape_in_c)
        print("  Shape_out(HWC):", self.shape_out_h, self.shape_out_w, self.shape_out_c)
        if (not mode):
            print("  FORMAT:", drpai_param.FORMAT.STR[self.format_in], drpai_param.FORMAT.STR[self.format_out])
        print("  ORDER :", drpai_param.ORDER.STR[self.order_in], drpai_param.ORDER.STR[self.order_out])
        print("  TYPE  :", drpai_param.TYPE.STR[self.type_in], drpai_param.TYPE.STR[self.type_out])


class Op():
    """
    Base class for operators
    """
    def __init__(self):
        """
        Initialize necessary variables.
        If external class, take input from user.
        Error check is not included.
        """
        self.shape_in_w = 0;
        self.shape_in_h = 0;
        self.shape_in_c = 0;
        self.format_in = 0;
        self.order_in = 0;
        self.type_in = 0;
        self.shape_out_w = 0;
        self.shape_out_h = 0;
        self.shape_out_c = 0;
        self.format_out = 0;
        self.order_out = 0;
        self.type_out = 0;
    def is_supported(self, op_io):
        """
        Check the user input and input/output information are supported by the operator.
        Returns: True if parameters are valid and supported. False if not.
        """
        pass
    def print_op(self):
        pass
    def get_api(self):
        """
        Convert internal operator into DRP-AI Translator Python API.
        Returns: A set of DRP-AI Translator Python API.
        """
        return []

class Crop(Op):
    """
    External class that user may call for following operator.
    DRP-AI Translator operator: crop
    """
    name = "Crop"
    # Supported parameter list
    format_in_supported = {
        drpai_param.FORMAT.RGB,
        drpai_param.FORMAT.BGR,
        drpai_param.FORMAT.GRAY
    }
    format_out_supported = format_in_supported
    order_in_supported = drpai_param.ORDER.STR.keys()
    order_out_supported = {
        drpai_param.ORDER.HWC
    }
    type_in_supported = {
        drpai_param.TYPE.UINT8,
        drpai_param.TYPE.FP16,
    }
    type_out_supported = type_in_supported

    def __init__(self, crop_tl_x=0, crop_tl_y=0, width=0, height=0):
        self.tl_x = crop_tl_x
        self.tl_y = crop_tl_y
        self.width = width
        self.height = height
        self.data_type = 0
        # Following variable is fixed, since Preruntime only supports HWC input.
        self.data_format = drpai_param.ORDER.HWC 
        self.op_io = None

    def is_supported(self, op_io):
        self.op_io = op_io
        shape_in_w = self.op_io.shape_in_w
        shape_in_h = self.op_io.shape_in_h
        shape_out_w = self.op_io.shape_out_w
        shape_out_h = self.op_io.shape_out_h
        type_in = self.op_io.type_in
        order_in = self.op_io.order_in
        
        # Check format, order, type is supported by the operator
        if (not (check_param_supported(self.op_io,
                                        self.format_in_supported,
                                        self.format_out_supported,
                                        self.order_in_supported,
                                        self.order_out_supported,
                                        self.type_in_supported,
                                        self.type_out_supported))):
            return False

        # Check Crop parameter is valid
        if ((self.width < 0) or (self.height < 0) or 
            (self.tl_x < 0) or (self.tl_y < 0) or 
            (not isinstance(self.width, int)) or (not isinstance(self.height, int)) or 
            (not isinstance(self.tl_x, int)) or (not isinstance(self.tl_y, int))):
            print("[ERROR] Crop parameter not valid.")
            print("          crop_tl_x = ", self.tl_x)
            print("          crop_tl_y = ", self.tl_y)
            print("          width     = ", self.width)
            print("          height    = ", self.height)
            return False
        #check restriction stated in DRP-AI Translator.
        if (self.tl_x > shape_in_w - 1):
            print("[ERROR] crop_tl_x not supported.")
            print("        Requirement: crop_tl_x < shape_in width")
            print("          crop_tl_x = ", self.tl_x)
            print("          width     = ", shape_in_w)
            return False
        
        if (self.tl_y > shape_in_h - 1):
            print("[ERROR] crop_tl_y not supported.")
            print("        Requirement: crop_tl_y < shape_in height")
            print("          crop_tl_y = ", self.tl_y)
            print("          height    = ", shape_in_h)
            return False
        # Check shape_in > shape_out.
        if (shape_in_w < shape_out_w):
            print("[ERROR] shape not supported.")
            print("        Requirement: shape_in width >= shape_out width")
            print("          shape_in_w  = ", shape_in_w)
            print("          shape_out_w = ", shape_out_w)
            return False
        if (shape_in_h < shape_out_h):
            print("[ERROR] shape not supported.")
            print("        Requirement: shape_in height >= shape_out height")
            print("          shape_in_h  = ", shape_in_h)
            print("          shape_out_h = ", shape_out_h)
            return False
        # Prepare the input parameter to DRP-AI Translator Python API
        if (type_in == drpai_param.TYPE.UINT8):
            self.data_type = drpai_param.TYPE.UINT8
        elif (type_in == drpai_param.TYPE.FP16):
            self.data_type = drpai_param.TYPE.FP16
        else:
            pass
        return True

    def print_op(self):
        print("  Crop: ", self.tl_x, self.tl_y, self.width, self.height)
    def get_api(self):
        api_list = [
            crop(
                shape_out=[self.height, self.width], 
                CROP_POS_X=self.tl_x,
                CROP_POS_Y=self.tl_y, 
                DATA_TYPE=self.data_type, 
                DATA_FORMAT=self.data_format
            )
        ]
        return api_list


class Resize(Op):
    """
    External class that user may call for following operator.
    DRP-AI Translator operator: resize_hwc
    """
    NEAREST = 0
    BILINEAR = 1
    name = "Resize"
    # Supported parameter list
    format_in_supported = {
        drpai_param.FORMAT.RGB,
        drpai_param.FORMAT.BGR,
        drpai_param.FORMAT.GRAY
    }
    format_out_supported = format_in_supported
    order_in_supported = {
        drpai_param.ORDER.HWC
    }
    order_out_supported = order_in_supported
    type_in_supported = {
        drpai_param.TYPE.UINT8,
        drpai_param.TYPE.FP16,
    }
    type_out_supported = type_in_supported

    def __init__(self, width=0, height=0, algorithm=NEAREST):
        self.width = width
        self.height = height
        self.algorithm = algorithm
        self.data_type = 0 # uint8
        self.op_io = None

    def is_supported(self, op_io):
        self.op_io = op_io
        shape_in_w = self.op_io.shape_in_w
        shape_in_h = self.op_io.shape_in_h
        shape_in_c = self.op_io.shape_in_c
        shape_out_w = self.op_io.shape_out_w
        shape_out_h = self.op_io.shape_out_h
        type_in = self.op_io.type_in
        type_out = self.op_io.type_out
        # Check format, order, type is supported by the operator
        if (not (check_param_supported(self.op_io,
                                        self.format_in_supported,
                                        self.format_out_supported,
                                        self.order_in_supported,
                                        self.order_out_supported,
                                        self.type_in_supported,
                                        self.type_out_supported))):
            return False

        # Check Resize parameter is valid
        if ((self.width < 0) or (self.height < 0) or 
            (not isinstance(self.width, int)) or (not isinstance(self.height, int)) or
            ((self.algorithm!= self.NEAREST) and (self.algorithm != self.BILINEAR))):
            print("[ERROR] Resize parameter not valid.")
            print("          width     = ", self.width)
            print("          height    = ", self.height)
            print("          algorithm = ", self.algorithm)
            return False
        # Check restriction stated in DRP-AI Translator.
        if ((shape_in_w <=2) or (shape_in_w > 4096)):
            print("[ERROR] shape_in width not supported.")
            print("        Requirement: 2 < shape_in width <= 4096")
            print("          shape_in_w  = ", shape_in_w)
            return False
        if ((shape_in_h <=2) or (shape_in_h > 4096)):
            print("[ERROR] shape_in height not supported.")
            print("        Requirement: 2 < shape_in height <= 4096")
            print("          shape_in_h  = ", shape_in_h)
            return False
        if (shape_in_c > 512):
            print("[ERROR] shape_in ch not supported.")
            print("        Requirement: shape_in ch <= 512")
            print("          shape_in_c  = ", shape_in_c)
            return False
        if ((shape_out_w <=2) or (shape_out_w > 4096)):
            print("[ERROR] shape_out width not supported.")
            print("        Requirement: 2 < shape_out width <= 4096")
            print("          shape_out_w  = ", shape_out_w)
            return False
        if ((shape_out_h <=2) or (shape_out_h > 4096)):
            print("[ERROR] shape_out height not supported.")
            print("        Requirement: 2 < shape_out height <= 4096")
            print("          shape_out_h  = ", shape_out_h)
            return False
        if (type_in == drpai_param.TYPE.FP16):
            self.data_type = 1
        return True
    def print_op(self):
        print("  Resize: ", self.width, self.height, self.algorithm)
    def get_api(self):
        api_list = [
            resize_hwc(
                shape_out=[self.height, self.width], 
                RESIZE_ALG=self.algorithm, 
                DATA_TYPE=self.data_type
            )
        ]
        return api_list

class Normalize(Op):
    """
    External class that user may call for following operators.
    DRP-AI Translator operator: cast_any_to_fp16
                                normalize
    """
    name = "Normalize"
    # Supported parameter list
    format_in_supported = {
        drpai_param.FORMAT.RGB,
        drpai_param.FORMAT.BGR,
        drpai_param.FORMAT.GRAY
    }
    format_out_supported = format_in_supported
    order_in_supported = {
        drpai_param.ORDER.HWC
    }
    order_out_supported = order_in_supported
    type_in_supported = {
        drpai_param.TYPE.UINT8,
        drpai_param.TYPE.FP16,
        drpai_param.TYPE.FP32
    }
    type_out_supported = {
        drpai_param.TYPE.FP16
    }
    
    def __init__(self, cof_add = None, cof_mul = None):
        self.cof_add = cof_add
        self.cof_mul = cof_mul
        self.op_io = None
        self.din_format = drpai_param.FORMAT.UNKNOWN
        self.dout_rgb_order = 0

    
    def is_supported(self, op_io):
        self.op_io = op_io
        type_in = self.op_io.type_in
        format_in = self.op_io.format_in
        format_out = self.op_io.format_out
        shape_in_w = self.op_io.shape_in_w
        shape_in_c = self.op_io.shape_in_c
        
        # Check format, order, type is supported by the operator
        if (not (check_param_supported(self.op_io,
                                        self.format_in_supported,
                                        self.format_out_supported,
                                        self.order_in_supported,
                                        self.order_out_supported,
                                        self.type_in_supported,
                                        self.type_out_supported))):
            return False

        # Check format is supported
        if (((format_in == drpai_param.FORMAT.GRAY) and (format_out != drpai_param.FORMAT.GRAY)) or
            ((format_in != drpai_param.FORMAT.GRAY) and (format_out == drpai_param.FORMAT.GRAY))):
            print("[ERROR] format not supported.")
            print("        Conversion of RGB/BGR and GRAY not supported.")
            print("        format_in  =", drpai_param.FORMAT.STR[format_in])
            print("        format_out =", drpai_param.FORMAT.STR[format_out])
            return False

        # Check if the cof_add/cof_mul are float list
        if (not self.check_cof(self.cof_add, "cof_add")):
            return False
        if (not self.check_cof(self.cof_mul, "cof_mul")):
            return False

        # Check format and cof_add/mul length matches 
        if (len(self.cof_add) != len(self.cof_mul)):
            print("[ERROR] cof_add/cof_mul invalid.")
            print("        cof_add =", self.cof_add)
            print("        cof_mul =", self.cof_mul)
            return False
        if ((format_in != drpai_param.FORMAT.GRAY) and (len(self.cof_add) != drpai_param.FORMAT.N_CH_3)):
            print("[ERROR] format and cof_add/cof_mul invalid.")
            print("        format_in  =", drpai_param.FORMAT.STR[format_in])
            print("        format_out =", drpai_param.FORMAT.STR[format_out])
            print("        cof_add =", self.cof_add)
            print("        cof_mul =", self.cof_mul)
            return False
        elif ((format_in == drpai_param.FORMAT.GRAY) and (len(self.cof_add) != drpai_param.FORMAT.N_CH_1)):
            print("[ERROR] format and cof_add/cof_mul invalid.")
            print("        format_in  =", drpai_param.FORMAT.STR[format_in])
            print("        format_out =", drpai_param.FORMAT.STR[format_out])
            print("        cof_add =", self.cof_add)
            print("        cof_mul =", self.cof_mul)
            return False
        else:
            pass

        # Define the DRP-AI Translator Python API parameter
        if ((format_in != drpai_param.FORMAT.GRAY) and (format_in != format_out)):
            self.dout_rgb_order = 1
        # Check type is supported and define the DRP-AI Translator Python API parameter
        if (self.dout_rgb_order == 1 and type_in != drpai_param.TYPE.UINT8):
            print("[ERROR] type_in and format not supported.")
            print("        type_in must be uint8 if format_in != format_out.")
            print("          type_in    :", drpai_param.TYPE.STR[type_in])
            print("          format_in  :", drpai_param.FORMAT.STR[format_in])
            print("          format_out :", drpai_param.FORMAT.STR[format_out])
            return False
        self.din_format = type_in
        
        # Check restriction stated in DRP-AI Translator except shape equality.
        # For cast_any_to_fp16
        size = shape_in_w * shape_in_c
        if (0xFFFFFFFF <= size):
            print("[ERROR] shape_in not supported.")
            print("        Requirement: width*ch < 0xFFFFFFFF")
            print("          width = ", self.op_io.shape_in_w)
            print("          ch    = ", self.op_io.shape_in_c)
            return False
        # For normalize, none
        return True

    def print_op(self):
        print("  Normalize: ", self.cof_add, self.cof_mul)
    def get_api(self):
        return [
            cast_any_to_fp16(DIN_FORMAT=self.din_format),
            normalize(cof_add = self.cof_add, cof_mul = self.cof_mul, DOUT_RGB_ORDER=self.dout_rgb_order)
        ]

    def check_cof(self, cof, label):
        """
        Unique method for Normalize class.
        Check cof_add/mul are float or not.
        Args:
            cof (list): cof_add or cof_mul
            label (string): string to shown in error log
        Returns:
            True is cof is valid.
            False if not.
        """
        for i in range(len(cof)):
            if (type(cof[i]) is int):
                # if int, convert it to float
                cof[i] = float(cof[i])
            elif (type(cof[i]) is not float):
                print("[ERROR]",label,"is not number.")
                print("       ",label,"=", cof)
                return False
            else:
                pass
        return True
    
class ConvColor(Op):
    """
    Internal class that runtime specifies the necessity for following operators.
    DRP-AI Translator operator: conv_yuv2rgb
                                conv_x2gray
    """
    name = "ConvColor"
    # Supported parameter list
    format_in_supported = {
        drpai_param.FORMAT.YUYV_422,
        drpai_param.FORMAT.YVYU_422,
        drpai_param.FORMAT.UYUV_422,
        drpai_param.FORMAT.VUYY_422,
        drpai_param.FORMAT.YUYV_420,
        drpai_param.FORMAT.UYVY_420,
        drpai_param.FORMAT.YV12_420,
        drpai_param.FORMAT.IYUV_420,
        drpai_param.FORMAT.NV12_420,
        drpai_param.FORMAT.NV21_420,
        drpai_param.FORMAT.IMC1_420,
        drpai_param.FORMAT.IMC2_420,
        drpai_param.FORMAT.IMC3_420,
        drpai_param.FORMAT.IMC4_420,
        drpai_param.FORMAT.RGB,
        drpai_param.FORMAT.BGR,
    }
    format_out_supported = {
        drpai_param.FORMAT.RGB,
        drpai_param.FORMAT.BGR,
        drpai_param.FORMAT.GRAY
    }
    order_in_supported = {
        drpai_param.ORDER.HWC
    }
    order_out_supported = order_in_supported
    type_in_supported = {
        drpai_param.TYPE.UINT8
    }
    type_out_supported = type_in_supported

    def __init__(self):
        self.format_in = drpai_param.FORMAT.UNKNOWN
        self.format_out = drpai_param.FORMAT.UNKNOWN
        self.is_yuv2rgb = 0
        self.op_io = None

    def is_supported(self, op_io):
        self.format_in = op_io.format_in
        self.format_out = op_io.format_out
        self.op_io = op_io
        # Check format, order, type is supported by the operator
        if (not (check_param_supported(self.op_io,
                                        self.format_in_supported,
                                        self.format_out_supported,
                                        self.order_in_supported,
                                        self.order_out_supported,
                                        self.type_in_supported,
                                        self.type_out_supported))):
            return False
        
        # Check special not supported format
        if ((self.format_in & drpai_param.FORMAT.YUV420) and 
            (self.format_out == drpai_param.FORMAT.GRAY)):
            print("[ERROR] YUV420 to Gray conversion is not supported.")
            print("        format_in  =", drpai_param.FORMAT.STR[self.format_in])
            print("        format_out =", drpai_param.FORMAT.STR[self.format_out])
            return False
        if ((self.format_in == drpai_param.FORMAT.GRAY) and 
            (self.format_out != drpai_param.FORMAT.GRAY)):
            print("[ERROR] Gray to RGB/BGR conversion is not supported.")
            print("        format_in  =", drpai_param.FORMAT.STR[self.format_in])
            print("        format_out =", drpai_param.FORMAT.STR[self.format_out])
            return False
        # Find which operator to be run.
        if (self.format_out == drpai_param.FORMAT.GRAY) :
            self.is_yuv2rgb = 0
        else:
            # format_out == RGB | BGR
            self.is_yuv2rgb = 1
        # Check operator restriction
        shape_in_w = self.op_io.shape_in_w
        shape_in_h = self.op_io.shape_in_h
        if (self.is_yuv2rgb):
            if (shape_in_w % 2 != 0):
                print("[ERROR] shape_in not supported.")
                print("        Requirement: shape_in_w % 2 == 0")
                print("          width  = ", shape_in_w)
                return False
            if ((shape_in_w < 4) or (shape_in_w > 65535)):
                print("[ERROR] shape_in not supported.")
                print("        Requirement: 4 <= width <= 65535")
                print("          width = ", shape_in_w)
                return False
            if ((shape_in_h < 5) or (shape_in_h > 65535)):
                print("[ERROR] shape_in not supported.")
                print("        Requirement: 5 <= height <= 65535")
                print("          height = ", shape_in_h)
                return False
        else:
            # For conv_x2gray
            if (self.format_in & drpai_param.FORMAT.NOT_YUY2):
                if (shape_in_w % 2 != 0):
                    print("[ERROR] shape_in not supported.")
                    print("        Requirement: shape_in_w % 2 == 0")
                    print("                     if format_in = RGB|BGR and format_out = GRAY.")
                    print("          width  = ", shape_in_w)
                    return False
        return True

    def print_op(self):
        print("  ConvColor: ", self.format_in, self.format_out)

    def get_api(self):
        api_list = []
        if (self.is_yuv2rgb):
            api_list = [ conv_yuv2rgb(DIN_YUV_FORMAT=self.format_in, DOUT_RGB_FORMAT=(self.format_out & 1))]
        else:
            d_in = self.format_in
            # Convert RGB/BGR into DRP-AI Translator parameter
            if (d_in & drpai_param.FORMAT.NOT_YUY2):
                d_in = 0x1000 + (d_in & 0x1)
            api_list = [ conv_x2gray(DIN_FORMAT=d_in)]
        return api_list


class Transpose(Op):
    """
    Internal class that runtime specifies the necessity for following operators.
    DRP-AI Translator operator: transpose
    """
    name = "Transpose"
    # Supported parameter list
    format_in_supported = {
        drpai_param.FORMAT.RGB,
        drpai_param.FORMAT.BGR,
        drpai_param.FORMAT.GRAY
    }
    format_out_supported = format_in_supported
    # post mode support is included
    order_in_supported = {
        drpai_param.ORDER.HWC,
        drpai_param.ORDER.CHW
    }
    # post mode support is included
    order_out_supported = order_in_supported
    type_in_supported = {
        drpai_param.TYPE.UINT8,
        drpai_param.TYPE.FP16
    }
    type_out_supported = type_in_supported

    def __init__(self, mode=drpai_param.MODE.PRE):
        self.op_io = None
        self.is_chw2hwc = 0
        self.word_size = drpai_param.TYPE.UINT8
        self.mode = mode

    def is_supported(self, op_io):
        self.op_io = op_io
        order_in = self.op_io.order_in
        type_in = self.op_io.type_in
        
        # Check format, order, type is supported by the operator
        if (not (check_param_supported(self.op_io,
                                        self.format_in_supported,
                                        self.format_out_supported,
                                        self.order_in_supported,
                                        self.order_out_supported,
                                        self.type_in_supported,
                                        self.type_out_supported,
                                        self.mode))):
            return False

        # Define the DRP-AI Translator Python API parameter
        if (order_in == drpai_param.ORDER.HWC):
            self.is_chw2hwc = 0
        else:
            # Only used for post mode.
            self.is_chw2hwc = 1

        # Check type is supported and define the DRP-AI Translator Python API parameter
        self.word_size = type_in
        return True

    def get_api(self):
        api_list = [
            transpose(WORD_SIZE=self.word_size, IS_CHW2HWC=self.is_chw2hwc)
        ]
        return api_list

class Cast(Op):
    """
    Internal class that runtime specifies the necessity for following operators.
    DRP-AI Translator operator: cast_fp16_fp32
    """
    name = "Cast"
    # Supported parameter list
    format_in_supported = drpai_param.FORMAT.STR.keys()
    format_out_supported = format_in_supported
    order_in_supported = drpai_param.ORDER.STR.keys()
    order_out_supported = order_in_supported
    type_in_supported = {
        drpai_param.TYPE.FP16,
        drpai_param.TYPE.FP32
    }
    type_out_supported = type_in_supported

    def __init__(self, mode=drpai_param.MODE.PRE):
        self.type_in = drpai_param.TYPE.UNKNOWN
        self.type_out = drpai_param.TYPE.UNKNOWN
        self.op_io = None
        self.cast_mode = 0
        self.mode = mode

    def is_supported(self, op_io):
        self.op_io = op_io
        self.type_in = op_io.type_in
        self.type_out = op_io.type_out
        shape_in_w = self.op_io.shape_in_w
        shape_in_h = self.op_io.shape_in_h
        shape_in_c = self.op_io.shape_in_c
        # Check format, order, type is supported by the operator
        if (not (check_param_supported(self.op_io,
                                        self.format_in_supported,
                                        self.format_out_supported,
                                        self.order_in_supported,
                                        self.order_out_supported,
                                        self.type_in_supported,
                                        self.type_out_supported,
                                        self.mode))):
            return False

        # Check restriction stated in DRP-AI Translator except shape equality.
        size = shape_in_w * shape_in_h * shape_in_c
        if (0xFFFFFFFF < size):
            print("[ERROR] shape_in not supported.")
            print("        Requirement: width*height*ch <= 0xFFFFFFFF")
            print("          width  = ", self.op_io.shape_in_w)
            print("          height = ", self.op_io.shape_in_h)
            print("          ch     = ", self.op_io.shape_in_c)
            return False
        
        # Check type is supported and define the DRP-AI Translator Python API parameter
        self.cast_mode = 0 # fp16 to fp32
        if (self.type_in == drpai_param.TYPE.FP32):
            self.cast_mode = 1 # fp32 to fp16
        return True

    def print_op(self):
        print("  Cast: ", self.type_in, self.type_out)

    def get_api(self):
        api_list = [
            cast_fp16_fp32(CAST_MODE=self.cast_mode)
        ]
        return api_list
class Argminmax(Op):
    """
    Post mode only
    Internal class that runtime specifies the necessity for following operators.
    DRP-AI Translator operator: argminmax
    """
    name = "Argminmax"
    # Supported parameter list
    # Note that format is not required for Post mode.
    format_in_supported = drpai_param.FORMAT.STR.keys()
    format_out_supported = format_in_supported
    order_in_supported ={
        drpai_param.ORDER.HWC,
        drpai_param.ORDER.CHW
    }
    order_out_supported = order_in_supported
    type_in_supported = {
        drpai_param.TYPE.FP16
    }
    type_out_supported = {
        drpai_param.TYPE.UINT8
    }

    label = {
        0 : "ch",
        1 : "width",
        2 : "height"
    }

    def __init__(self, axis=0, arg_mode = 0):
        self.axis = axis
        self.arg_mode = arg_mode
        self.op_io = None
        self.din_format = 0
        self.dout_type = 1
        self.mode = drpai_param.MODE.POST

    def is_supported(self, op_io):
        self.op_io = op_io
        order_in = op_io.order_in
        type_out = op_io.type_out
        shape_in_h = self.op_io.shape_in_h
        shape_in_w = self.op_io.shape_in_w
        shape_in_c = self.op_io.shape_in_c
        # Check format, order, type is supported by the operator
        if (not (check_param_supported(self.op_io,
                                        self.format_in_supported,
                                        self.format_out_supported,
                                        self.order_in_supported,
                                        self.order_out_supported,
                                        self.type_in_supported,
                                        self.type_out_supported,
                                        self.mode))):
            return False

        # Check user input is valid format
        if (self.arg_mode < 0 or self.arg_mode > 1):
            print("[ERROR] arg_mode not valid: arg_mode=", self.arg_mode)
            return False

        # Check restriction stated in DRP-AI Translator except shape equality.
        restricted_axis = 0
        if (self.axis == 0):
            restricted_axis = shape_in_c
        elif (self.axis == 1):
            restricted_axis = shape_in_w
        elif (self.axis == 2):
            restricted_axis = shape_in_h
        else:
            print("[ERROR] parameter not supported: axis=", self.axis)
            print("        Requirement: axis == (0 | 1 | 2)")
            return False
        
        if (restricted_axis > 256):
            print("[ERROR] shape_in not supported.")
            print("        Requirement: Argminmax axis size <= 256")
            print("         ",self.label[self.axis],"= ", restricted_axis)
            return False

        # Check order and type is supported and define the DRP-AI Translator Python API parameter
        self.din_format = order_in
        self.dout_type = type_out
        return True

    def print_op(self):
        print("  Argminmax: ", self.axis, self.arg_mode)

    def get_api(self):
        api_list = [
            argminmax(
                DIN_FORMAT=self.din_format,
                DOUT_TYPE=self.dout_type,
                AXIS=self.axis,
                ARG_MODE=self.arg_mode
            )
        ]
        return api_list
class Softmax(Op):
    """
    Post mode only
    Internal class that runtime specifies the necessity for following operators.
    DRP-AI Translator operator: softmax
    """
    name = "Softmax"
    # Supported parameter list
    # Note that format is not required for Post mode.
    format_in_supported = drpai_param.FORMAT.STR.keys()
    format_out_supported = format_in_supported
    order_in_supported ={
        drpai_param.ORDER.C
    }
    order_out_supported = order_in_supported
    type_in_supported = {
        drpai_param.TYPE.FP16
    }
    type_out_supported = {
        drpai_param.TYPE.FP16,
        drpai_param.TYPE.FP32
    }

    def __init__(self):
        self.dout_format = 0 # fp16
        self.op_io = None
        self.mode = drpai_param.MODE.POST

    def is_supported(self, op_io):
        self.op_io = op_io
        type_out = op_io.type_out
        shape_in_c = self.op_io.shape_in_c
        # Check format, order, type is supported by the operator
        if (not (check_param_supported(self.op_io,
                                        self.format_in_supported,
                                        self.format_out_supported,
                                        self.order_in_supported,
                                        self.order_out_supported,
                                        self.type_in_supported,
                                        self.type_out_supported,
                                        self.mode))):
            return False

        # Check restriction stated in DRP-AI Translator except shape equality.
        if ((shape_in_c < 1) or (shape_in_c > 16384)):
            print("[ERROR] shape_in not supported.")
            print("        Requirement: 1 <= ch <= 16384")
            print("          ch     = ", shape_in_c)
            return False
        
        # Check type is supported and define the DRP-AI Translator Python API parameter
        if (type_out == drpai_param.TYPE.FP32):
            self.dout_format = 1 # fp32
        return True

    def print_op(self):
        print("  Softmax")

    def get_api(self):
        api_list = [
            softmax(DOUT_FORMAT=self.dout_format)
        ]
        return api_list