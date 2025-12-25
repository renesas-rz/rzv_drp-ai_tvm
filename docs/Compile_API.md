
# Compile Reference

Note: EdgeCortix MERA is the base framework for DRP-AI TVM powered by EdgeCortix. DRP-AI TVM[^1] v2.7.0 and later supports MERA2 and has added several features. For more information, please refer About [MERA](./About_mera.md).

## Frontend APIs
### Functions list  

|API |Note|
|---|---|
| [mera2_.from_onnx()](#mera2_from_onnx) | Import an ONNX model file.|   
| [mera2_.from_pytorch()](#mera2_from_pytorch) | Import an PyTorch model file.|   
| [mera2_.from_tflite()](#mera2_from_tflite) | Import a TensorFlow Lite (TFLite) model file.|   
| [mera2_.from_exir()](#mera2_from_exir) | Import an EXIR model file.|   
| [mera2_.from_onnx_qat()](#mera2_from_onnx_qat) | Import a Quantization-Aware Trained (QAT) ONNX model.|   

### mera2_.from_onnx()
This function loads an ONNX model and converts it into an internal format compatible with Mera’s compilation flow. It supports both quantized (V2H, V2N) and non-quantized (V2M, V2L) targets with optional control over CPU
precision.

#### Sample code:   
```py
import mera2_ as mera2
mod, params = mera2.from_onnx(
    model_file, 
    shape_dict, 
    is_quant=True, 
    cpu_data_type="float16",
    use_mera2=True
    )
```

#### Arguments and Returns:   

| Index | Argument       | Note                                              |
|:-:|--------------------|---------------------------------------------------|
| 1 | onnx_file                | Path to the ONNX model file to be imported.     |
| 2 | shape_dict             | Dictionary mapping input tensor names to their shapes.<br> Example: {"input_0": [1, 3, 224, 224]}.        |
| 3 | is_quant              | If True, import the model for quantized targets (V2H, V2N). If False, import for non-quantized targets (V2M, V2L). Defaults to True        |
| 4 | cpu_data_type | Data type used for model segments that run on the CPU. <br>Supported values: "float32" or "float16". Defaults to "float32".     |
| 5 | use_mera2         | If True, use the Mera2 importer. If False, fall back to the legacy Mera1 importer. Defaults to True.         |

| Index |Return       | Note                                              |
|:-:|--------------------|---------------------------------------------------|
| 1 | model               | The imported model object ready for compilation.     |
| 2 | params             | model parameters extracted from the ONNX file.        |

### mera2_.from_pytorch()
This function loads a PyTorch model and converts it into an internal format compatible with the Mera framework.   
Note: Mera2 currently does not support PyTorch directly — this API always falls back to the legacy Mera1
importer.   

#### Sample code:   

```py
import mera2_ as mera2
mod, params = mera2.from_pytorch(
    model_file, 
    shape_dict, 
    )
```   

#### Arguments and Returns:   

| Index | Argument       | Note                                              |
|:-:|--------------------|---------------------------------------------------|
| 1 | pytorch_file                | Path to the PyTorch model file to be imported.     |
| 2 | shape_list             | List of input shapes corresponding to the model inputs. <br>Example: [[1, 3, 224, 224]].        |


| Index |Return       | Note                                              |
|:-:|--------------------|---------------------------------------------------|
| 1 | model               | The imported model object ready for compilation.     |
| 2 | params             | model parameters extracted from the ONNX file.        |

### mera2_.from_tflite()
This function loads a TensorFlow Lite model and converts it into an internal format compatible with Mera’scompilation. It supports both quantized (V2H, V2N) and non-quantized (V2M, V2L) targets, with optional control
over CPU precision and Mera version selection.   

#### Sample code:   

```py
import mera2_ as mera2
mod, params = mera2.from_tflite(
    model_file, 
    is_quant=True, 
    cpu_data_type="float16",
    use_mera2=True)
```   

#### Arguments and Returns:   

| Index | Argument       | Note                                              |
|:-:|--------------------|---------------------------------------------------|
| 1 | tflite_file                | Path to the tflite model file to be imported.     |
| 2 | is_quant              | If True, import the model for quantized targets (V2H, V2N). If False, import for non-quantized targets (V2M, V2L). Defaults to True        |
| 3 | cpu_data_type | Data type used for model segments that run on the CPU. <br>Supported values: "float32" or "float16". Defaults to "float32".     |
| 4 | use_mera2         | If True, use the Mera2 importer. If False, fall back to the legacy Mera1 importer. Defaults to True.         |

| Index |Return       | Note                                              |
|:-:|--------------------|---------------------------------------------------|
| 1 | model               | The imported model object ready for compilation.     |
| 2 | params             | model parameters extracted from the ONNX file.        |

### mera2_.from_exir()
This function loads a pre-converted EXIR model (an intermediate representation format) and prepares it for
deployment on supported targets. It supports both quantized (V2H) and non-quantized (V2M, V2L) models, with configurable CPU precision.   

#### Sample code:   

```py
import mera2_ as mera2
mod, params = mera2.from_exir(
    model_file, 
    shape_dict, 
    is_quant=True, 
    cpu_data_type="float16",
    use_mera2=True
    )
```

#### Arguments and Returns:   

| Index | Argument       | Note                                              |
|:-:|--------------------|---------------------------------------------------|
| 1 | exir_file                | Path to the EXIR model file to be imported.     |
| 2 | shape_dict             | Dictionary mapping input tensor names to their shapes.<br> Example: {"input_0": [1, 3, 224, 224]}.        |
| 3 | is_quant              | If True, import the model for quantized targets (V2H, V2N). If False, import for non-quantized targets (V2M, V2L). Defaults to True        |
| 4 | cpu_data_type | Data type used for model segments that run on the CPU. <br>Supported values: "float32" or "float16". Defaults to "float32".     |


| Index |Return       | Note                                              |
|:-:|--------------------|---------------------------------------------------|
| 1 | model               | The imported model object ready for compilation.     |
| 2 | params             | model parameters extracted from the EXIR file.        |

### mera2_.from_onnx_qat
This function behaves similarly to from_onnx(), but is specifically designed for importing ONNX models that were generated using Quantization-Aware Training (QAT). It prepares the model for deployment on V2H targets, supporting DRP-AI quantization tools.   

#### Sample code:   

```py
import mera2_ as mera2
qat_type = mera2.QatType.from_str("PyTorch") # or "TensorFlow"
mod, params = mera2.from_onnx_qat(
    model_file, 
    shape_dict, 
    qat_type, 
    opts["quantization_tool"],  
    cpu_data_type,
    use_mera2=not opts["onnx_use_mera1"])
```

#### Arguments and Returns:   

| Index | Argument       | Note                                              |
|:-:|--------------------|---------------------------------------------------|
| 1 | onnx_file                | Path to the ONNX model file to be imported.     |
| 2 | shape_dict             | Dictionary mapping input tensor names to their shapes.<br> Example: {"input_0": [1, 3, 224, 224]}.        |
| 3 | qat_type              | Type of QAT model. "pytorch" or "tensorflow".        |
| 4 | drpai_quant_tool              | Path to the DRP-AI quantization tool directory used for QAT processing.        |
| 5 | cpu_data_type | Data type used for model segments that run on the CPU. <br>Supported values: "float32" or "float16". Defaults to "float32".     |
| 6 | use_mera2         | If True, use the Mera2 importer. If False, fall back to the legacy Mera1 importer. Defaults to True.         |

| Index |Return       | Note                                              |
|:-:|--------------------|---------------------------------------------------|
| 1 | model               | The imported model object ready for compilation.     |
| 2 | params             | model parameters extracted from the ONNX file.        |

## Backend APIs
### Functions list  

|API |Note|
|---|---|
| [mera2_.drp.build()](#mera2_drpbuild) | Compile and build a model for deployment on the specified target.|

### mera2_.drp.build()   
This function takes a model imported via from_onnx, from_exir, or from_tflite, along with its parameters,
and generates the runtime deployment files for the target architecture. The generated outputs include a
runtime configuration JSON, model parameters, and compiled libraries.   

#### Sample code:   

```py
import mera2_ as mera2
json, params, lib_path = mera2.drp.build(
    mod, 
    params, 
    "arm", 
    drp_config, 
    output_dir, 
    disable_concat=False, 
    cpu_data_type="float16"
    )
```

#### Arguments and Returns:   

| Index | Argument       | Note                                              |
|:-:|--------------------|---------------------------------------------------|
| 1 | mod                | Imported model object returned by from_onnx, from_exir, or from_tflite.     |
| 2 | params             | Model parameters returned by from_onnx, from_exir, or from_tflite        |
| 3 | host_arch              | Host architecture on which the compilation and deployment process will run.<br>Examples: x86, arm.        |
| 4 | drpai_config              | Dictionary containing the compilation configuration. <br>See [drp_config](#drpai_config) for the detail      |
| 5 | output_dir | Directory where the compiled deployment outputs will be stored. This includes runtime JSON, parameter files, and libraries     |   
| 6 | disable_concat | If True, the Concat operation will be forced to run on CPU instead of DRP-AI hardware. Applicable to V2M/L targets only. Defaults to False.      |   
| 7 | cpu_data_type | Data type for CPU-executed parts of the model. <br>Supported values: "float32" or "float16". <br>Defaults to "float32".      |   


| Index |Return       | Note                                              |
|:-:|--------------------|---------------------------------------------------|
| 1 | runtime_config               | JSON object containing runtime deployment configuration.     |
| 2 | params             | Model parameters used for deployment.        |
| 3 | lib_path             | Path to the compiled library files for the target.        |

#### drpai_config   

- target(str)   
   + "DrpAiQuant": Compile the model for quantized targets (V2H or V2N).   
   + "DrpAi": Compile the model for non-quantized targets (V2M or V2L).   
   + "Fp32DataRecorder": Generate calibration data for quantization.   
   + "InterpreterQuant": Interpreter mode for quantized models.   
   + "Interpreter": Interpreter mode for non-quantized models.   
- addr_map_start (int): Start address of the DRPAI memory area.   
- toolchain_dir (str): Directory of the DRPAI compilation toolchain.   
- drp_compiler_version (str): DRPAI compiler version. Use "100" for the latest version. 
- sdk_root (str): Root directory of the SDK/toolchain used for CPU segment generation.   
- quantization_tool (str): Directory containing the quantization tool.   
- quantization_option (str): Options for quantization tool.   
- calibration_data (str): Directory containing calibration data used for quantization.   

Example 1: Config for V2H
```py
drp_config = {
    "target": "DrpAiQuant",
    "addr_map_start": 0x00,
    "toolchain_dir": opts["drp_compiler_dir"],
    "drp_compiler_version": opts["drp_compiler_version"],
    "sdk_root": opts["toolchain_dir"],
    "quantization_tool": opts["quantization_tool"],
    "quantization_option": opts["quantization_option"],
    "calibration_data": opts["record_dir"]
}
```

Example 2: Config for V2M
```py
drp_config_runtime = {
    "target": "DrpAi",
    "addr_map_start": 0x0,
    "toolchain_dir": opts["drp_compiler_dir"],
    "sdk_root": opts["toolchain_dir"]
}
```

Example 3: Config for generating calibration data for V2H
```py
drp_config = {
    "target": "Fp32DataRecorder",
    "drp_compiler_version": opts["drp_compiler_version"],
    "record_dir": opts["record_dir"],
}
```

Example 4: Config for Interpreter mode of V2H
```py
drp_config = {
    "target": "InterpreterQuant",
    "drp_compiler_version": opts["drp_compiler_version"],
    "quantization_tool": opts["quantization_tool"],
    "quantization_option": opts["quantization_option"],
    "calibration_data": opts["record_dir"]
}
```

Example 5: Config for Interpreter mode of V2M
```py
drp_config = {
    "target": "Interpreter"
}
```

[^1]: DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.   