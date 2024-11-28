
# How to use MERA Runtime Wrapper

## List of functions

| Function name                                    | Note                                                        |
| ----------------------------------------- | ----------------------------------------------------------- |
| [GetInputDataType](#getinputdatatype) | Return input data type                                    |
| [GetNumOutput](#getnumoutput)         | Get the number of outputs from the graph.                   |
| [GetOutput](#getoutput)               | Returns inference result                                        |
| [LoadModel](#loadmodel)               | Deploy the DRP-AI TV[^1] model       |
| [ProfileRun](#profilerun)             | Run AI model graph with profile                                  |
| [Run](#run)                           |Run AI model graph                                                   |
| [SetInput](#setinput)                 | Copy data to the input layer                                |

----
## GetInputDataType
 - Return input data type
### Format
```c++
#include "MeraDrpRuntimeWrapper.h"

InOutDataType MeraDrpRuntimeWrapper::GetInputDataType(int index);
```
### Note
 - **GetInputDataType()** method returns the data type of the input layer specified by the argument index.
### Return   
 - Output data type
   - FLOAT32 : float 32bit
   - FLOAT16 : flaot 16bit 
   - OTHER : Not supported type by DRP-AI TVM[^1]


----
## GetNumOutput
 - Get the number of outputs from the graph.

### Format
```c++
#include "MeraDrpRuntimeWrapper.h"

int MeraDrpRuntimeWrapper::GetNumOutput(void);
```
### Note
 - **GetNumOutput()** method returns the number of output layers of the inference model deployed to the MeraDrpRuntimeWrapper object.

### Return
 - The number of output nodes. A non-zero positive integer. A value of 0 or negative indicates that a problem occurred during the processing of this function.

----
## GetOutput
 - Returns inference result.

### Format
```c++
#include "MeraDrpRuntimeWrapper.h"

std::tuple<InOutDataType, void*, int64_t> MeraDrpRuntimeWrapper::GetOutput(int index);
```
### Note
 - **GetOutput()**  method returns the inference result as a tuple type object. Also, specify the index of the output layer as an argument.

### Return   
 - Inference results. The data type depends on the AI model graph.
   - FLOAT32 : float 32bit
   - FLOAT16 : flaot 16bit 
   - OTHER : Not supported type by DRP-AI TVM[^1]

----
## LoadModel
 - Deploy the DRP-AI TVM[^1] model specified in the MeraDrpRuntimeWrapper class object.

### Format
```c++
#include "MeraDrpRuntimeWrapper.h"

bool MeraDrpRuntimeWrapper::LoadModel(const std::string& model_dir, 
                                      uint32_t start_address);
```
### Note

- Expand the runtime model data into the memory for the DRP-AI device. The directory where inference models and their weights are stored is specified by the argument `model_dir`. The memory address for the DRP-AI device is specified by the argument `start_address`.

### Return

- Except v1.1.1 and earlier, If a CPU-only model is loaded , false is returned.
  - true : Success. There were no problems.
  - false : CPU-only model was specified. Therefore, device_type must be set to kDLCPU.

   If you will load CPU-only model, change kDLDrpAi to kDLCPU in the apps/MeraDrpRuntimeWrapper.cpp.

----
## ProfileRun
 - Run AI model graph with profile 

### Format

```c++
#include "MeraDrpRuntimeWrapper.h"

void MeraDrpRuntimeWrapper::ProfileRun(const std::string& profile_table, 
                                       const std::string& profile_csv);
void MeraDrpRuntimeWrapper::ProfileRun(const std::string& profile_table,
                                       const std::string& profile_csv, 
                                       int freq_index); // Works only on the RZ/V2H.
```

### Note

- **ProfileRun()** method runs the runtime and saves the runtime profiling results as a file. The profile result is information summarizing the execution time for each subgraph generated when the AI model is automatically divided into CPU and DRP-AI. The subgraph assigned to DRP-AI uses the name "tvmgen_default_tvmgen_default_mera_drp_*".
  - In the case of RZ/V2H, you can specify the `freq_index` argument to lower the operating frequency of the DRP-AI hardware. For example, setting `freq_index` to **5** will operate the DRP-AI at 315MHz.

### Return

- None

----

## Run

- Run AI model graph.

### Format

```c++
#include "MeraDrpRuntimeWrapper.h"

void MeraDrpRuntimeWrapper::Run(void);
void MeraDrpRuntimeWrapper::Run(int freq_index); // Works only on the RZ/V2H.
```

### Note

- **Run()** method performs inference.
  - In the case of RZ/V2H, you can specify the `freq_index` argument to lower the operating frequency of the DRP-AI hardware. For example, setting `freq_index` to **5** will operate the DRP-AI at 315MHz.

### Return

- None

----

## SetInput

- Copy data to the input layer.

### Format
```c++
#include "MeraDrpRuntimeWrapper.h"

template <typename T>
void MeraDrpRuntimeWrapper::SetInput(int input_index, const T* data_ptr);
```
### Note
 - **SetInput()** method copies the value specified by *data_ptr* to the input layer specified by the argument index. *data_ptr* data type is **float**/**unsigned short**(*16bit floating point*) only.

### Return   
 - None

[^1]: DRP-AI TVM is powered by EdgeCortix MERA™ Compiler Framework.
