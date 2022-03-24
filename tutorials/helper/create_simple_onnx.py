import torch
import torch.nn as nn
import torch.nn.functional as F


class Net(nn.Module):
    def __init__(self):
        super(Net, self).__init__()
        self.conv1 = nn.Conv2d(3, 32, 3, 1)
        self.conv2 = nn.Conv2d(32, 16, 3, 1)
        self.conv3 = nn.Conv2d(16, 8, 3, 1)
        self.fc1 = nn.Linear(23328, 8)
        self.fc2 = nn.Linear(8, 1000)

    def forward(self, x):
        x = self.conv1(x)
        x = F.relu(x)
        x = self.conv2(x)
        x = F.relu(x)
        x = F.max_pool2d(x, 2)
        x = self.conv3(x)
        x = F.relu(x)
        x = F.max_pool2d(x, 2)
        x = torch.flatten(x, 1)
        x = self.fc1(x)
        x = F.relu(x)
        output = self.fc2(x)
        return output

    def reset_weight_and_bias(self):
        self.conv1.weight.fill_(1.0)
        self.conv1.bias.fill_(1.0)
        self.conv2.weight.fill_(1.0)
        self.conv2.bias.fill_(1.0)
        self.conv3.weight.fill_(1.0)
        self.conv3.bias.fill_(1.0)


def main():
    model = Net().to("cpu")
    x = torch.ones((1, 3, 224, 224), dtype=torch.float32)
    with torch.no_grad():
        model.reset_weight_and_bias()
        torch.onnx.export(model, x, "small_01.onnx",
            export_params=True,
            opset_version=12,
            do_constant_folding=False,
            input_names = ['data'],
            output_names = ['output']
        )

if __name__ == '__main__':
    main()

