/*
 * (C) Copyright EdgeCortix, Inc. 2024
 */

#include <iostream>
#include <fstream>
#include <numeric>
#include <string>
#include <regex>
#include <sstream>
#include <dirent.h>

#include "MeraDrpRuntimeWrapper.h"
#include "mera2_runtime_plan/plan_io.h"
#include "mera_runtime.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include <dlpack/dlpack.h>
#include <tvm/runtime/module.h>
#include <tvm/runtime/packed_func.h>
#include <tvm/runtime/registry.h>
#include <tvm/runtime/profiling.h>
#include <builtin_fp16.h>

using namespace mera2_runtime_plan;

template <typename T>
std::vector<T> LoadBinary(const std::string& bin_file) {
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

class MeraDrpRuntimeWrapper::Impl {
 public:
  Impl(int device_type): device_type_(device_type) {
    console_ = spdlog::get("console");
    if (!console_) {
      // The logger was not registered, so need do it.
      console_ = spdlog::stdout_color_mt("console");
    }
  }
  virtual ~Impl() {};
  virtual bool LoadModel(const std::string& model_dir, uint64_t start_address, bool profile_mode) = 0;
  virtual uint64_t GetLastAddress() = 0;
  virtual void SetInput(int input_index, const float* data_ptr) = 0;
  virtual void SetInput(int input_index, const uint16_t* data_ptr) = 0;

  virtual std::vector<std::tuple<std::string, size_t, InOutDataType>> GetInputInfo() = 0;
  virtual void* GetInputPtr(const std::string& name) = 0;
  virtual std::vector<std::tuple<std::string, size_t, InOutDataType>> GetOutputInfo() = 0;
  virtual void* GetOutputPtr(const std::string& name) = 0;

  virtual void Run(int freq_index) = 0;
  virtual void ProfileRun(const std::string& profile_table, const std::string& profile_csv, int freq_index) = 0;
  virtual void ProfileRunMera2(const std::string& profile_table, int freq_index) {
      throw std::runtime_error("Supported for MERA2 only");
  };
  virtual int GetNumInput(std::string model_dir) = 0;
  virtual InOutDataType GetInputDataType(int index) = 0;
  virtual int GetNumOutput() = 0;
  virtual std::tuple<InOutDataType, void*, int64_t> GetOutput(int index) = 0;
 protected:
  int device_type_{kDLDrpAi};
  int device_id_ = 0;
  std::shared_ptr<spdlog::logger> console_;
};

class ImplMera2 : public MeraDrpRuntimeWrapper::Impl {
public:
  ImplMera2(int device_type): MeraDrpRuntimeWrapper::Impl(device_type) {
  }
  ~ImplMera2() = default;

  bool LoadModel(const std::string& model_dir, uint64_t start_address, bool profile_mode) override {
    const std::string plan_file = model_dir + "/mera.plan";
    console_->info("MERA 2.0 Runtime");
    auto plan = mera2_runtime_plan::Mera2RuntimePlanLoad(plan_file);
    rt_ = std::make_unique<MeraRuntime>(plan, model_dir);
    if (profile_mode) {
      rt_->EnableProfileMode();
    }
    rt_->SetStartAddress(start_address);
    rt_->SetDeviceType(device_type_);
    rt_->Init();
    auto input_info = rt_->InputNames();
    auto output_info = rt_->OutputNames();
    int idx = 0;
    for (const auto& [name, buffer] : input_info) {
      index_to_input_buffers_[idx++] = buffer;
    }
    idx = 0;
    for (const auto& [name, buffer] : output_info) {
      index_to_output_buffers_[idx++] = buffer;
    }
    return true;
  }

  uint64_t GetLastAddress() {
    return rt_->GetLastAddress();
  }

  void SetInput(int input_index, const float* data_ptr) override {
    SetInputImpl(input_index, data_ptr);
  }
  void SetInput(int input_index, const uint16_t* data_ptr) override {
    SetInputImpl(input_index, data_ptr);
  }

  template <typename T>
  void SetInputImpl(int input_index, const T* data_ptr) {
    auto input_name = index_to_input_buffers_[input_index].name;
    auto size = index_to_input_buffers_[input_index].bytes_size;
    auto* input_ptr = reinterpret_cast<void*>(rt_->GetInputPtr(input_name));
    std::memcpy(input_ptr, data_ptr, size);
  }

  std::vector<std::tuple<std::string, size_t, InOutDataType>> GetInputInfo() {
    std::vector<std::tuple<std::string, size_t, InOutDataType>> inputs{};
    auto input_info = rt_->InputNames();
    auto convert_dtype = [] (auto mera_dtype) {
      InOutDataType data_type = InOutDataType::OTHER;
      using DType_T = mera2_runtime_plan::Plan::MemoryPlan::DType;
      switch (mera_dtype) {
        case DType_T::FLOAT32: data_type = InOutDataType::FLOAT32; break;
        case DType_T::FLOAT16: data_type = InOutDataType::FLOAT16; break;
        case DType_T::INT64:  data_type = InOutDataType::INT64; break;
        case DType_T::INT32:  data_type = InOutDataType::INT32; break;
        default: throw std::runtime_error("Unsupported dtype");
      }
      return data_type;
    };

    for (const auto& [name, buffer] : input_info) {
      inputs.emplace_back(name, buffer.bytes_size, convert_dtype(buffer.data_type));
    }
    return std::move(inputs);
  }

  void* GetInputPtr(const std::string& name) {
    return reinterpret_cast<void*>(rt_->GetInputPtr(name));
  }

  std::vector<std::tuple<std::string, size_t, InOutDataType>> GetOutputInfo() {
    std::vector<std::tuple<std::string, size_t, InOutDataType>> outputs{};
    auto output_info = rt_->OutputNames();
    auto convert_dtype = [] (auto mera_dtype) {
      InOutDataType data_type = InOutDataType::OTHER;
      using DType_T = mera2_runtime_plan::Plan::MemoryPlan::DType;
      switch (mera_dtype) {
        case DType_T::FLOAT32: data_type = InOutDataType::FLOAT32; break;
        case DType_T::FLOAT16: data_type = InOutDataType::FLOAT16; break;
        case DType_T::INT64:  data_type = InOutDataType::INT64; break;
        case DType_T::INT32:  data_type = InOutDataType::INT32; break;
        default: throw std::runtime_error("Unsupported dtype");
      }
      return data_type;
    };

    for (const auto& [name, buffer] : output_info) {
      outputs.emplace_back(name, buffer.bytes_size, convert_dtype(buffer.data_type));
    }
    return std::move(outputs);
  }

  void* GetOutputPtr(const std::string& name) {
    return reinterpret_cast<void*>(rt_->GetOutputPtr(name));
  }

  void Run(int freq_index) override {
    rt_->SetFrequencyIndex(freq_index);
    rt_->Run();
  }

  void ProfileRun(const std::string& profile_table, const std::string& profile_csv, int freq_index) override {
    rt_->ProfileRun(profile_table, profile_csv, freq_index);
  }

  void ProfileRunMera2(const std::string& profile_table, int freq_index) override {
    rt_->SetFrequencyIndex(freq_index);
    rt_->Run();
    auto profile = rt_->ProfileReport();
    double total_latency;
    std::ofstream ofs_table (profile_table + ".txt", std::ofstream::out);
    for (const auto& p : profile) {
      ofs_table << "[RT] Unit " << p.first << ": " << p.second << " (us)" << std::endl;
      total_latency += p.second;
    }
    ofs_table << "[RT] Total latency: " << total_latency << " (us)" << std::endl;
  }

  int GetNumInput(std::string model_dir) override {
    return index_to_input_buffers_.size();
  }

  InOutDataType GetInputDataType(int index) override {
    auto mera_dtype = index_to_input_buffers_[index].data_type;
    InOutDataType data_type = InOutDataType::OTHER;
    using DType_T = mera2_runtime_plan::Plan::MemoryPlan::DType;
    switch (mera_dtype) {
      case DType_T::FLOAT32: data_type = InOutDataType::FLOAT32; break;
      case DType_T::FLOAT16: data_type = InOutDataType::FLOAT16; break;
      case DType_T::INT64:  data_type = InOutDataType::INT64; break;
      default: throw std::runtime_error("Unsupported dtype");
    }
    return data_type;
  }

  int GetNumOutput() override {
    return index_to_output_buffers_.size();
  }

  std::tuple<InOutDataType, void*, int64_t> GetOutput(int index) override {
    InOutDataType data_type = InOutDataType::OTHER;
    auto buffer = index_to_output_buffers_[index];
    using DType_T = mera2_runtime_plan::Plan::MemoryPlan::DType;
    switch (buffer.data_type) {
      case DType_T::FLOAT32: data_type = InOutDataType::FLOAT32; break;
      case DType_T::FLOAT16: data_type = InOutDataType::FLOAT16; break;
      case DType_T::INT64:  data_type = InOutDataType::INT64; break;
      case DType_T::INT32:  data_type = InOutDataType::INT32; break;
      default: throw std::runtime_error("Unsupported dtype");
    }
    auto* output_ptr = reinterpret_cast<void*>(rt_->GetOutputPtr(buffer.name));
    return std::make_tuple(data_type, output_ptr, buffer.size);
  }

private:
  std::unordered_map<int, mera2_runtime_plan::Plan::MemoryPlan::Buffer> index_to_input_buffers_{};
  std::unordered_map<int, mera2_runtime_plan::Plan::MemoryPlan::Buffer> index_to_output_buffers_{};
  std::unique_ptr<MeraRuntime> rt_;
};

class ImplDrpTvm : public MeraDrpRuntimeWrapper::Impl {
public:
  ImplDrpTvm(int device_type): MeraDrpRuntimeWrapper::Impl(device_type) {
  }
  ~ImplDrpTvm() = default;
  bool LoadModel(const std::string& model_dir, uint64_t start_address = 0x00, bool profile_mode = false) override {
    console_->info("MERA 1.0 (drp-tvm) Runtime");
    const std::string json_file(model_dir + "/deploy.json");
    std::ifstream json_in(json_file.c_str(), std::ios::in);
    std::string json_data((std::istreambuf_iterator<char>(json_in)), std::istreambuf_iterator<char>());
    json_in.close();
    tvm::runtime::Module mod_syslib = tvm::runtime::Module::LoadFromFile(model_dir + "/deploy.so");
    if (profile_mode) {
      mod = (*tvm::runtime::Registry::Get("tvm.graph_executor_debug.create"))(
        json_data, mod_syslib, device_type_, device_id_);
    } else {
      mod = (*tvm::runtime::Registry::Get("tvm.graph_executor.create"))(
        json_data, mod_syslib, device_type_, device_id_);
    }
    tvm::runtime::PackedFunc load_params = mod.GetFunction("load_params");
    auto params_data = LoadBinary<char>(model_dir + "/deploy.params");
    TVMByteArray params_arr;
    params_arr.data = params_data.data();
    params_arr.size = params_data.size();
    load_params(params_arr);
    tvm::runtime::PackedFunc set_start_address = mod.GetFunction("set_start_address");
    if (set_start_address != nullptr) {
      set_start_address(start_address);
    }
    return true;
  }

  uint64_t GetLastAddress() override {
    uint64_t last_address{0x00};
    tvm::runtime::PackedFunc get_last_address = mod.GetFunction("get_last_address");
    if (get_last_address != nullptr) {
      last_address = get_last_address();
    }
    return last_address;
  }

  void SetInput(int input_index, const float* data_ptr) override {
    SetInputImpl(input_index, data_ptr);
  }
  void SetInput(int input_index, const uint16_t* data_ptr) override {
    SetInputImpl(input_index, data_ptr);
  }

  std::vector<std::tuple<std::string, size_t, InOutDataType>> GetInputInfo() {
    std::cerr << "Mera1.x unsupports this function: " << __func__ << std::endl;
    return {};
  }

  void* GetInputPtr(const std::string& name) {
    std::cerr << "Mera1.x unsupports this function: " << __func__ << std::endl;
    return nullptr;
  }

  std::vector<std::tuple<std::string, size_t, InOutDataType>> GetOutputInfo() {
    std::cerr << "Mera1.x unsupports this function: " << __func__ << std::endl;
    return {};
  }

  void* GetOutputPtr(const std::string& name) {
    std::cerr << "Mera1.x unsupports this function: " << __func__ << std::endl;
    return nullptr;
  }

  template <typename T>
  void SetInputImpl(int input_index, const T* data_ptr) {
    tvm::runtime::PackedFunc get_input = mod.GetFunction("get_input");
    tvm::runtime::NDArray xx = get_input(input_index);
    auto in_shape = xx.Shape();
    std::vector<int64_t> ishape{};
    for (int i = 0; i < in_shape.size(); ++i) {
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

  void Run(int freq_index) override {
    mod.GetFunction("run")(freq_index);
  }

  void ProfileRun(const std::string& profile_table, const std::string& profile_csv, int freq_index) override {
    tvm::runtime::PackedFunc profile = mod.GetFunction("profile");
    tvm::runtime::Array<tvm::runtime::profiling::MetricCollector> collectors;
    tvm::runtime::profiling::Report report = profile(collectors, freq_index);

    std::string rep_table = report->AsTable();
    std::ofstream ofs_table (profile_table + ".txt", std::ofstream::out);
    ofs_table << rep_table << std::endl;
    ofs_table.close();

    std::string rep_csv = report->AsCSV();
    std::ofstream ofs_csv (profile_csv + ".csv", std::ofstream::out);
    ofs_csv << rep_csv << std::endl;
    ofs_csv.close();
  }

  int GetNumInput(std::string model_dir) override {
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

  InOutDataType GetInputDataType(int index) override {
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

  int GetNumOutput() override {
    return mod.GetFunction("get_num_outputs")();
  }

  std::tuple<InOutDataType, void*, int64_t> GetOutput(int index) override {
    tvm::runtime::PackedFunc get_output = mod.GetFunction("get_output");
    tvm::runtime::NDArray out = get_output(index);
    int64_t out_size = 1;
    for (int i = 0; i < out.Shape().size(); ++i) {
      out_size *= out.Shape()[i];
    }

    InOutDataType data_type = InOutDataType::OTHER;
    if (out.DataType().is_float() && out.DataType().bits() == 32) {
      data_type = InOutDataType::FLOAT32;
    } else if (out.DataType().is_float() && out.DataType().bits() == 16) {
      data_type = InOutDataType::FLOAT16;
    } else if (out.DataType().is_int() && out.DataType().bits() == 64) {
      data_type = InOutDataType::INT64;
    } else if (out.DataType().is_int() && out.DataType().bits() == 32) {
      data_type = InOutDataType::INT32;
    }
    return std::make_tuple(data_type, reinterpret_cast<void*>(out->data), out_size);
  }
 private:
  tvm::runtime::Module mod;
};

MeraDrpRuntimeWrapper::MeraDrpRuntimeWrapper(int device_type) : device_type_(device_type) {
};
#ifdef KDLCPUMODE
MeraDrpRuntimeWrapper::MeraDrpRuntimeWrapper() : device_type_(kDLCPU) {
};
#else
MeraDrpRuntimeWrapper::MeraDrpRuntimeWrapper() : device_type_(kDLDrpAi) {
};
#endif
MeraDrpRuntimeWrapper::~MeraDrpRuntimeWrapper() {};

bool MeraDrpRuntimeWrapper::LoadModel(const std::string& model_dir, uint64_t start_address) {
  const std::string plan_file = model_dir + "/mera.plan";
  std::ifstream plan_fd(plan_file);
  if (plan_fd.good()) {
    impl_ = std::make_unique<ImplMera2>(device_type_);
  } else {
    impl_ = std::make_unique<ImplDrpTvm>(device_type_);
  }
  return impl_->LoadModel(model_dir, start_address, true);
}

uint64_t MeraDrpRuntimeWrapper::GetLastAddress() {
  return impl_->GetLastAddress();
}

void MeraDrpRuntimeWrapper::SetInput(int input_index, const float* data_ptr) {
  impl_->SetInput(input_index, data_ptr);
}

void MeraDrpRuntimeWrapper::SetInput(int input_index, const uint16_t* data_ptr) {
  impl_->SetInput(input_index, data_ptr);
}

std::vector<std::tuple<std::string, size_t, InOutDataType>> MeraDrpRuntimeWrapper::GetInputInfo() {
  return impl_->GetInputInfo();
}

void* MeraDrpRuntimeWrapper::GetInputPtr(const std::string& name) {
  return impl_->GetInputPtr(name);
}

std::vector<std::tuple<std::string, size_t, InOutDataType>> MeraDrpRuntimeWrapper::GetOutputInfo() {
  return impl_->GetOutputInfo();
}

void* MeraDrpRuntimeWrapper::GetOutputPtr(const std::string& name) {
  return impl_->GetOutputPtr(name);
}

void MeraDrpRuntimeWrapper::Run(int freq_index) {
  impl_->Run(freq_index);
}

void MeraDrpRuntimeWrapper::Run() {
  impl_->Run(1 /* freq_index = 1 */);
}

void MeraDrpRuntimeWrapper::ProfileRun(const std::string& profile_table, const std::string& profile_csv, int freq_index) {
  impl_->ProfileRun(profile_table, profile_csv, freq_index);
}

void MeraDrpRuntimeWrapper::ProfileRun(const std::string& profile_table, const std::string& profile_csv) {
  impl_->ProfileRun(profile_table, profile_csv, 1 /* freq_index = 1 */);
}

void MeraDrpRuntimeWrapper::ProfileRunMera2(const std::string& profile_table, int freq_index) {
  impl_->ProfileRunMera2(profile_table, freq_index);
}

int MeraDrpRuntimeWrapper::GetNumInput(std::string model_dir) {
  return impl_->GetNumInput(model_dir);
}

InOutDataType MeraDrpRuntimeWrapper::GetInputDataType(int index) {
  return impl_->GetInputDataType(index);
}

int MeraDrpRuntimeWrapper::GetNumOutput() {
  return impl_->GetNumOutput();
}

std::tuple<InOutDataType, void*, int64_t> MeraDrpRuntimeWrapper::GetOutput(int index) {
  return impl_->GetOutput(index);
}
