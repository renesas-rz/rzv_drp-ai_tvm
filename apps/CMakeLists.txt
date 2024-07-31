cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
project(tutorial_app_cpp)

set(TVM_ROOT $ENV{TVM_HOME})
include_directories(${TVM_ROOT}/include)
include_directories(${TVM_ROOT}/3rdparty/dlpack/include)
include_directories(${TVM_ROOT}/3rdparty/dmlc-core/include)
include_directories(${TVM_ROOT}/3rdparty/compiler-rt)
#include_directories($ENV{SDK}/sysroots/aarch64-poky-linux/usr/include/opencv4)

set(TVM_RUNTIME_LIB ${TVM_ROOT}/build_runtime/libtvm_runtime.so)
if(V2H)
  set(SRC tutorial_app.cpp MeraDrpRuntimeWrapper.cpp PreRuntimeV2H.cpp)
else()
  set(SRC tutorial_app_v2ml.cpp MeraDrpRuntimeWrapper.cpp PreRuntime.cpp)
endif()


set(EXE_NAME tutorial_app)
add_executable(${EXE_NAME} ${SRC})
target_link_libraries(${EXE_NAME} ${TVM_RUNTIME_LIB})
target_link_libraries(${EXE_NAME} ${OpenCV_LIBS})
target_compile_options(${EXE_NAME} PRIVATE -O3 -Ofast -mtune=cortex-a55 -Wall -DDEBUG_LOG)
