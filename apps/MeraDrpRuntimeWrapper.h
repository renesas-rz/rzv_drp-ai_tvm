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

enum class InOutDataType {
  FLOAT32,
  FLOAT16,
  OTHER
};

class MeraDrpRuntimeWrapper {
 public:
  MeraDrpRuntimeWrapper();
  ~MeraDrpRuntimeWrapper();

  bool LoadModel(const std::string& model_dir, uint32_t start_address);
  bool LoadModel(const std::string& model_dir, uint64_t start_address);
  template <typename T>
  void SetInput(int input_index, const T* data_ptr);
  void Run();
  void Run(int freq_index);
  void ProfileRun(const std::string& profile_table, const std::string& profile_csv);
  void ProfileRun(const std::string& profile_table, const std::string& profile_csv, int freq_index);
  int GetNumInput(std::string model_dir);
  InOutDataType GetInputDataType(int index);
  int GetNumOutput();

  std::tuple<InOutDataType, void*, int64_t> GetOutput(int index);

 private:
  int device_type;
  int device_id;
  tvm::runtime::Module mod;
};
