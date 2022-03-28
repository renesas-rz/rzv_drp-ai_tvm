#
# (C) Copyright EdgeCortix, Inc. 2021
#
import os
import onnx
import tvm
import sys
import numpy as np

from tvm import relay, runtime
from tvm.relay import mera
from tvm.contrib import graph_executor
from google.protobuf.json_format import MessageToDict
from optparse import OptionParser


# Input shape helper
# Some ONNX only contain symbolic shapes for non-batch dimensions
def get_standard_input_shape(network_name):
    standard_input_shape = {
        'mobilenetv2-7.onnx': [1, 3, 224, 224],
        'fcn-resnet50-11.onnx': [1, 3, 520, 520],
        'fcn-resnet101-11.onnx': [1, 3, 520, 520],
        'retinanet-9.onnx': [1, 3, 480, 640],
        'yolov4.onnx': [1, 416, 416, 3],
    }
    for k in standard_input_shape.keys():
        if k in network_name:
            return standard_input_shape[k]


# Input data helper
# Some models are not very stable when using random data
def get_standard_input_data(network_name):
    standard_input_shape = {
        'resnet18-v1-7': 'test_input_data/resnet18-v1-7/ref_inp_0.bin',
        'resnet50-v1-7': 'test_input_data/resnet50-v1-7/ref_inp_0.bin',
        'mobilenetv2-7.onnx': 'test_input_data/mobilenetv2-7/ref_inp_0.bin',
        'retinanet-9.onnx': 'test_input_data/retina-9/ref_inp_0.bin',
    }
    for k in standard_input_shape.keys():
        if k in network_name:
            return standard_input_shape[k]
    return None


def get_input_shape(network_filename, inp_msg):
    dim_info = MessageToDict(inp_msg).get("type").get("tensorType").get("shape").get("dim")
    input_shape = []

    is_symbolic = False
    for d in dim_info:
        if "dimParam" in d:
            is_symbolic = True
            break
    if is_symbolic:
        return get_standard_input_shape(network_filename)
    else:
        return [int(d.get("dimValue")) for d in dim_info]


if __name__ == "__main__":
    np.random.seed(1234567)
    usage = "usage: %prog [options] model_file"
    parser = OptionParser(usage)
    parser.set_defaults(drp_compiler_dir='./drp_compiler', toolchain_dir='/opt/poky/2.4.3', disable_concat = False)
    parser.add_option("-d", "--drp_compiler_dir", dest="drp_compiler_dir", help="DRP-AI Translator root directory", metavar="DIR")
    parser.add_option("-t", "--toolchain_dir", dest="drp_compiler_dir", help="Cross-compilation toolchain root directory", metavar="DIR")
    parser.add_option("-c", "--disable_concat", action="store_true", dest="disable_concat", default=False, help="Disable concat at DRP")
    (options, args) = parser.parse_args()
    opts = vars(options)
    if len(args) != 1:
        parser.error("Expected a model file")
    print(opts)

    model_file = str(args[0])
    print("Compiling ", model_file, ":")
    print("  Cross-compilation toolchain: ", opts["toolchain_dir"])
    print("  DRP-AI Translator: ", opts["drp_compiler_dir"])

    onnx_model = onnx.load_model(model_file)

    model_initializers = set()
    model_inputs = set()
    for init in onnx_model.graph.initializer:
        model_initializers.add(init.name)
    for inp in onnx_model.graph.input:
        model_inputs.add(inp.name)
    model_vars = []
    for inp in model_inputs:
        if inp not in model_initializers:
            model_vars.append(inp)

    output_dir="_out"
    os.makedirs(output_dir, exist_ok=True)
    shape_dict = {}
    input_data = []
    inp_idx = 0
    for inp in onnx_model.graph.input:
        input_name = inp.name
        if input_name not in model_vars:
            continue
        input_shape = get_input_shape(model_file, inp)
        shape_dict[input_name] = input_shape
        data_input_file = get_standard_input_data(model_file)
        if data_input_file is None:
            print("Using random input data...")
            data = np.random.uniform(-1.0, 1.0, input_shape).astype(np.float32)
        else:
            print("Using real input data...")
            data = np.fromfile(data_input_file, dtype=np.float32).reshape(input_shape)
        input_data.append(data)
        data.flatten().astype(np.float32).tofile(os.path.join(output_dir, "input_" + str(inp_idx) + ".bin"))
        inp_idx += 1

    mod, params = relay.frontend.from_onnx(onnx_model, shape_dict)

    # tvm normal
    target = tvm.target.Target("llvm", host="llvm")
    dev = tvm.cpu(0)
    with tvm.transform.PassContext(opt_level=3):
        lib = relay.build(mod, target=target, params=params)

    m = graph_executor.GraphModule(lib["default"](dev))
    for inp_idx in range(len(input_data)):
        m.set_input(inp_idx, input_data[inp_idx])
    m.run()

    # tvm byoc
    drp_config = {
        "interpreter": True,
        "addr_map_start": 0x5f800000,
        "toolchain_dir": opts["drp_compiler_dir"],
    }
    json, params, lib_path = mera.drp.build(mod, params, "x86", drp_config, output_dir="_out", disable_concat = opts["disable_concat"])
    lib = runtime.load_module(lib_path)
    ctx = runtime.cpu()
    rt_mod = graph_executor.create(json, lib, ctx)
    rt_mod.set_input(**params)

    for inp_idx in range(len(input_data)):
        rt_mod.set_input(inp_idx, input_data[inp_idx])
    rt_mod.run()

    # save to use as reference
    for i in range(rt_mod.get_num_outputs()):
        byoc_output = rt_mod.get_output(i).asnumpy()
        if byoc_output.dtype == "float32":
            byoc_output.flatten().astype(np.float32).tofile(
                os.path.join(output_dir, "ref_result_" + str(i) + ".bin"))
        elif byoc_output.dtype == "float16":
            byoc_output.flatten().astype(np.float16).tofile(
                os.path.join(output_dir, "ref_result_" + str(i) + ".bin"))

    for i in range(rt_mod.get_num_outputs()):
        byoc_output = rt_mod.get_output(i).asnumpy()
        tvm_output = m.get_output(i).asnumpy()
        np.testing.assert_allclose(byoc_output, tvm_output, atol=1e-1, rtol=1e-1)
        print("Non-zero values: ", (np.count_nonzero(byoc_output) / byoc_output.size) * 100.0, "%")

    drp_config_runtime = {
        "interpreter": False,
        "addr_map_start": 0x5f800000,
        "toolchain_dir": opts["drp_compiler_dir"],
        "sdk_root": opts["toolchain_dir"]
    }
    json, params, lib_path = mera.drp.build(mod, params, "arm", drp_config_runtime, output_dir="_out", disable_concat = opts["disable_concat"])
