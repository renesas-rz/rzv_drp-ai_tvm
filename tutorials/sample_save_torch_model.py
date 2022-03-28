#
#  Original code(C) Copyright EdgeCortix, Inc. 2022
#  Modified Portion (C) Copyright Renesas Electronics Corporation 2022
#

import numpy as np

# PyTorch imports
import torch
import torchvision

model_name = "resnet18"
model = getattr(torchvision.models, model_name)(pretrained=True)
model = model.eval()

# We grab the TorchScripted model via tracing
input_shape = [1, 3, 224, 224]
input_data = torch.randn(input_shape)
scripted_model = torch.jit.trace(model, input_data).eval()

scripted_model.save('resnet18.pt') # Save
print("Torch model saved to ./resnet18.pt")
