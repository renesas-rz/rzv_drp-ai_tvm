
import onnxruntime as ort
import yaml
import sys

import onnx

def rename_onnx(in_path: str, 
                        out_path: str, 
                        mapping: dict[str, str]) -> None:
    
    model = onnx.load(in_path)

    # Node
    for node in model.graph.node:
        node.input[:]  = [mapping.get(n, n) for n in node.input]
        node.output[:] = [mapping.get(n, n) for n in node.output]

    # Input/Output/Value_info
    for vi in list(model.graph.input) + list(model.graph.output) + list(model.graph.value_info):
        if vi.name in mapping:
            vi.name = mapping[vi.name]
    # Initializer
    for init in model.graph.initializer:
        if init.name in mapping:
            init.name = mapping[init.name]
    for sp in model.graph.sparse_initializer:
        if sp.values.name in mapping:
            sp.values.name = mapping[sp.values.name]
        if sp.indices.name in mapping:
            sp.indices.name = mapping[sp.indices.name]

    onnx.checker.check_model(model)

    onnx.save(model, out_path)


def _gen_dict(org_model_path : str,
              tvm_model_path)->dict:
    
    # Check original onnx in/out node name
    in_list_org = list()
    out_list_org = list()
    sess = ort.InferenceSession(org_model_path, providers=["CPUExecutionProvider"])
    for i, inp in enumerate(sess.get_inputs()):
        #print(f"[{i}] name={inp.name}, dtype={inp.type}, shape={inp.shape}")
        in_list_org.append(inp.name)
    for i, out in enumerate(sess.get_outputs()):
        #print(f"[{i}] name={out.name}, dtype={out.type}, shape={out.shape}")
        out_list_org.append(out.name)

    # Check TVM onnx in/out node name
    in_list = list()
    out_list = list()
    sess = ort.InferenceSession(tvm_model_path, providers=["CPUExecutionProvider"])
    for i, inp in enumerate(sess.get_inputs()):
        #print(f"[{i}] name={inp.name}, dtype={inp.type}, shape={inp.shape}")
        in_list.append(inp.name)

    for i, out in enumerate(sess.get_outputs()):
        #print(f"[{i}] name={out.name}, dtype={out.type}, shape={out.shape}")
        out_list.append(out.name)

    parse_dict = dict()

    for idx in range(len(in_list)):
        parse_dict[in_list_org[idx]] = in_list[idx]
    for idx in range(len(out_list)):
        parse_dict[out_list_org[idx]] = out_list[idx]

    return parse_dict

if __name__ == "__main__":

    org_onnx_path = sys.argv[1]
    tvm_onnx_path = sys.argv[2]
    out_onnx_path = sys.argv[3]

    map_dict = _gen_dict(org_onnx_path, tvm_onnx_path)

    rename_onnx(org_onnx_path, out_onnx_path, map_dict)
    #print(f"[Save new onnx] : {out_onnx_path}")

