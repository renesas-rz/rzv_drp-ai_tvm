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
import shutil
import sys
import yaml
from . import op, drpai_param

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

class RuntimeSupportedParam():
    """
    Class to store supported I/O parameter in DRP-AI Preprocessing Runtime
    """
    FORMAT_IN = drpai_param.FORMAT.STR.keys()
    FORMAT_OUT= {
        drpai_param.FORMAT.RGB,
        drpai_param.FORMAT.BGR,
        drpai_param.FORMAT.GRAY
    }
    ORDER_IN = {
        drpai_param.ORDER.HWC
    }
    ORDER_OUT = {
        drpai_param.ORDER.HWC,
        drpai_param.ORDER.CHW
    }
    ORDER_IN_POST = drpai_param.ORDER.STR.keys()
    ORDER_OUT_POST = drpai_param.ORDER.STR.keys()
    TYPE_IN = drpai_param.TYPE.STR.keys()
    TYPE_OUT = drpai_param.TYPE.STR.keys()
    OP = {
        op.Crop(),
        op.Resize(),
        op.Normalize()
    }
    OP_POST = {
        op.Softmax(),
        op.Argminmax()
    }

def update_op_io(op_io, current_list, config):
    """
    Method to update Op_IO based on the User input or previous operator
    Args:
        op_io (Op_IO): Operator In/Out information to be updated
        current_list (list): current operator list
        config (Config): User input information
    Returns:
        op_io (Op_IO): Updated Operator In/Out information 
    """
    if (len(current_list) > 0):
        # if there are previous operators, copy op_io data from previous operator
        prev = current_list[-1].op_io
        op_io.cp_prev(prev)
    else:
        # if this is first operator, copy io data from preruntime io.
        op_io = config.assign_io()    
    return op_io

def in_list(param, list, log):
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
        print("[ERROR] Not valid input:", log, "=",param)
        return False
    return True

def check_shape_post(shape, order, label):
    """
    Method to check the shape is valid for post mode.
    Args:
        shape (list): shape to be checked
        order (int) : order of shape, i.e. HWC
        label (string): label to be shown on error log. i.e., "in" or "out"
    Returns:
        True if valid. 
        False if not.
    """
    shape_str = "shape_"+label
    order_str = "order_"+label
    length = len(shape)
    if ((4 != length) and (2 != length)):
        print("[ERROR]",shape_str,"not supported: ", shape)
        print("       ",shape_str,"list length must be 4 or 2.")
        return False
    if ((order == drpai_param.ORDER.C) and (4 == length)):
        print("[ERROR] shape and order do not match.")
        print("         ",shape_str,"=", shape)
        print("         ",order_str,"=", drpai_param.ORDER.STR[order])
        return False
    return True

def check_ch_size_valid(shape, format, order, label):
    """
    Method to check the shape, format, order is valid for post mode.
    Args:
        shape (list): shape to be checked
        format (int) : format of shape, i.e. BGR
        order (int) : order of shape, i.e. HWC
        label (string): label to be shown on error log. i.e., "in" or "out"
    Returns:
        True if valid. 
        False if not.
    """
    shape_str = "shape_"+label
    format_str = "format_"+label
    order_str = "order_"+label
    length = len(shape)
    ch_index = 3 #for HWC
    if (order == drpai_param.ORDER.CHW):
        ch_index = 1
    
    ch_size = shape[ch_index]
    # Check if format=YUV
    if not (format & drpai_param.FORMAT.NOT_YUY2):
        if (2 == ch_size):
            return True
    # Check if format=Gray
    elif (format == drpai_param.FORMAT.GRAY):
        if (1 == ch_size):
            return True
    # Check if format=RGB|BGR
    elif (format & drpai_param.FORMAT.NOT_YUY2):
        if (3 == ch_size):
            return True
    else:
        pass
    print("[ERROR] shape, format and order do not match.")
    print("         ",shape_str,"=", shape)
    print("         ",format_str,"=", drpai_param.FORMAT.STR[format])
    print("         ",order_str,"=", drpai_param.ORDER.STR[order])
    return False


def check_param_valid(config):
    """
    Method to check the user input is valid. 
    (Does not include the check for supported parameter by runtime and each operator.)
    Args:
        config (Config): User input
    Returns:
        True if valid. 
        False if not.
    """
    post_mode = config.mode
    # Check if shape_in/out is NCHW/NHWC
    in_len = len(config.shape_in)
    out_len = len(config.shape_out)
    if (post_mode):
        if (in_len != out_len):
            print("[ERROR] shape not supported.")
            print("        shape_in and shape_out list length must be the same")
            print("        shape_in =", config.shape_in)
            print("        shape_out =", config.shape_out)
            return False
        if (not check_shape_post(config.shape_in, config.order_in, "in")):
            return False
        if (not check_shape_post(config.shape_out, config.order_out, "out")):
            return False
    else:
        if (in_len != 4):
            print("[ERROR] shape_in not supported: ", config.shape_in)
            print("        shape_in list length must be 4.")
            return False
        if (out_len != 4):
            print("[ERROR] shape_out not supported: ", config.shape_out)
            print("        shape_out list length must be 4.")
            return False
    # Check the shape_in/out is list of integer
    for i in config.shape_in:
        if (type(i) is not int):
            print("[ERROR] shape_in must be integer", config.shape_in)
            return False
    for i in config.shape_out:
        if (type(i) is not int):
            print("[ERROR] shape_out must be integer", config.shape_out)
            return False
    if (not post_mode):
        # Check input format is valid
        if not in_list(config.format_in, drpai_param.FORMAT.STR.keys(), "format_in"):
            return False
        if not check_ch_size_valid(config.shape_in, config.format_in, config.order_in, "in"):
            return False
        # Check output format is valid
        if not in_list(config.format_out, drpai_param.FORMAT.STR.keys(), "format_out"):
            return False
        if not check_ch_size_valid(config.shape_out, config.format_out, config.order_out, "out"):
            return False
    # Check input order is valid
    if not in_list(config.order_in, drpai_param.ORDER.STR.keys(), "order_in"):
        return False
    # Check output order is valid
    if not in_list(config.order_out, drpai_param.ORDER.STR.keys(), "order_out"):
        return False
    # Check input type is valid
    if not in_list(config.type_in, drpai_param.TYPE.STR.keys(), "type_in"):
        return False
    # Check output type is valid
    if not in_list(config.type_out, drpai_param.TYPE.STR.keys(), "type_out"):
        return False
    return True

def check_param_supported(config):
    """
    Method to check the user input is supported by the runtime. 
    (Does not include the check for supported parameter by each operator.)
    Args:
        config (Config): User input
    Returns:
        True if valid. 
        False if not.
    """
    post_mode = config.mode
    if (not post_mode):
        # Check input format is supported
        if not in_list(config.format_in, RuntimeSupportedParam.FORMAT_IN, "format_in"):
            return False
        # Check output format is supported
        if not in_list(config.format_out, RuntimeSupportedParam.FORMAT_OUT, "format_out"):
            return False

    # Check input order is supported
    list = []
    if (post_mode):
        list = RuntimeSupportedParam.ORDER_IN_POST
    else:
        list = RuntimeSupportedParam.ORDER_IN
    if not in_list(config.order_in, list, "order_in"):
        return False

    # Check output order is supported
    if (post_mode):
        list = RuntimeSupportedParam.ORDER_OUT_POST
    else:
        list = RuntimeSupportedParam.ORDER_OUT
    if not in_list(config.order_out, list, "order_out"):
        return False

    # Check the order matches in post mode
    if (post_mode):
        if ((config.order_in == drpai_param.ORDER.C) or (config.order_out == drpai_param.ORDER.C )):
            if (config.order_in != config.order_out):
                print("[ERROR] order not supported. (POST MODE ONLY)")
                print("          order_in :", drpai_param.ORDER.STR[config.order_in])
                print("          order_out:", drpai_param.ORDER.STR[config.order_out])
                return False

    # Check input type is supported
    if not in_list(config.type_in, RuntimeSupportedParam.TYPE_IN, "type_in"):
        return False
    # Check output type is supported
    if not in_list(config.type_out, RuntimeSupportedParam.TYPE_OUT, "type_out"):
        return False
    return True


def check_convcolor(config):
    """
    Check if ConvColor operator is necessary
    Args:
        config (Config): Configuration of DRP-AI Pre-processing Runtime.
    Returns:
        bool: True if operator is required. False if not.
    """
    f_in = config.format_in
    f_out = config.format_out

    # Check if In=YUV
    if not (f_in & drpai_param.FORMAT.NOT_YUY2):
        return True
    # Check if In=RGB|BGR & Out=Gray 
    elif ((f_in & drpai_param.FORMAT.NOT_YUY2) and 
          (f_in != drpai_param.FORMAT.GRAY) and 
          (f_out == drpai_param.FORMAT.GRAY)):
        return True
    else:
        pass
    return False
    
def check_transpose(config):
    """
    Check if Transpose operator is necessary
    Args:
        config (Config): Configuration of DRP-AI Pre-processing Runtime.
    Returns:
        bool: True if operator is required. False if not.
    """
    o_in = config.order_in
    o_out = config.order_out
    if (o_in != drpai_param.ORDER.C) and (o_out != drpai_param.ORDER.C):
        if (o_in != o_out):
            return True
    return False

def check_cast(config, fp32tofp16=0, other_ops=None):
    """
    Check if Cast operator is necessary
    Args:
        config (Config): Configuration of DRP-AI Pre-processing Runtime.
        fp32tofp16 (bool): Flag to distinguish conversion. Set 1 for fp32 to fp16 conversion.
        other_ops (list): list of operators specified by user.
    Returns:
        bool: True if operator is required. False if not.
    """
    t_in = config.type_in
    t_out = config.type_out
    if (fp32tofp16):
        if (config.mode):
            if ((t_in == drpai_param.TYPE.FP32)):
                if (t_out == drpai_param.TYPE.FP32):
                    if ((check_transpose(config)) or
                        (-1 != get_op_index(other_ops, op.Argminmax())) or
                        (-1 != get_op_index(other_ops, op.Softmax()))):
                        return True
                else:
                    return True
        else:
            if (other_ops == None):
                return False
            # if Crop/Resize is used
            if ((t_in == drpai_param.TYPE.FP32) and 
                (t_out != drpai_param.TYPE.UINT8)):
                if ((-1 != get_op_index(other_ops, op.Crop())) or
                    (-1 != get_op_index(other_ops, op.Resize()))):
                    return True
    else:
        if (config.mode):
            if  (t_out == drpai_param.TYPE.FP32):
                if ((-1 == get_op_index(other_ops, op.Argminmax()))
                    and (-1 == get_op_index(other_ops, op.Softmax()))):
                        return True
        else:
            if (t_out == drpai_param.TYPE.FP32):
                return True
    return False

def check_normalize(config, other_ops):
    """
    Check Normalize operator is necessary even if user did not specify in the config.ops.
    Normalize will be used as FP16 conversion only.
    Args:
        config (Config): Configuration of DRP-AI Pre-processing Runtime.
        other_ops (list): list of operators specified by user.
    Returns:
        bool: True if operator is required. False if not.
    """
    t_in = config.type_in
    t_out = config.type_out
    f_in = config.format_in
    f_out = config.format_out

    if (t_out != drpai_param.TYPE.UINT8):
        # Check if the normalize is required for uint8 to fp16 cast
        if (t_in == drpai_param.TYPE.UINT8):
            return True
        # Check if the normalize is required for fp32 to fp16 cast
        elif (t_in == drpai_param.TYPE.FP32):
            if ((-1 == get_op_index(other_ops, op.Crop())) or
                (-1 == get_op_index(other_ops, op.Resize()))):
                return True
        else:
            pass
    # Check if the normalize is required for RGB/BGR conversion
    if (((f_in == drpai_param.FORMAT.RGB) and (f_out == drpai_param.FORMAT.BGR)) or
        ((f_in == drpai_param.FORMAT.BGR) and (f_out == drpai_param.FORMAT.RGB))):
        return True
    return False

def check_out_match(config, last_op_io):
    """
    Check last operator's output information matches User entered output information.
    Args:
        config (Config): Configuration of DRP-AI Pre-processing Runtime.
        last_op_io (Op_IO): last operator to be run
    Returns:
        bool: True if operator is required. False if not.
    """
    if (config.order_out == drpai_param.ORDER.HWC):
        if ((config.shape_out[1]!= last_op_io.shape_out_h) or 
            (config.shape_out[2]!= last_op_io.shape_out_w) or 
            (config.shape_out[3]!= last_op_io.shape_out_c)):
            print("[ERROR] shape_out not valid.")
            print("          specified shape_out=",config.shape_out)
            print("          actual shape_out   =",[1, last_op_io.shape_out_h, last_op_io.shape_out_w, last_op_io.shape_out_c])
            return False
    elif (config.order_out == drpai_param.ORDER.CHW):
        if ((config.shape_out[1]!= last_op_io.shape_out_c) or 
            (config.shape_out[2]!= last_op_io.shape_out_h) or 
            (config.shape_out[3]!= last_op_io.shape_out_w)):
            print("[ERROR] shape_out not valid.")
            print("          specified shape_out=",config.shape_out)
            print("          actual shape_out   =",[1, last_op_io.shape_out_c, last_op_io.shape_out_h, last_op_io.shape_out_w])
            return False
    elif (config.order_out == drpai_param.ORDER.C):
        if ((config.shape_out[1]!= last_op_io.shape_out_c)):
            print("[ERROR] shape_out not valid.")
            print("          specified shape_out=",config.shape_out)
            print("          actual shape_out   =",[1, last_op_io.shape_out_c])
            return False
    else:
        pass

    if (not config.mode):
        if (config.format_out != last_op_io.format_out):
            print("[ERROR] format_out not valid.")
            print("          specified format_out=",config.format_out)
            print("          actual format_out   =",last_op_io.format_out)
            return False
    if (config.order_out != last_op_io.order_out):
        print("[ERROR] order_out not valid.")
        print("          specified order_out=",config.order_out)
        print("          actual order_out   =",last_op_io.order_out)
        return False
    if (config.type_out != last_op_io.type_out):
        print("[ERROR] type_out not valid.")
        print("          specified type_out=",config.type_out)
        print("          actual type_out   =",last_op_io.type_out)
        return False
    return True

def get_op_index(list, empty_op):
    """
    Find the specified operator in the operator list and return its index.
    Args:
        list (list): list of operators.
        empty_op (class): Empty operator class to check which class to be found.
    Returns:
        int: index of operator found.  -1 if not found.
    """
    i = 0
    for o in list:
        if (isinstance(o, type(empty_op))):
            return i
        i = i+1
    return -1

class Config():
    """
    Class to store User input information.
    """
    def __init__(self):
        self.shape_in = []
        self.format_in = drpai_param.FORMAT.UNKNOWN
        self.order_in = drpai_param.ORDER.UNKNOWN
        self.type_in = drpai_param.TYPE.UNKNOWN
        self.shape_out = []
        self.format_out = drpai_param.FORMAT.UNKNOWN
        self.order_out = drpai_param.ORDER.UNKNOWN
        self.type_out = drpai_param.TYPE.UNKNOWN
        self.ops = [];
        
        # not public
        self.mode = drpai_param.MODE.PRE;
        self.debug = 0;
    
    def print_ops(self):
        print("Pre-processing Runtime Operation details")
        for op in self.ops:
            op.print_op()
    
    def assign_io(self):
        op_io = op.Op_IO()
        # shape_in
        if (self.order_in == drpai_param.ORDER.HWC):
            op_io.shape_in_h = self.shape_in[1]
            op_io.shape_in_w = self.shape_in[2]
            op_io.shape_in_c = self.shape_in[3]
        if (self.order_in == drpai_param.ORDER.CHW):
            op_io.shape_in_c = self.shape_in[1]
            op_io.shape_in_h = self.shape_in[2]
            op_io.shape_in_w = self.shape_in[3]
        elif (self.order_in == drpai_param.ORDER.C):
            op_io.shape_in_c = self.shape_in[1]
            op_io.shape_in_h = 0
            op_io.shape_in_w = 0
        else:
            pass

        op_io.format_in = self.format_in
        op_io.order_in = self.order_in
        op_io.type_in = self.type_in
        op_io.refresh()
        return op_io

    def generateYAML(self, filename="preproc.yaml"):
        # Initialize DRP-AI Translator PrePost class
        pp = drp_prepost()

        # Define in/out name for YAML
        in_name = "in"
        out_name = "out"

        # Check parameters are correct format and not random data.
        if (not check_param_valid(self)):
            sys.exit(-1)
        # Check parameters are supported by runtime.
        if (not check_param_supported(self)):
            sys.exit(-1)

        # Convert NHWC/NCHW/NC to HWC/CHW/C
        if ((len(self.shape_in)>1) and (len(self.shape_out)>1)):
            shape_in = self.shape_in[1:]
            shape_out = self.shape_out[1:]

        # Initialize Op_IO class and internal operator list
        op_io = self.assign_io()
        internal_op_list = []

        # Convert parameter into string for YAML
        order_in_str = drpai_param.ORDER.STR[self.order_in]
        order_out_str = drpai_param.ORDER.STR[self.order_out]
        type_in_str = drpai_param.TYPE.STR[self.type_in]
        type_out_str = drpai_param.TYPE.STR[self.type_out]
        
        # Initialize Pre & Post processing sequence
        pp_sequence = []

        if (self.mode == drpai_param.MODE.POST):
            #############
            # Post Mode #
            #############
            # Check user specified operator is supported in post mode.
            for o in self.ops:
                if (-1 != get_op_index(RuntimeSupportedParam.OP, o)):
                    print("[ERROR] Specified operator is not supported.")
                    print("       ",o.name)
                    sys.exit(-1)

            if (self.debug):
                print("Checking Cast (fp32 to fp16) is required...")
            if (check_cast(self, 1, self.ops)):
                operator = op.Cast(self.mode)
                op_io = update_op_io(op_io, internal_op_list, self)
                op_io.type_out = drpai_param.TYPE.FP16;
                if (self.debug):
                    op_io.print( self.mode)

                if (operator.is_supported(op_io)):
                    internal_op_list.append(operator)
                else:
                    print("Config I/O info not supported.")
                    sys.exit(-1)

            if (self.order_in == drpai_param.ORDER.C):
                # Check if user specified invalid operator (Argminmax)
                index = get_op_index(self.ops, op.Argminmax())
                if (-1 != index):
                    print("[ERROR] Argminmax cannot be specified when order == C.")
                    sys.exit(-1)

                if (self.debug):
                    print("Checking Softmax is required...")
                # Check Softmax is specified by user
                index = get_op_index(self.ops, op.Softmax())
                if (-1 != index):
                    operator = self.ops[index]
                    op_io = update_op_io(op_io, internal_op_list, self)
                    op_io.type_out = self.type_out
                    if (self.debug):
                        op_io.print( self.mode)

                    if (operator.is_supported(op_io)):
                        internal_op_list.append(operator)
                    else:
                        print("Config I/O info not supported.")
                        sys.exit(-1)
            else:
                # Check if user specified invalid operator (Softmax)
                index = get_op_index(self.ops, op.Softmax())
                if (-1 != index):
                    print("[ERROR] Softmax cannot be specified when order == HWC|CHW.")
                    sys.exit(-1)

                if (self.debug):
                    print("Checking Transpose is required...")
                if (check_transpose(self)):
                    operator = op.Transpose(self.mode)
                    op_io = update_op_io(op_io, internal_op_list, self)
                    op_io.order_out = (op_io.order_in + 1) % 2
                    if (self.debug):
                        op_io.print( self.mode)

                    if (operator.is_supported(op_io)):
                        internal_op_list.append(operator)
                    else:
                        print("Config I/O info not supported.")
                        sys.exit(-1)

                if (self.debug):
                    print("Checking Argminmax is required...")
                # Check Argminmax is specified by user
                index = get_op_index(self.ops, op.Argminmax())
                if (-1 != index):
                    operator = self.ops[index]
                    op_io = update_op_io(op_io, internal_op_list, self)
                    if (0 == operator.axis):
                        op_io.shape_out_c = 1
                    elif (1 == operator.axis):
                        op_io.shape_out_w = 1
                    elif (2 == operator.axis):
                        op_io.shape_out_h = 1
                    else :
                        pass
                    op_io.type_out = drpai_param.TYPE.UINT8
                    if (self.debug):
                        op_io.print(self.mode)

                    if (operator.is_supported(op_io)):
                        internal_op_list.append(operator)
                    else:
                        print("Config I/O info not supported in Argminmax operator.")
                        sys.exit(-1)

            if (self.debug):
                print("Checking Cast (fp16 to fp32) is required...")
            if (check_cast(self, 0, internal_op_list)):
                operator = op.Cast(self.mode)
                op_io = update_op_io(op_io, internal_op_list, self)
                op_io.type_out = self.type_out
                if (self.debug):
                    op_io.print( self.mode)
                if (operator.is_supported(op_io)):
                    internal_op_list.append(operator)
                else:
                    print("Config I/O info not supported.")
                    sys.exit(-1)

            # Check if there are any operators exist in internal_op_list.
            if (0 == len(internal_op_list)):
                print("[ERROR] No operator can be used.")
                print("        Please check your In/Out data and operators.")
                sys.exit(-1)

            # Check the final operator I/O information matches User input.
            if (not check_out_match(self, internal_op_list[-1].op_io)):
                print("        Please check your In/Out data and operators.")
                sys.exit(-1)

            # Get operator list in  DRP-AI Translator Python API
            for op_class in internal_op_list:
                op_api = op_class.get_api()
                pp_sequence = pp_sequence + op_api

            # Set post processing information
            pp.set_output_from_body(
                in_name,
                shape=shape_in,
                order=order_in_str,
                type=type_in_str
            )
            pp.set_output_from_post(
                out_name,
                shape=shape_out,
                order=order_out_str,
                type=type_out_str
            )

            # Set processing sequence
            pp.set_postprocess_sequence(
                src=[in_name],
                dest=[out_name],
                pp_seq=pp_sequence
            )
        else:
            ###############
            # Normal Mode #
            ###############
            format_in_str = drpai_param.FORMAT.STR[self.format_in]
            format_out_str = drpai_param.FORMAT.STR[self.format_out]
                
            # Check user specified operator is supported.
            for o in self.ops:
                if (-1 != get_op_index(RuntimeSupportedParam.OP_POST, o)):
                    print("[ERROR] Specified operator is not supported.")
                    print("       ",o.name)
                    sys.exit(-1)

            if (self.debug):
                print("Checking Cast (fp32 to fp16) is required...")
            if (check_cast(self, 1, self.ops)):
                operator = op.Cast()
                op_io = update_op_io(op_io, internal_op_list, self)
                op_io.type_out = drpai_param.TYPE.FP16;
                if (self.debug):
                    op_io.print()

                if (operator.is_supported(op_io)):
                    internal_op_list.append(operator)
                else:
                    print("Config I/O info not supported.")
                    sys.exit(-1)

            if (self.debug):
                print("Checking ConvColor is required...")
            if (check_convcolor(self)):
                operator = op.ConvColor()
                op_io = update_op_io(op_io, internal_op_list, self)
                op_io.format_out = self.format_out;
                if (op_io.format_out == drpai_param.FORMAT.GRAY):
                    op_io.shape_out_c = drpai_param.FORMAT.N_CH_1
                else:
                    op_io.shape_out_c = drpai_param.FORMAT.N_CH_3
                if (self.debug):
                    op_io.print()

                if (operator.is_supported(op_io)):
                    internal_op_list.append(operator)
                else:
                    print("Config I/O info not supported.")
                    sys.exit(-1)

            if (self.debug):
                print("Checking Crop is required...")
            # Check Crop is specified by user
            index = get_op_index(self.ops, op.Crop())
            if (-1 != index):
                operator = self.ops[index]
                op_io = update_op_io(op_io, internal_op_list, self)
                op_io.shape_out_h = operator.height
                op_io.shape_out_w = operator.width
                op_io.order_out = drpai_param.ORDER.HWC
                if (self.debug):
                    op_io.print()

                if (operator.is_supported(op_io)):
                    internal_op_list.append(operator)
                else:
                    print("Config I/O info not supported in Crop operator.")
                    sys.exit(-1)
            
            if (self.debug):
                print("Checking Resize is required...")
            # Check Resize is specified by user
            index = get_op_index(self.ops, op.Resize())
            if (-1 != index):
                operator = self.ops[index]
                op_io = update_op_io(op_io, internal_op_list, self)
                op_io.shape_out_h = operator.height
                op_io.shape_out_w = operator.width
                if (self.debug):
                    op_io.print()

                if (operator.is_supported(op_io)):
                    internal_op_list.append(operator)
                else:
                    print("Config I/O info not supported in Resize operator.")
                    sys.exit(-1)
            
            if (self.debug):
                print("Checking Normalize is required...")
            index = get_op_index(self.ops, op.Normalize())
            operator = None
            if ((-1 != index) or (check_normalize(self, self.ops))):
                if (-1 != index):
                    # Check user specified Normalize
                    operator = self.ops[index]
                else:
                    # Check if Normalize is required for FP16 conversion
                    if (self.format_out == drpai_param.FORMAT.GRAY):
                        operator = op.Normalize([0.0], [1.0])
                    else:
                        operator = op.Normalize([0.0, 0.0, 0.0], [1.0, 1.0, 1.0])

                op_io = update_op_io(op_io, internal_op_list, self)
                op_io.type_out = drpai_param.TYPE.FP16
                op_io.format_out = self.format_out
                if (self.debug):
                    op_io.print()
                if (operator.is_supported(op_io)):
                    internal_op_list.append(operator)
                else:
                    print("Config I/O info not supported in Normalize operator.")
                    sys.exit(-1)
            
            if (self.debug):
                print("Checking Transpose is required...")
            if (check_transpose(self)):
                operator = op.Transpose()
                op_io = update_op_io(op_io, internal_op_list, self)
                op_io.order_out = (op_io.order_in + 1) % 2
                if (self.debug):
                    op_io.print()

                if (operator.is_supported(op_io)):
                    internal_op_list.append(operator)
                else:
                    print("Config I/O info not supported.")
                    sys.exit(-1)

            if (self.debug):
                print("Checking Cast (fp16 to fp32) is required...")
            if (check_cast(self, 0, internal_op_list)):
                operator = op.Cast()
                op_io = update_op_io(op_io, internal_op_list, self)
                op_io.type_out = self.type_out
                if (self.debug):
                    op_io.print()
                if (operator.is_supported(op_io)):
                    internal_op_list.append(operator)
                else:
                    print("Config I/O info not supported.")
                    sys.exit(-1)

            # Check if there are any operators exist in internal_op_list.
            if (0 == len(internal_op_list)):
                print("[ERROR] No operator can be used.")
                print("        Please check your In/Out data and operators.")
                sys.exit(-1)

            # Check the final operator I/O information matches User input.
            if (not check_out_match(self, internal_op_list[-1].op_io)):
                print("        Please check your In/Out data and operators.")
                sys.exit(-1)

            # Get operator list in  DRP-AI Translator Python API
            for op_class in internal_op_list:
                op_api = op_class.get_api()
                pp_sequence = pp_sequence + op_api

            # Set pre processing information
            pp.set_input_to_pre(
                in_name,
                shape=shape_in,
                order=order_in_str,
                type=type_in_str,
                format=format_in_str 
            )
            pp.set_input_to_body(
                out_name,
                shape=shape_out,
                order=order_out_str,
                type=type_out_str,
                format=format_out_str 
            )

            # Set processing sequence
            pp.set_preprocess_sequence(
                src=[in_name],
                dest=[out_name],
                pp_seq=pp_sequence
            )

        # Show setting info. DEBUG mode only
        if (self.debug):
            pp.show_params()

        # Save data as yaml file
        pp.save(filename)

        print("File saved as ", filename)


class PreRuntime():
    """
    Pre-processing Runtime class.
    """
    def __init__(self, config, out_dir, product="V2L"):
        debug = 0
        if (out_dir == ""):
            print("[ERROR] No output directory specified.")
            sys.exit(-1)
        if (not isinstance(config, type(Config()))):
            print("[ERROR] First argument Config() is not initialized.")
            sys.exit(-1)
        print("[INFO] Product: "+product)
        self.config = config
        self.out_dir = out_dir
        self.product = product
        if (self.product == "V2MA"):
            self.product = "V2M"
        
        if (self.config.mode == drpai_param.MODE.POST):
            print("[INFO] Running in POST mode.")
        if (self.config.debug):
            debug = 1;
            self.config.print_ops()

        # Set absolute path
        cur_path = os.getcwd() # get current path

        # Necessary variables
        # dynamic_alloc_addr = 0x40000000
        dynamic_alloc_addr = 0x00000000
        addr_file = os.path.join(cur_path, "addr_map.yaml")
        pp_file = os.path.join(cur_path, "preprocess.yaml")
        dummy_file = ""
        prefix = "pp"
        translator_out_path = os.path.join(TRANSLATOR, 'output', prefix)
        pp_out_path = os.path.join(cur_path, self.out_dir)

        # Delete the DRP-AI Translator intermediate files from previous execution
        if os.path.isdir(pp_out_path):
            shutil.rmtree(pp_out_path)

        # Make prepost definition file.
        self.config.generateYAML(pp_file)
        
        # Initialize DRP-AI Translator class
        drpai_tran = drp_ai_translator()

        # Choose device of run script
        drpai_tran.set_translator(self.product)

        # Make address map file with dynamic allocatable address.
        drpai_tran.make_addrfile(dynamic_alloc_addr, addr_file)  

        # Run DRP-AI Translator
        drpai_tran.run_translate("pp", \
            onnx=dummy_file, \
            prepost=pp_file, \
            addr=addr_file, \
            options=["-PrePostOnly"]
        )

        # Move result to output directory specified in argument.
        # Generate only necessary files from DRP-AI Translator
        dir_list = [
            'aimac_desc.bin', 
            'drp_desc.bin', 
            'drp_param.bin', 
            prefix+'_drpcfg.mem', 
            prefix+'_weight.dat', 
            'drp_param_info.txt', 
            prefix+'_addrmap_intm.txt'
        ]
        # DEBUG: generate all intermediate files of DRP-AI Translator
        if (debug):
            dir_list = os.listdir(translator_out_path)

        # Check DRP-AI Translator passed or failed
        if (0 == len(dir_list) or (debug and 21 != len(dir_list))):
            print("[ERROR] DRP-AI Pre-processing Runtime failed.", flush=True)
            sys.exit(-1)
        for p in dir_list:
            path = os.path.join(translator_out_path, p)
            if (not os.path.isfile(path)):
                print("[ERROR] DRP-AI Pre-processing Runtime failed.", flush=True)
                sys.exit(-1)

        os.makedirs(pp_out_path, exist_ok=True)
        for p in dir_list:
            shutil.move(os.path.join(translator_out_path, p), os.path.join(pp_out_path, p))
        print("DRP-AI Pre-processing Runtime saved at: ", flush=True)
        print('./'+self.out_dir, flush=True)

