cmake_minimum_required(VERSION 3.24)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(inference)

set(default_build_type "Release")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

if(NOT TVM_ROOT)
  message(SEND_ERROR "Need to specify location of drp-tvm with option TVM_ROOT")
endif()

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories(${TVM_ROOT}/include)
include_directories(${TVM_ROOT}/3rdparty/dlpack/include)
include_directories(${TVM_ROOT}/3rdparty/dmlc-core/include)
include_directories(${TVM_ROOT}/3rdparty/compiler-rt)

add_executable(inference
  ${SOURCE_DIR}/inference.cc
  ${SOURCE_DIR}/MeraDrpRuntimeWrapper.cc
)

add_definitions(-DMERA_DRP_RUNTIME)
target_include_directories(inference PUBLIC ${SOURCE_DIR}/include)
target_include_directories(inference PUBLIC ${SOURCE_DIR}/asio/asio/include)
target_include_directories(inference PUBLIC ${SOURCE_DIR}/spdlog/include)

set (LIBMERA_RT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib)
target_link_directories(inference PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,${LIBMERA_RT_PATH} -Wl,-rpath-link,${LIBMERA_RT_PATH}")

target_link_libraries(inference PRIVATE
  mera2_runtime
  mera2_plan_io
  drp_tvm_rt
  pthread
)
