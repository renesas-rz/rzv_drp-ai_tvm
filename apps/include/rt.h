/* 
* (C) Copyright EdgeCortix, Inc. 2025 
*/

#ifndef MERA_RT_H
#define MERA_RT_H

#include <string>
#include <map>
#include <cstdint>

#include "mera2_runtime_plan/plan.h"

typedef mera2_runtime_plan::Plan::MemoryPlan::Buffer Buffer;

class Runtime {
public:
  virtual ~Runtime() {}
  virtual void Init(const std::string& deployment_location) = 0;
  virtual void SetInputMemoryPtr(const std::string& area_name, void* address) = 0;
  virtual void SetOutputMemoryPtr(const std::string& area_name, void* address) = 0;
  virtual void Run() = 0;
  // for Renesas
  virtual void ProfileRun(const std::string& profile_table, const std::string& profile_csv, int freq_index) {};
  virtual void SetMemoryAreaIndex(const std::string& area_name, size_t index) {};
  virtual void* Allocate(size_t size) {return nullptr;};
  virtual void SetInputAreaInfo(const std::string& area_name, const Buffer& buffer) {};
  virtual void SetOutputAreaInfo(const std::string& area_name, const Buffer& buffer) {};
  virtual void SetUnitName(const std::string& unit_name) {};
  virtual void Prepare() {};
  virtual void SetStartAddress(uint64_t address) {};
  virtual void SetFrequencyIndex(int freq_index) {};
  virtual void SetDeviceType(int dev_type) {};
  virtual uint64_t GetLastAddress() {return 0;};
  void SetProfileMode() {
    profile_mode_enabled = true;
  }
  void SetDynamicOutputs(const std::set<std::string> &dyn_outputs) {
    dynamic_outputs = dyn_outputs;
  }

  virtual std::map<std::string, size_t> GetInputAreasByteSizes() {
    return {};
  };

  virtual std::map<std::string, size_t> GetOutputAreasByteSizes() {
    return {};
  };

  virtual std::string GetRuntimeMetrics() {
    return "";
  }

  virtual std::string GetPowerMetrics() {
    return "";
  }

  virtual void SetDevices(const std::vector<int>& device_ids) {};

protected:
  bool profile_mode_enabled {false};
  std::set<std::string> dynamic_outputs{};
};

#endif // MERA_RT_H
