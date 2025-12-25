from ultralytics import YOLO
import argparse

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("modelpt", help=".pt file of the model")
    args = parser.parse_args()

   
    # Load a model
    model = YOLO(args.modelpt)    # .pt file
    # Export the model to ONNX format
    path = model.export(format="onnx", opset=13)


