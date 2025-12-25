/*
 * (C) Copyright EdgeCortix, Inc. 2024
 */

#ifndef MERA_DRP_RUNTIME_API_H_
#define MERA_DRP_RUNTIME_API_H_

#include <string>
#include <memory>
#include <ostream>
#include <tvm/runtime/profiling.h>

enum class InOutDataType {
  FLOAT32,
  FLOAT16,
  INT32,
  INT64,
  OTHER
};

class MeraDrpRuntimeWrapper {
public:
  MeraDrpRuntimeWrapper(int device_type);
  MeraDrpRuntimeWrapper();
  ~MeraDrpRuntimeWrapper();

  bool LoadModel(const std::string& model_dir, uint64_t start_address);
  uint64_t GetLastAddress();

  void SetInput(int input_index, const float* data_ptr);
  void SetInput(int input_index, const uint16_t* data_ptr);
  int GetNumInput(std::string model_dir);
  InOutDataType GetInputDataType(int index);
  int GetNumOutput();
  std::tuple<InOutDataType, void*, int64_t> GetOutput(int index);

  // Zero copy api.
  std::vector<std::tuple<std::string, size_t, InOutDataType>> GetInputInfo();
  void* GetInputPtr(const std::string& name);
  std::vector<std::tuple<std::string, size_t, InOutDataType>> GetOutputInfo();
  void* GetOutputPtr(const std::string& name);

  void Run(int freq_index);
  void Run();

  void ProfileRun(const std::string& profile_table, const std::string& profile_csv);
  void ProfileRun(const std::string& profile_table, const std::string& profile_csv, int freq_index);

  // new API to get MERA2 profile level.
  void ProfileRunMera2(const std::string& profile_table, int freq_index);

  class Impl;
private:
  std::unique_ptr<Impl> impl_;
  int device_type_;
};

#endif /* MERA_DRP_RUNTIME_API_H_ */
