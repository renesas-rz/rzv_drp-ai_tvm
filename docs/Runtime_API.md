
# MERA Wrapper API References

## List of functions

| Function name                                    | Note                                                        |
| ----------------------------------------- | ----------------------------------------------------------- |
| [GetInputDataType](#GetInputDataType) | Return input data type                                    |
| [GetNumOutput](#GetNumOutput)         | Get the number of outputs from the graph.                   |
| [GetOutput](#GetOutput)               | Returns inference result                                        |
| [LoadModel](#LoadModel)               | Deploy the DRP-AI TV[^1] model       |
| [ProfileRun](#ProfileRun)             | Run AI model graph with profile                                  |
| [Run](#Run)                           |Run AI model graph                                                   |
| [SetInput](#SetInput)                 | Copy data to the input layer                                |

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
 - Output datat type
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
 - The number of out put node. A non-zero positive integer. A value of 0 or negative indicates that a problem occurred during the processing of this function.

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

void MeraDrpRuntimeWrapper::LoadModel(const std::string& model_dir);
```
### Note
 - Expand the inference model and its weights stored in the runtime model data directory specified by the argument `model_dir` to the resources secured in the MeraDrpRuntimeWrapper class. By executing this method, other methods defined in the MeraDrpRuntimeWrapper class can be used.
### Return   
 - None

----
## ProfileRun
 - Run AI model graph with profile 

### Format
```c++
#include "MeraDrpRuntimeWrapper.h"

void MeraDrpRuntimeWrapper::ProfileRun(const std::string& profile_table, 
                                       const std::string& profile_csv);
```
### Note
- **ProfileRun()** method runs the runtime and saves the runtime profiling results as a file. The profile result is information summarizing the execution time for each subgraph generated when the AI model is automatically divided into CPU and DRP-AI. The subgraph assigned to DRP-AI uses the name "tvmgen_default_tvmgen_default_mera_drp_*".
### Return   
- None

----
## Run
 - Run AI model graph.

### Format
```c++
#include "MeraDrpRuntimeWrapper.h"

void MeraDrpRuntimeWrapper::Run(void);
```
### Note
 - **Run()** method performs inference. 
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