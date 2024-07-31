/*
 * Original Code (C) Copyright Edgecortix, Inc. 2022
 * Modified Code (C) Copyright Renesas Electronics Corporation 2023　
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 */
#include <tvm/runtime/module.h>
#include <tvm/runtime/packed_func.h>
#include <tvm/runtime/registry.h>
#include <tvm/runtime/profiling.h>

#include <fstream>
#include <regex>
#include <dirent.h>
#include "MeraDrpRuntimeWrapper.h"

template <typename T>
static std::vector<T> LoadBinary(const std::string& bin_file) {
  std::ifstream file(bin_file.c_str(), std::ios::in | std::ios::binary);
  if (!file.is_open()) {
    LOG(FATAL) << "unable to open file " + bin_file;
  }

  file.seekg(0, file.end);
  const uint32_t file_size = static_cast<uint32_t>(file.tellg());
  file.seekg(0, file.beg);

  const auto file_buffer = std::unique_ptr<char>(new char[file_size]);
  file.read(file_buffer.get(), file_size);

  if (file.bad() || file.fail()) {
    LOG(FATAL) << "error occured while reading the file";
  }

  file.close();

  auto ptr = reinterpret_cast<T*>(file_buffer.get());
  const auto num_elements = file_size / sizeof(T);
  return std::vector<T>(ptr, ptr + num_elements);
}

MeraDrpRuntimeWrapper::MeraDrpRuntimeWrapper() {
  //device_type = kDLCPU;
  device_type = kDLDrpAi;
  device_id = 0;
};

MeraDrpRuntimeWrapper::~MeraDrpRuntimeWrapper() = default;

bool MeraDrpRuntimeWrapper::LoadModel(const std::string& model_dir, uint32_t start_address){
    device_type = kDLCPU;

    return LoadModel(model_dir, (uint64_t)start_address);
}

bool MeraDrpRuntimeWrapper::LoadModel(const std::string& model_dir, uint64_t start_address = 0x00) {
    LOG(INFO) << "Loading json data...";
    const std::string json_file(model_dir + "/deploy.json");
    std::ifstream json_in(json_file.c_str(), std::ios::in);
    std::string json_data((std::istreambuf_iterator<char>(json_in)), std::istreambuf_iterator<char>());
    json_in.close();

    #if 0
    if(json_data.find("drp") == json_data.npos && device_type != kDLCPU){
        LOG(INFO) <<"Break! this model is Not for DRP-AI retry as CPU Only";
        return false;
    }
    #else
    if(json_data.find("drp") == json_data.npos && device_type != kDLCPU){
        LOG(INFO) <<"try as CPU Only";
        device_type = kDLCPU;
    }
    #endif

    LOG(INFO) << "Loading runtime module...";
    tvm::runtime::Module mod_syslib = tvm::runtime::Module::LoadFromFile(model_dir + "/deploy.so");
    mod = (*tvm::runtime::Registry::Get("tvm.graph_executor_debug.create"))(
      json_data, mod_syslib, device_type, device_id);

    LOG(INFO) << "Loading parameters...";
    tvm::runtime::PackedFunc load_params = mod.GetFunction("load_params");
    auto params_data = LoadBinary<char>(model_dir + "/deploy.params");
    TVMByteArray params_arr;
    params_arr.data = params_data.data();
    params_arr.size = params_data.size();
    load_params(params_arr);
    tvm::runtime::PackedFunc set_start_address = mod.GetFunction("set_start_address");
    if(set_start_address != nullptr){
      set_start_address(start_address);
    }
    return true;
}

template <typename T>
void MeraDrpRuntimeWrapper::SetInput(int input_index, const T* data_ptr) {
    tvm::runtime::PackedFunc get_input = mod.GetFunction("get_input");
    tvm::runtime::NDArray xx = get_input(input_index);
    auto in_shape = xx.Shape();
    std::vector<int64_t> ishape{};
    for (unsigned long i = 0; i < in_shape.size(); ++i) {
      ishape.emplace_back(in_shape[i]);
    }

    DLTensor dlt;
    dlt.device = {kDLCPU, 0};
    dlt.ndim = ishape.size();
    dlt.shape = ishape.data();
    dlt.byte_offset = 0;
    dlt.strides = nullptr;
    dlt.data = const_cast<void*>(static_cast<const void*>(data_ptr));
    if (std::is_same<T, float>::value) {
      dlt.dtype = {kDLFloat, 32, 1};
    } else if (std::is_same<T, uint16_t>::value) {
      dlt.dtype = {kDLFloat, 16, 1};
    } else {
      LOG(FATAL) << "Unsupport input type " << typeid(T).name();
    }
    tvm::runtime::PackedFunc set_input = mod.GetFunction("set_input");
    set_input(input_index, &dlt);
}

template void MeraDrpRuntimeWrapper::SetInput<float>(int input_index, const float*);
template void MeraDrpRuntimeWrapper::SetInput<unsigned short>(int input_index, const unsigned short*);

void MeraDrpRuntimeWrapper::Run() {
    mod.GetFunction("run")();
}

void MeraDrpRuntimeWrapper::Run(int freq_index) {
    mod.GetFunction("run")(freq_index);
}

void MeraDrpRuntimeWrapper::ProfileRun(const std::string& profile_table, const std::string& profile_csv) {
    
    ProfileRun(profile_table, profile_csv, 1);

}

void MeraDrpRuntimeWrapper::ProfileRun(const std::string& profile_table, const std::string& profile_csv, int freq_index) {
    tvm::runtime::PackedFunc profile = mod.GetFunction("profile");
    tvm::runtime::Array<tvm::runtime::profiling::MetricCollector> collectors;
    tvm::runtime::profiling::Report report = profile(collectors, freq_index);

    std::string rep_table = report->AsTable();
    std::ofstream ofs_table (profile_table, std::ofstream::out);
    ofs_table << rep_table << std::endl;
    ofs_table.close();

    std::string rep_csv = report->AsCSV();
    std::ofstream ofs_csv (profile_csv, std::ofstream::out);
    ofs_csv << rep_csv << std::endl;
    ofs_csv.close();
}

int MeraDrpRuntimeWrapper::GetNumInput(std::string model_dir) {
    // TVM does not support api to get number input of model.
    // This function calculate input number base on convention
    // of input data name (input_xyz.bin)
    DIR *dir;
    dirent *diread;
    int num_input = 0;
    if ((dir = opendir(model_dir.c_str())) != nullptr) {
      while ((diread = readdir(dir)) != nullptr) {
        std::string file_name(diread->d_name);
        if (std::regex_match(file_name, std::regex("(input_)(.*)(bin)") )) {
          num_input++;
        }
      }
      closedir(dir);
    } else {
      LOG(FATAL) << "Can not open model dir : " << model_dir;
    }

    return num_input;
}

InOutDataType MeraDrpRuntimeWrapper::GetInputDataType(int index) {
    tvm::runtime::PackedFunc get_input = mod.GetFunction("get_input");
    tvm::runtime::NDArray input_info = get_input(index);
    InOutDataType data_type = InOutDataType::OTHER;
    if (input_info.DataType().is_float() && input_info.DataType().bits() == 32) {
      data_type = InOutDataType::FLOAT32;
    } else if (input_info.DataType().is_float() && input_info.DataType().bits() == 16) {
      data_type = InOutDataType::FLOAT16;
    }
    return data_type;
  }

int MeraDrpRuntimeWrapper::GetNumOutput() {
    return mod.GetFunction("get_num_outputs")();
  }

std::tuple<InOutDataType, void*, int64_t> MeraDrpRuntimeWrapper::GetOutput(int index) {
    tvm::runtime::PackedFunc get_output = mod.GetFunction("get_output");
    tvm::runtime::NDArray out = get_output(index);
    int64_t out_size = 1;
    for ( unsigned long i = 0; i < out.Shape().size(); ++i) {
      out_size *= out.Shape()[i];
    }

    InOutDataType data_type = InOutDataType::OTHER;
    if (out.DataType().is_float() && out.DataType().bits() == 32) {
      data_type = InOutDataType::FLOAT32;
    } else if (out.DataType().is_float() && out.DataType().bits() == 16) {
      data_type = InOutDataType::FLOAT16;
    } else if (out.DataType().is_int() && out.DataType().bits() == 64) {
      data_type = InOutDataType::INT64;
    }
    return std::make_tuple(data_type, reinterpret_cast<void*>(out->data), out_size);
}
