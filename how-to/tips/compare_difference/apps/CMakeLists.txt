cmake_minimum_required(VERSION 3.10)
project(inference_comparison_cpp)

set(TVM_ROOT ${CMAKE_CURRENT_BINARY_DIR}/../../tvm/)
include_directories(${TVM_ROOT}/include)
include_directories(${TVM_ROOT}/3rdparty/dlpack/include)
include_directories(${TVM_ROOT}/3rdparty/dmlc-core/include)
include_directories(${TVM_ROOT}/3rdparty/compiler-rt)

set(TVM_RUNTIME_LIB ${TVM_ROOT}/build_runtime/libtvm_runtime.so)
set(SRC inference_comparison.cpp MeraDrpRuntimeWrapper.cpp)
set(EXE_NAME inference_comparison)

add_executable(${EXE_NAME} ${SRC})
target_link_libraries(${EXE_NAME} ${TVM_RUNTIME_LIB})
