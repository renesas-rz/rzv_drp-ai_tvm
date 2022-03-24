#
# (C) Copyright EdgeCortix, Inc. 2021
#

import sys
import numpy as np
import onnx, onnxruntime
import onnxruntime as rt

import tvm
from tvm import relay
from tvm.relay.build_module import bind_params_by_name
from tvm.relay.transform import ToMixedPrecision
from tvm.relay.op.contrib.register import get_pattern_table
from tvm.relay.mera.drp import prune_no_mac_subgraphs


def get_relay_ir_drp_float16(mod):
    pass_list = tvm.transform.Sequential([
                    relay.transform.SimplifyInference(simplify_batchnorm=False),
                    relay.transform.FoldConstant(),
                    relay.transform.RemoveUselessPadding(),
                    relay.transform.FoldExplicitPadding(),
                    relay.transform.BackwardFoldScaleAxis(),
                    relay.transform.ForwardFoldScaleAxis(),
                    relay.transform.FoldConstant(),
                    relay.transform.DynamicToStatic(),
                    relay.transform.FoldMulAddToBN(),
                    relay.transform.FoldConstant(),
                    ToMixedPrecision("float16"),
                    relay.transform.FoldConstant(),  # remove cast operators
                    relay.transform.MergeComposite(get_pattern_table("mera_drp_no_concat")),
                    relay.transform.MergeComposite(get_pattern_table("mera_drp_only_concat")),
                    relay.transform.AnnotateTarget("mera_drp"),
                    relay.transform.MergeCompilerRegions(),
                    relay.transform.PartitionGraph(),
                    relay.transform.SimplifyInference(simplify_batchnorm=True),
                    relay.transform.FoldConstant(),
                ])
    mod = pass_list(mod)
    mod = prune_no_mac_subgraphs(mod)
    relay_ir = mod["main"].astext(show_meta_data=False)
    return str(relay_ir)


def test_remove_empty_subgraphs():
    def case1():
        data = relay.var("data", relay.ty.TensorType((1,3,32,32), "float32"))
        out = relay.clip(data, a_min=0, a_max=1)
        func = relay.Function([data], out)
        mod = tvm.IRModule()
        mod['main'] = func
        return get_relay_ir_drp_float16(mod)

    assert "@tvmgen_default_mera_drp_" not in case1()


def test_model(model_path):
    def _test_model(model_file):
        onnx_model = onnx.load_model(model_file)
        session = onnxruntime.InferenceSession(model_file, None)
        shape_dict = {}
        input_details = session.get_inputs()
        for x in input_details:
            name = x.name
            shape = x.shape
            if x.shape[0] == 'batch_size' or x.shape[0] == 'None':
                shape = [1, x.shape[1], x.shape[2], x.shape[3]]
            shape_dict[name] = shape
        # print(shape_dict)
        mod, params = relay.frontend.from_onnx(onnx_model, shape_dict)
        # print(mod["main"].astext(show_meta_data=False))
        mod["main"] = bind_params_by_name(mod["main"], params)
        ir = get_relay_ir_drp_float16(mod)
        print(ir)

    _test_model(model_path+"classification/resnet/model/resnet18-v1-7.onnx")
    _test_model(model_path+"classification/resnet/model/resnet34-v1-7.onnx")
    _test_model(model_path+"classification/resnet/model/resnet50-v1-7.onnx")
    _test_model(model_path+"classification/resnet/model/resnet101-v1-7.onnx")
    _test_model(model_path+"classification/resnet/model/resnet18-v2-7.onnx")
    _test_model(model_path+"classification/mobilenet/model/mobilenetv2-7.onnx")
    _test_model(model_path+"classification/shufflenet/model/shufflenet-9.onnx")
    _test_model(model_path+"classification/inception_and_googlenet/inception_v2/model/inception-v2-7.onnx")
    _test_model(model_path+"object_detection_segmentation/retinanet/model/retinanet-9.onnx")
    _test_model(model_path+"object_detection_segmentation/yolov2-coco/model/yolov2-coco-9.onnx")
    _test_model(model_path+"object_detection_segmentation/tiny-yolov2/model/tinyyolov2-7.onnx")

    _test_model(model_path+"yolov2_416x416.onnx")


if __name__ == "__main__":
    test_remove_empty_subgraphs()

    # model_path = str(sys.argv[1])
    # test_model(model_path)
