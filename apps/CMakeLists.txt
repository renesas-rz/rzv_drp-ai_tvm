cmake_minimum_required(VERSION 3.24)
set(CMAKE_CXX_STANDARD 17)
project(tutorial_app)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(default_build_type "Release")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

if(NOT DEFINED ENV{TVM_ROOT})
  message(SEND_ERROR "Need to specify location of drp-tvm with option TVM_ROOT")
endif()

if(NOT DEFINED ENV{PRODUCT})
  message(SEND_ERROR "Need to specify product name with option PRODUCT")
endif()

set(TVM_ROOT "$ENV{TVM_ROOT}")
include_directories(${TVM_ROOT}/tvm/include)
include_directories(${TVM_ROOT}/setup/include)
include_directories(${TVM_ROOT}/tvm/3rdparty/dlpack/include)
include_directories(${TVM_ROOT}/tvm/3rdparty/dmlc-core/include)
include_directories(${TVM_ROOT}/tvm/3rdparty/compiler-rt)

if(V2H OR V2N)
  set (LIBMERA_RT_PATH ${TVM_ROOT}/obj/build_runtime/v2h/lib)
  add_definitions(-DMERA_DRP_RUNTIME)
  if(BENCH)
    set(SRC benchmark.cpp MeraDrpRuntimeWrapper.cpp PreRuntimeV2H.cpp)
    set(EXE_NAME benchmark)
  elseif(INFERENCE)
    set(SRC run_inference_v2h.cpp MeraDrpRuntimeWrapper.cpp PreRuntimeV2H.cpp)
    set(EXE_NAME run_inference)
  else()
    set(SRC tutorial_app.cpp MeraDrpRuntimeWrapper.cpp PreRuntimeV2H.cpp)
    set(EXE_NAME tutorial_app)
  endif()
else()
  set (LIBMERA_RT_PATH ${TVM_ROOT}/obj/build_runtime/v2m/lib)
  add_definitions(-DMERA_DRP_RUNTIME)
  if(BENCH)
    message(FATAL_ERROR "For now, benchmark program supports only V2H or V2N.")
  endif()
  set(SRC tutorial_app_v2ml.cpp MeraDrpRuntimeWrapper.cpp PreRuntime.cpp)
  set(EXE_NAME tutorial_app_v2ml)
endif()

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,${LIBMERA_RT_PATH} -Wl,-rpath-link,${LIBMERA_RT_PATH}")
add_executable(${EXE_NAME} ${SRC})
target_include_directories(${EXE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${EXE_NAME} PUBLIC ${TVM_ROOT}/3rdparty/asio/asio/include)
target_include_directories(${EXE_NAME} PUBLIC ${TVM_ROOT}/3rdparty/spdlog/include)
target_link_directories(${EXE_NAME} PRIVATE ${LIBMERA_RT_PATH})
find_package(OpenCV REQUIRED)
target_link_libraries(${EXE_NAME} PRIVATE ${OpenCV_LIBS})
target_link_libraries(${EXE_NAME} PRIVATE
  mera2_runtime
  mera2_plan_io
  drp_tvm_rt
  pthread
)
if (V2H OR V2N)
    target_compile_definitions(${EXE_NAME} PUBLIC KDLDRPAI)
else()
    target_compile_definitions(${EXE_NAME} PUBLIC KDLCPUMODE)
endif()

target_compile_options(${EXE_NAME} PRIVATE -O3 -mtune=cortex-a55 -Wall -DDEBUG_LOG)
