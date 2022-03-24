#
# (C) Copyright EdgeCortix, Inc. 2021
#

import os
import onnx
import numpy as np

import tvm
from tvm.relay import transform
from tvm.relay.param_dict import save_param_dict
from tvm.relay.build_module import bind_params_by_name
from tvm.relay.build_module import build as _build
from tvm.ir.transform import Sequential, PassContext
from tvm.contrib import cc
from optparse import OptionParser


if __name__ == "__main__":
    usage = "usage: %prog [options] model_file"
    parser = OptionParser(usage)
    parser.set_defaults(toolchain_dir='/opt/poky/2.4.3', output_dir="out_arm_only", input_name="data", fp16=False, input_shape=None)
    parser.add_option("-t", "--toolchain_dir", dest="toolchain_dir", help="Cross-compilation toolchain root directory", metavar="DIR")
    parser.add_option("-o", "--output_dir", dest="output_dir", help="Output directory", metavar="DIR")
    parser.add_option("-i", "--input_name", dest="input_name", help="Input name")
    parser.add_option("-s", "--input_shape", dest="input_shape", help="Input shape")
    parser.add_option("-q", "--fp16", action="store_true", dest="fp16", help="Convert to FP16")
    (options, args) = parser.parse_args()
    opts = vars(options)
    if len(args) != 1:
        parser.error("Expected an ONNX model file")
    print(opts)

    model_file = str(args[0])
    print("Compiling ", model_file, ":")
    print("  Cross-compilation toolchain: ", opts["toolchain_dir"])
    input_shape = opts["input_shape"]
    if input_shape is not None:
        input_shape = tuple([int(dim) for dim in input_shape.split(',')])
        print("  User specified input shape: ", input_shape)
    else:
        input_shape = [1, 3, 224, 224]
        print("  Using default input shape: ", input_shape)

    input_name = opts["input_name"]
    output_dir = opts["output_dir"]
    sdk_root = opts["toolchain_dir"]
    
    host_arch = "arm"
    target = "llvm -device=arm_cpu -mtriple=aarch64-linux-gnu -mattr=+neon"
    fcompile = cc.cross_compiler(
        "{sdk_root}/sysroots/x86_64-pokysdk-linux/usr/bin/aarch64-poky-linux/aarch64-poky-linux-g++".format(sdk_root = sdk_root),
        [
            "--sysroot={sdk_root}/sysroots/aarch64-poky-linux".format(sdk_root = sdk_root),
            "-march=armv8-a",
            "-mtune=cortex-a53"
        ])

    shape_dict = {input_name: input_shape}
    onnx_model = onnx.load_model(model_file)
    mod, params = tvm.relay.frontend.from_onnx(onnx_model, shape_dict)
    if params:
        mod["main"] = bind_params_by_name(mod["main"], params)
    
    with PassContext(opt_level=3):
        pass_list = [
            transform.SimplifyInference(),
            transform.FoldConstant(),
            transform.FoldExplicitPadding(),
            transform.BackwardFoldScaleAxis(),
            transform.ForwardFoldScaleAxis(),
            transform.FoldConstant(),
            transform.DynamicToStatic(),
        ]
        if opts["fp16"]:
            pass_list += [
                transform.ToMixedPrecision("float16"),
                transform.FoldConstant(),
            ]
        pass_list += [
            transform.RemoveUnusedFunctions(),
        ]

    simplify = Sequential(pass_list)
    mod = simplify(mod)
    
    with PassContext(opt_level=3):
        json, lib, all_params = _build(mod, target=target, target_host=target, params=params)
    
    os.makedirs(output_dir, exist_ok=True)
    lib_path = os.path.join(output_dir, "deploy.so")
    lib.export_library(lib_path, fcompile=fcompile)
    with open(os.path.join(output_dir, "deploy.json"), "w") as f:
        f.write(json)
    with open(os.path.join(output_dir, "deploy.params"), "wb") as f:
        f.write(save_param_dict(all_params))

