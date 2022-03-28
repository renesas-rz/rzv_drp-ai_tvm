/*
 * Original Code (C) Copyright Edgecortix, Inc. 2022
 * Modified Code (C) Copyright Renesas Electronics Corporation 2022　
 */
#include <tvm/runtime/module.h>

enum class InOutDataType {
  FLOAT32,
  FLOAT16,
  OTHER
};

class MeraDrpRuntimeWrapper {
 public:
  MeraDrpRuntimeWrapper();
  ~MeraDrpRuntimeWrapper();

  void LoadModel(const std::string& model_dir);
  template <typename T>
  void SetInput(int input_index, const T* data_ptr);
  void Run();
  void ProfileRun(const std::string& profile_table, const std::string& profile_csv);
  int GetNumInput(std::string model_dir);
  InOutDataType GetInputDataType(int index);
  int GetNumOutput();

  std::tuple<InOutDataType, void*, int64_t> GetOutput(int index);

 private:
  int device_type;
  int device_id;
  tvm::runtime::Module mod;
};
