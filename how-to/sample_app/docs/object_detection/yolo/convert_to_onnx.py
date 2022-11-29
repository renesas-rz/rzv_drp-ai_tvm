import torch.onnx
import sys
import yolo
from read_ini import IniFile

if __name__ == "__main__":
    # Load YOLO neural network parameters
    ini = IniFile("yolo.ini")
    model_dict = ini.model_dict

    # Obtain the model name from command line argument
    if ( len(sys.argv) > 1 and sys.argv[1] in model_dict.keys() ):
        model_name = sys.argv[1]
        print("Model [", model_name, "] is selected.")
    else:
        print("Default model [ yolov3 ] is selected.")
        model_name = "yolov3"
    # Get the model information
    model_params = ini.architecture[model_dict[model_name]].params

    # Initialise one random value filled input tensor of an image size 3x416x416 (CHW)
    dummy_input = torch.randn(1, 3, 416, 416)

    # Loads from YOLO neural network structure
    model = yolo.Yolo(model_params.get('cfg'))
    model.load_state_dict(torch.load(model_params.get('pth')))

    # set the model to inference mode
    model.eval()
    # Define the input tensor and output tensor name of the converted onnx neural network
    input_names=model_params.get('input')
    output_names=model_params.get('output')

    # Starts the pythorch to onnx conversion
    torch.onnx.export(model, dummy_input, model_params.get('onnx'), verbose=True, opset_version=12, input_names=input_names, output_names=output_names)
