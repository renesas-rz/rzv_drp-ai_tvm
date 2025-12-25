/* 
* (C) Copyright EdgeCortix, Inc. 2025 
*/

#ifndef MERA2_RUNTIME_PLAN_IO_H
#define MERA2_RUNTIME_PLAN_IO_H

#include "plan.h"

namespace mera2_runtime_plan {

Plan Mera2RuntimePlanLoad(const std::string& plan_file);
std::string Mera2RuntimePlanToString(const Plan& plan);

} // namespace mera2_runtime_plan

#endif // MERA2_RUNTIME_PLAN_IO_H
