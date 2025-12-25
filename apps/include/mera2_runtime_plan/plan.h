/* 
* (C) Copyright EdgeCortix, Inc. 2025 
*/

#ifndef MERA2_RUNTIME_PLAN_H
#define MERA2_RUNTIME_PLAN_H

#include <cstddef>
#include <string>
#include <vector>
#include <map>
#include <set>

#ifdef _MSC_VER
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

//
// A Plan consist on:
//  * A memory plan: buffers to be used and shared between execution units
//  * An execution plan: definition of execution units and order of execution
//
// The execution plan contains definitions of the execution Units that make use
// of the buffers specified on the memory plan.
// Buffers are referenced by index on the Memory Plan
//
// Execution plan also specifies the order of execution of the Units.
// The execution plan is divided into stages of execution.
// Each Stage is a list of Groups.
// Units within the same Group are meant to be executed in parallel.
// Within the same Stage, each group is meant to be executed serially.
//
namespace mera2_runtime_plan {

struct Plan {
  struct MemoryPlan {
    enum class AllocationMode {
      UNDEFINED=0,
      LINUX_MALLOC,
      LINUX_HUGE_PAGES
    };

    enum class DType {
      UNDEFINED=0,
      BOOL,
      UINT8,
      INT8,
      INT32,
      FLOAT16,
      FLOAT32,
      INT64
    };

    struct Buffer {
      size_t alignment;
      size_t offset;
      size_t size;
      size_t bytes_size;
      std::vector<size_t> shape;
      AllocationMode alloc_mode;
      DType data_type;
      std::string name;
    };

    std::vector<Buffer> buffers;
    std::vector<std::pair<std::string, size_t>> input_buffers;
    std::vector<std::pair<std::string, size_t>> output_buffers;

    size_t ComputeBufferAreaMaxSize() const;
  } memory;

  struct ExecutionPlan {
    enum class Backend {
      UNDEFINED=0,
      DEBUG,
      MERA_DNA,
      MERA_DNAX,
      MERA_DRP,
      MERA_DRPAI,
      ACL,
      APACHE_TVM,
      DRP_TVM,
      MERA_BLOCK,
      MERA_C99_SOURCE_CPU,
      MERA_C99_SOURCE_ETHOS_U55,
    };
    struct Unit {
      Backend backend;
      std::string name; // should be unique
      std::string binaries_location;
    };

    struct Allocation {
      std::map<std::string, size_t> input_buffers;
      std::map<std::string, size_t> output_buffers;
      std::vector<std::string> input_order;
      std::vector<std::string> output_order;
    };

    // An execution plan is represented as a list of stages
    // - Stage 1: [(unit1)]
    // - Stage 2: [(unit2), (unit3, unit4)]
    // - Stage 3: [(unit5)]
    //
    // A group is represented as a list of units (u1, u2, u3...) that need
    // to be executed in parallel.
    // An stage is a list of groups.
    // Groups in a stage are executed one after another serially
    //
    // () ==> Group
    // [] ==> Stage
    // [[], [], ...] ==> Plan
    //
    // (...) ==> parallel execution
    // [...] => serial execution
    //
    typedef std::vector<std::pair<Unit, Allocation>> Group;
    typedef std::vector<Group> Stage;
    std::vector<Stage> stages;

    void AddStage(const std::vector<Group>& stage) {
      return stages.push_back(stage);
    }

    //
    // utilities
    //
    std::set<Backend> Backends() const {
      std::set<Backend> backends;
      for (const auto& stage : stages) {
        for (const auto& group : stage) {
          for (const auto& [unit, allocation] : group) {
            backends.insert(unit.backend);
          }
        }
      }
      return backends;
    }

    bool ParallelExecutionRequired() const {
      for (const auto& stage : stages) {
        for (const auto& sequence : stage) {
          if (sequence.size() > 1) {
            return true;
          }
        }
      }
      return false;
    }

  } execution;

  EXPORT void SaveTo(const std::string &path) const;
};

} // namespace mera2_runtime_plan

#endif // MERA_PLAN_H
