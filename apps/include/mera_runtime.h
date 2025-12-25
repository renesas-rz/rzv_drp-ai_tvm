/* 
* (C) Copyright EdgeCortix, Inc. 2025 
*/

#include <chrono>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <list>
#include <numeric>

#include <spdlog/spdlog.h>
#include <asio.hpp>
#include <asio/io_context.hpp>

#include "mera2_runtime_plan/plan_io.h"
#include "rt.h"
#ifdef MERA_DNA_RUNTIME
#include <mera/mdna_interpreter.h>
#endif /* MERA_DNA_RUNTIME */

class Profile {
public:
  Profile() = default;

  void Add(std::string name, double duration_us) {
    times[name].second += duration_us;
    times[name].first++;
  }

  std::map<std::string, double> Avg() {
    std::map<std::string, double> avg;
    for (const auto& [name, ts] : times) {
      avg[name] = ts.second / ts.first;
    }
    return avg;
  }

private:
  std::map<std::string, std::pair<size_t, double>> times;
};

using unique_void_ptr = std::unique_ptr<void, void(*)(void const*)>;

class MeraRuntime {
public:
  MeraRuntime(const mera2_runtime_plan::Plan& plan, const std::string& root_dir);
  ~MeraRuntime();

  void EnableProfileMode();
  void SetDevices(const std::vector<int>& device_ids);
  void Init();
  void Run();
  // for Renesas
  void ProfileRun(const std::string& profile_table, const std::string& profile_csv, int freq_index);
  void* GetInputPtr(const std::string& name);
  void* GetOutputPtr(const std::string& name);
  std::map<std::string, double> ProfileReport();

  void SetDynamicOutputs(bool needs_row_fetch, const std::set<std::string> &outputs) {
    needs_row_fetching = needs_row_fetch;
    dynamic_output_list = outputs;
  }
  inline bool IsDynamicOutput(const std::string &name) const {
    return std::find(dynamic_output_list.begin(), dynamic_output_list.end(), name) != dynamic_output_list.end();
  }
  inline bool NeedsRowFetching() const { return needs_row_fetching; }
  void FetchDynamicOutput(const std::string &output_name, size_t row_idx);

  void SetDynamicPosition(int64_t pos_idx);

  std::vector<std::pair<std::string, mera2_runtime_plan::Plan::MemoryPlan::Buffer>> InputNames();
  std::vector<std::pair<std::string, mera2_runtime_plan::Plan::MemoryPlan::Buffer>> OutputNames();
  std::vector<size_t> GetInputShape(const std::string& name);
  std::vector<size_t> GetOutputShape(const std::string& name);
  std::map<std::string, std::string> GetRuntimeMetrics();
  std::map<std::string, std::string> GetPowerMetrics();

  void SetStartAddress(uint64_t address) {
    start_address = address;
  };
  uint64_t GetLastAddress() {
    // start_address is updated for next drp-tvm instance.
    // so it keeps exactly last address of loaded instance.
    return start_address;
  };
  void SetFrequencyIndex(int index) {
    freq_index = index;
  };
  void SetDeviceType(int dev_type) {
    device_type = dev_type;
  };

private:
  void RunUnitAndNotify(const std::string& name);
  void UnitFinished();

  const mera2_runtime_plan::Plan plan;
  const std::string root_dir;
  std::vector<unique_void_ptr> io_areas;
  std::map<std::string, std::unique_ptr<Runtime>> execution_units;
#ifdef MERA_DRP_RUNTIME
  std::unique_ptr<Runtime> drpai_buffer_mng;
#endif /* MERA_DRP_RUNTIME */
  // Dynamic outputs
  bool needs_row_fetching{false};
  std::set<std::string> dynamic_output_list{};
  // for Renesas
  uint64_t start_address{0x00};
  int freq_index{1};
  int device_type{1 /* default kDLCPU */};

  std::shared_ptr<spdlog::logger> logger;
  Profile profile;
  bool profile_mode_enabled {false};

  // Queues and threads: only initialized and used if we find
  // that the execution plan requires parallel executions
  typedef std::shared_ptr<asio::io_context>
    io_context_ptr;
  typedef asio::executor_work_guard<asio::io_context::executor_type>
    io_context_work;
  io_context_ptr cpu_queue;
  io_context_ptr notification_queue;
  std::list<io_context_work> work;
  std::vector<std::shared_ptr<asio::thread>> threads;
  size_t parallel_tasks_running{0};
  std::vector<int> device_ids;
};

template<typename T>
class MeraRuntimeInputView {
public:
    MeraRuntimeInputView(MeraRuntime& mrt)
      : mrt(mrt) {}
    void SelectArea(const std::string& area_name) {
      name = area_name;
    }
    T* getValues() {
      return reinterpret_cast<T*>(mrt.GetInputPtr(name));
    }
    std::vector<size_t> GetShape() {
      return mrt.GetInputShape(name);
    }
private:
    MeraRuntime& mrt;
    std::string name;
};

template<typename T>
class MeraRuntimeOutputView {
public:
    MeraRuntimeOutputView(MeraRuntime& mrt)
      : mrt(mrt) {}
    void SelectArea(const std::string& area_name) {
      name = area_name;
    }
    void FetchDynamicOutput(size_t row_idx) {
      if (mrt.NeedsRowFetching()) {
        mrt.FetchDynamicOutput(name, row_idx);
      } else {
        const int W = GetShape().back();
        ptr_offset = W * row_idx;
      }
    }
    T* getValues() {
      return reinterpret_cast<T*>(mrt.GetOutputPtr(name)) + ptr_offset;
    }
    std::vector<size_t> GetShape() {
      auto shape = mrt.GetOutputShape(name);
      if (mrt.IsDynamicOutput(name)) {
        // Force it to have height=1
        if (shape.size() > 1) {
          shape[shape.size() - 2] = 1;
        }
      }
      return shape;
    }
private:
    MeraRuntime& mrt;
    std::string name;
    size_t ptr_offset{0};
};
