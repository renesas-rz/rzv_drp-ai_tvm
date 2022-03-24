/*
 * (C) Copyright EdgeCortix, Inc. 2021
 */
#include <dlpack/dlpack.h>
#include <tvm/runtime/module.h>
#include <tvm/runtime/packed_func.h>
#include <tvm/runtime/registry.h>
#include <tvm/runtime/profiling.h>
#include <builtin_fp16.h>

#include <algorithm>
#include <chrono>
#include <fstream>
#include <numeric>
#include <string>
#include <regex>
#include <dirent.h>
#include <getopt.h>

enum class InOutDataType {
  FLOAT32,
  FLOAT16,
  OTHER
};

std::ostream& operator<<(std::ostream& os, InOutDataType type) {
  switch (type) {
    case InOutDataType::FLOAT32:
      os << "FLOAT32";
      break;
    case InOutDataType::FLOAT16:
      os << "FLOAT16";
      break;
    case InOutDataType::OTHER:
      os << "OTHER";
      break;
  }
  return os;
}

float float16_to_float32(uint16_t a) {
  return __extendXfYf2__<uint16_t, uint16_t, 10, float, uint32_t, 23>(a);
}

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

template <typename T>
void CheckOutput(const T* const ref_result, const T* const tvm_result, size_t size,
                 float atol = 1e-2, float rtol = 1e-3) {
  int num_correct = 0;
  T max_abs_diff = std::numeric_limits<T>::min();
  T mean_abs_diff = 0;
  int64_t non_zero_count = 0;
  for (size_t i = 0; i < size; ++i) {
    auto abs_diff = std::abs(tvm_result[i] - ref_result[i]);
    max_abs_diff = std::max(abs_diff, max_abs_diff);
    mean_abs_diff += abs_diff;
    non_zero_count += static_cast<int64_t>(0.0f != tvm_result[i]);
    num_correct += static_cast<int>(static_cast<float>(abs_diff) <= atol + std::abs(rtol * ref_result[i]));
  }

  LOG(INFO) << "max abs diff: " << max_abs_diff;
  LOG(INFO) << "mean abs diff: " << mean_abs_diff / size;
  LOG(INFO) << "correct ratio: " << num_correct / (double) size << ", with absolute tolerance: " << atol << ", relative tolerance: " << rtol;
  LOG(INFO) << "Non-zero values: " << non_zero_count / (double) size * 100.0 << "(%)" << std::endl;
}

// CheckOutput for FP16
void CheckOutput(const uint16_t* const ref_result, const uint16_t* const tvm_result, size_t size,
                 float atol = 1e-2, float rtol = 1e-3) {
  int num_correct = 0;
  float max_abs_diff = std::numeric_limits<float>::min();
  float mean_abs_diff = 0;
  int64_t non_zero_count = 0;
  for (size_t i = 0; i < size; ++i) {
    float tvm_result_f32 = float16_to_float32(tvm_result[i]);
    float ref_result_f32 = float16_to_float32(ref_result[i]);

    auto abs_diff = std::abs(tvm_result_f32 - ref_result_f32);
    max_abs_diff = std::max(abs_diff, max_abs_diff);
    mean_abs_diff += abs_diff;
    non_zero_count += static_cast<int64_t>(0.0f != tvm_result_f32);
    num_correct += static_cast<int>(abs_diff <= atol + std::abs(rtol * ref_result_f32));
  }
  LOG(INFO) << "max abs diff: " << max_abs_diff;
  LOG(INFO) << "mean abs diff: " << mean_abs_diff / size;
  LOG(INFO) << "correct ratio: " << num_correct / (double) size << ", with absolute tolerance: " << atol << ", relative tolerance: " << rtol;
  LOG(INFO) << "Non-zero values: " << non_zero_count / (double) size * 100.0 << "(%)" << std::endl;
}

class MeraDrpRuntimeWrapper {
 public:
  MeraDrpRuntimeWrapper() {};
  ~MeraDrpRuntimeWrapper() = default;

  void LoadModel(const std::string& model_dir) {
    LOG(INFO) << "Loading json data...";
    const std::string json_file(model_dir + "/deploy.json");
    std::ifstream json_in(json_file.c_str(), std::ios::in);
    std::string json_data((std::istreambuf_iterator<char>(json_in)), std::istreambuf_iterator<char>());
    json_in.close();

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
  }

  template <typename T>
  void SetInput(int input_index, const T* data_ptr) {
    LOG(INFO) << "Loading input...";

    tvm::runtime::PackedFunc get_input = mod.GetFunction("get_input");
    tvm::runtime::NDArray xx = get_input(0);
    auto in_shape = xx.Shape();
    int64_t in_size = 1;
    for (int i = 0; i < in_shape.size(); ++i) {
      in_size *= in_shape[i];
    }

    DLDevice ctx;
    ctx.device_id = device_id;
    ctx.device_type = DLDeviceType(device_type);

    auto input_array = tvm::runtime::NDArray::Empty(in_shape, xx.DataType(), ctx);
    auto input_data = (T*)(input_array->data);
    std::memcpy(input_data, data_ptr, sizeof(T) * in_size);
    tvm::runtime::PackedFunc set_input = mod.GetFunction("set_input");
    set_input(input_index, input_array);
  }

  void Run() {
    mod.GetFunction("run")();
  }

  void ProfileRun(const std::string& profile_table, const std::string& profile_csv) {
    tvm::runtime::PackedFunc profile = mod.GetFunction("profile");
    tvm::runtime::Array<tvm::runtime::profiling::MetricCollector> collectors;
    tvm::runtime::profiling::Report report = profile(collectors);

    std::string rep_table = report->AsTable();
    std::ofstream ofs_table (profile_table, std::ofstream::out);
    ofs_table << rep_table << std::endl;
    ofs_table.close();

    std::string rep_csv = report->AsCSV();
    std::ofstream ofs_csv (profile_csv, std::ofstream::out);
    ofs_csv << rep_csv << std::endl;
    ofs_csv.close();
  }

  int GetNumInput(std::string model_dir) {
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

  InOutDataType GetInputDataType(int index) {
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

  int GetNumOutput() {
    return mod.GetFunction("get_num_outputs")();
  }

  std::tuple<InOutDataType, void*, int64_t> GetOutput(int index) {
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
    }
    return std::make_tuple(data_type, reinterpret_cast<void*>(out->data), out_size);
  }

 private:
  int device_type = kDLCPU;
  int device_id = 0;
  tvm::runtime::Module mod;
};

int main(int argc, char** argv) {
  auto usage_log = [] {
    std::cout << "Usage: inference [arguments]" << std::endl;
    std::cout << "Arguments:" << std::endl;
    std::cout << "  -h, --help           Show this usage log" << std::endl;
    std::cout << "  -d, --deploy         Specific deployment folder" << std::endl;
    std::cout << "  -a, --atol           Specific absolute tolerent to comparing with reference, default 0.01" << std::endl;
    std::cout << "  -r, --rtol           Specific relative tolerent to comparing with reference, default 0.001" << std::endl;
    std::cout << "  -p, --profile        Fine-grained per-operator profiling mode" << std::endl;
    std::cout << "  -t, --profile-table  Output file to store profiling data in table format" << std::endl;
    std::cout << "  -c, --profile-csv    Output file to store profiling data in CSV format" << std::endl;
    std::cout << "Example: ./inference -d model_dir" << std::endl;
  };

  int c = 0;
  float atol = 0.01, rtol = 0.001;
  std::string model_dir = "";
  std::string profile_table = "./profile_table.txt";
  std::string profile_csv = "./profile.csv";
  bool profile_mode = false;

  while (1) {
    struct option long_options[] = {
      {"help", no_argument, 0, 'h'},
      {"deploy", required_argument, 0, 'd'},
      {"atol", required_argument, 0, 'a'},
      {"rtol", required_argument, 0, 'r'},
      {"profile", no_argument, 0, 'p'},
      {"profile-table", optional_argument, 0, 't'},
      {"profile-csv", optional_argument, 0, 'c'},
      {0, 0, 0, 0}
    };

    int option_index = 0;
    c = getopt_long(argc, argv, "d:a:r:t:c:ph", long_options, &option_index);
    if (-1 == c) {
      break;
    }

    switch (c) {
      case 0:
        /* If this option set a flag, do nothing else now. */
        break;
      case 'd':
        model_dir = std::string(optarg);
        break;
      case 'a':
        atol = std::stof(optarg);
        break;
      case 'r':
        rtol = std::stof(optarg);
        break;
      case 'p':
        profile_mode = true;
        break;
      case 't':
        profile_table = std::string(optarg);
        break;
      case 'c':
        profile_csv = std::string(optarg);
        break;
      case 'h':
      default:
        usage_log();
        return 0;
    }
  }

  if (optind < argc) {
    std::cout << "non-option ARGV-elements: " << std::endl;
    while (optind < argc) {
      std::cout << argv[optind++] << std::endl;
    }
  }

  if (model_dir.empty()) {
    usage_log();
    return 0;
  }

  MeraDrpRuntimeWrapper runtime;
  runtime.LoadModel(model_dir);

  auto input_num = runtime.GetNumInput(model_dir);
  const std::string input_name("input");
  for (int i = 0; i < input_num; i++) {
    std::string filename = model_dir + "/" + input_name + "_" + std::to_string(i) + ".bin";
    auto input_data_type = runtime.GetInputDataType(i);
    if (InOutDataType::FLOAT32 == input_data_type) {
      auto input = LoadBinary<float>(filename);
      runtime.SetInput(i, input.data());
    } else {
      CHECK_EQ(InOutDataType::FLOAT16, input_data_type);
      auto input = LoadBinary<uint16_t>(filename);
      runtime.SetInput(i, input.data());
    }
  }

  if (profile_mode) {
    LOG(INFO) << "Profiling mode enabled";
    runtime.ProfileRun(profile_table, profile_csv);
    return 0;
  }

  // warm up
  LOG(INFO) << "Warming up...";
  for (int i = 0; i < 3; i++) {
    runtime.Run();
  }

  const int num_runs = 100;
  LOG(INFO) << "Running " << num_runs << " times...";
  auto t0 = std::chrono::system_clock::now();
  for (int i = 0; i < num_runs; ++i) {
    runtime.Run();
  }
  auto t1 = std::chrono::system_clock::now();
  auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
  LOG(INFO) << "Took " << elapsed_ms / (double)num_runs << " msec.";

  auto output_num = runtime.GetNumOutput();
  // Comparing output with reference.
  for (int i = 0; i < output_num; i++) {
    auto output_buffer = runtime.GetOutput(i);
    if (InOutDataType::FLOAT32 == std::get<0>(output_buffer)) {
      float* data_ptr = reinterpret_cast<float*>(std::get<1>(output_buffer));
      int64_t out_size = std::get<2>(output_buffer);

      std::string filename(model_dir + "/" + "ref_result_" + std::to_string(i) + ".bin");
      auto ref_result = LoadBinary<float>(filename);

      CHECK_EQ(out_size, ref_result.size());
      CheckOutput(ref_result.data(), data_ptr, out_size, atol, rtol);
    } else {
      CHECK_EQ(InOutDataType::FLOAT16, std::get<0>(output_buffer));
      uint16_t* data_ptr = reinterpret_cast<uint16_t*>(std::get<1>(output_buffer));
      int64_t out_size = std::get<2>(output_buffer);

      std::string filename(model_dir + "/" + "ref_result_" + std::to_string(i) + ".bin");
      auto ref_result = LoadBinary<uint16_t>(filename);

      CHECK_EQ(out_size, ref_result.size());
      CheckOutput(ref_result.data(), data_ptr, out_size, atol, rtol);
    }
  }

  return 0;
}
