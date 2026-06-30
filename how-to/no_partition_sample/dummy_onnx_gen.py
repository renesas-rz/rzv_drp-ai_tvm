
import onnxruntime as ort
import onnx
import numpy as np
from onnx import helper, TensorProto, numpy_helper, shape_inference

import sys

def gen_resize(i_name: str,
               o_name: str,
               shape_name : str,
               roi_name : str,
               scales_name : str,
               resize_mode = "nearest"):
    """
    Generate resize node
    """
    resize = helper.make_node(
        "Resize",
        inputs=[i_name, roi_name, scales_name, shape_name],
        outputs=[o_name],
        mode=resize_mode,
        coordinate_transformation_mode="asymmetric",
        cubic_coeff_a=-0.75,
        nearest_mode="floor"

    )
    return resize

def build_model(
    model_path: str = "dummy.onnx",
    opset: int = 13,
    seed: int = 42,
    resize_mode: str = "linear",
    i_shape_list = [[1, 3, 640, 640]],
    o_shape_list = [[1,80, 160,160]]
):
    """
    Generate dummy onnx graph
    """

    nodes_all = list()   # All node list
    init_all = list()    # All initializer list
    output_list = list() # output list

    # Define input & output nodes
    if(len(i_shape_list)>1):
        print("[ERROR] Multi input model is not supported")
        sys.exit(-1)
    # Check output shape
    for _o_shape in o_shape_list:
        if(len(_o_shape)!=4):
            print("[ERROR] Currently, only 4‑dimensional output shapes(NCHW) are supported.")
            sys.exit(-1)
    input_vi = helper.make_tensor_value_info(f"input", TensorProto.FLOAT, i_shape_list[0])
    for idx, o_shape in enumerate(o_shape_list):
        output_list.append(helper.make_tensor_value_info(f"output_{idx}", TensorProto.FLOAT, o_shape_list[idx]))

    # A) Define 1st conv layer
    rng = np.random.default_rng(seed)
    ch = i_shape_list[0][1]
    och0 = 16
    W = rng.standard_normal((och0, ch, 1, 1), dtype=np.float32) * 0.01  
    B = np.zeros((och0,), dtype=np.float32)
    W_init = numpy_helper.from_array(W, name=f"conv_w0")
    B_init = numpy_helper.from_array(B, name=f"conv_b0")
    conv = helper.make_node(
        "Conv",
        inputs=[f"input", f"conv_w0", f"conv_b0"],
        outputs=[f"x1"],
        kernel_shape=[1, 1],
        strides=[1, 1],
        pads=[0, 0, 0, 0],
        dilations=[1, 1],
        group=1
    )
    init_all.extend([W_init, B_init])
    nodes_all.append(conv)

    for idx in range(len(o_shape_list)):
        # B) Define 2nd Resize layer
        _shape = [1, och0, o_shape_list[idx][2], o_shape_list[idx][3]]
        #print(f"  -- Target intm shape: {_shape}")
        sizes_arr = np.array(_shape, dtype=np.int64)
        re_shape_name = f"resize_sizes_{idx}"
        sizes_init = numpy_helper.from_array(sizes_arr, name=re_shape_name)
        
        roi_arr = np.array([], dtype=np.float32)
        roi_name = f"roi_sizes_{idx}"
        roi_init = numpy_helper.from_array(roi_arr, name=roi_name)
        scales_arr = np.array([], dtype=np.float32)
        scales_name = f"scales_sizes_{idx}"
        scales_init = numpy_helper.from_array(scales_arr, name=scales_name)
        resize = gen_resize("x1", f"x2_{idx}", re_shape_name, roi_name, scales_name, resize_mode)

        init_all.extend([sizes_init, roi_init,scales_init])
        nodes_all.append(resize)

        # C) Define last Conv 1x1 layer
        rng = np.random.default_rng(seed)
        och = o_shape_list[idx][1]
        W = rng.standard_normal((och, och0, 1, 1), dtype=np.float32) * 0.01  # 小さめ初期値
        B = np.zeros((och,), dtype=np.float32)

        W_init = numpy_helper.from_array(W, name=f"conv_w_{idx}")
        B_init = numpy_helper.from_array(B, name=f"conv_b_{idx}")

        init_all.extend([W_init, B_init])

        conv = helper.make_node(
            "Conv",
            inputs=[f"x2_{idx}", f"conv_w_{idx}", f"conv_b_{idx}"],
            outputs=[f"output_{idx}"],
            kernel_shape=[1, 1],
            strides=[1, 1],
            pads=[0, 0, 0, 0],
            dilations=[1, 1],
            group=1
        )

        nodes_all.append(conv)

    # Make a graph
    graph = helper.make_graph(
        nodes=nodes_all, # Set node list
        name="dummy_onnx_model",
        inputs=[input_vi],
        outputs=output_list,
        initializer=init_all
    )
    # Make a model
    model = helper.make_model(
        graph,
        opset_imports=[helper.make_opsetid("", opset)],
        producer_name="custom-onnx-api-script",
        producer_version="1.0"
    )

    # Validation and make onnx file
    onnx.checker.check_model(model)
    model_inferred = shape_inference.infer_shapes(model)
    onnx.save(model_inferred, model_path)
    print(f"Saved ONNX model to: {model_path}")

def dummy_onnx_gen(model_path, out_path):
    #print(f"Input onnx : {model_path}")
    in_list = list()
    out_list = list()
    sess = ort.InferenceSession(model_path, providers=["CPUExecutionProvider"])
    for i, inp in enumerate(sess.get_inputs()):
        #print(f"[{i}] name={inp.name}, dtype={inp.type}, shape={inp.shape}")
        in_list.append(inp.shape)
    #print(in_list)

    #print("\n== Outputs ==")
    for i, out in enumerate(sess.get_outputs()):
        #print(f"[{i}] name={out.name}, dtype={out.type}, shape={out.shape}")
        out_list.append(out.shape)
    #print(out_list)

    build_model(
        model_path=out_path,
        opset=13,
        seed=42,
        resize_mode="nearest",  # linear
        i_shape_list = in_list,
        o_shape_list = out_list
    )

if __name__ == "__main__":
    model_path = sys.argv[1]
    print(f"Input onnx : {model_path}")
    in_list = list()
    out_list = list()
    sess = ort.InferenceSession(model_path, providers=["CPUExecutionProvider"])
    for i, inp in enumerate(sess.get_inputs()):
        print(f"[{i}] name={inp.name}, dtype={inp.type}, shape={inp.shape}")
        in_list.append(inp.shape)
    print(in_list)

    print("\n== Outputs ==")
    for i, out in enumerate(sess.get_outputs()):
        print(f"[{i}] name={out.name}, dtype={out.type}, shape={out.shape}")
        out_list.append(out.shape)
    print(out_list)

    build_model(
        model_path="dummy_model_for_tvm.onnx",
        opset=13,
        seed=42,
        resize_mode="nearest",  # linear
        i_shape_list = in_list,
        o_shape_list = out_list
    )
