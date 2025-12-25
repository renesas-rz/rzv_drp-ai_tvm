from facenet_pytorch import InceptionResnetV1
import torch

model = InceptionResnetV1(pretrained='vggface2').eval()
dummy_din = torch.ones(1,3,160,160)
torch.onnx.export(model, dummy_din, \
                "InceptionResnetV1_vggface2.onnx", \
                input_names=["input"])

model = InceptionResnetV1(pretrained='casia-webface').eval()
dummy_din = torch.ones(1,3,160,160)
torch.onnx.export(model, dummy_din, \
                "InceptionResnetV1_casia-webface.onnx", \
                input_names=["input"])

model = InceptionResnetV1(num_classes=100).eval()
dummy_din = torch.ones(1,3,160,160)
torch.onnx.export(model, dummy_din, \
                "InceptionResnetV1_100.onnx", \
                input_names=["input"])

model = InceptionResnetV1(classify=True, num_classes=1001).eval()
dummy_din = torch.ones(1,3,160,160)
torch.onnx.export(model, dummy_din, \
                "InceptionResnetV1_1001.onnx", \
                input_names=["input"])
