
# Reference URL:
# https://pytorch.org/hub/intelisl_midas_v2/

import torch
import onnx
from onnxsim import simplify

# donwload model from torch hub
model_type = "MiDaS_small"  # MiDaS v2.1 - Small  
midas = torch.hub.load("intel-isl/MiDaS", model_type)
device =torch.device("cpu")
midas.to(device)
midas.eval()

# Define dummy input shape
ICH=3
DIN_H=256
DIN_W=256
tsr_din = torch.ones(1, ICH, DIN_H, DIN_W)

# Export to onnx model
model_name = "./midas_small_256x256.onnx"
torch.onnx.export(midas, tsr_din, model_name, input_names=["input"])

# simplified the onnx graph
model_simple =  "./midas_small_256x256_simple.onnx"
model_onnx= onnx.load(model_name)
model_simp, check = simplify(model_onnx)
onnx.save(model_simp, model_simple)
# Use above model as DRP-AI TVM input data