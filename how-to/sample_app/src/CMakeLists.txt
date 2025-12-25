cmake_minimum_required (VERSION 3.24)

project(sample_app_drpai_tvm_usbcam_http LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Default build type
set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Environment checks
if(NOT DEFINED ENV{TVM_ROOT})
  message(SEND_ERROR "Need to specify location of drp-tvm with option TVM_ROOT")
endif()
if(NOT DEFINED ENV{PRODUCT})
  message(SEND_ERROR "Need to specify product name with option PRODUCT")
endif()

# Directories based on TVM_ROOT
set(TVM_ROOT "$ENV{TVM_ROOT}")
include_directories(${TVM_ROOT}/tvm/include)
include_directories(${TVM_ROOT}/setup/include)
include_directories(${TVM_ROOT}/tvm/3rdparty/dlpack/include)
include_directories(${TVM_ROOT}/tvm/3rdparty/dmlc-core/include)
include_directories(${TVM_ROOT}/tvm/3rdparty/compiler-rt)
include_directories(${TVM_ROOT}/apps)
include_directories(${TVM_ROOT}/apps/include)

set(EXE_NAME ${PROJECT_NAME})

# Runtime path
set(LIBMERA_RT_PATH ${TVM_ROOT}/obj/build_runtime/v2m/lib)
add_definitions(-DMERA_DRP_RUNTIME)


# Sources (for how-to/sample_app/src)
file(GLOB_RECURSE  SOURCE
  ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/*.h
  ${TVM_ROOT}/apps/MeraDrpRuntimeWrapper.*
  ${TVM_ROOT}/apps/PreRuntime.*
)
list(FILTER SOURCE EXCLUDE REGEX "CMakeFiles")
add_executable (${EXE_NAME}
${SOURCE}
)

# rpath to runtime libs
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,${LIBMERA_RT_PATH} -Wl,-rpath-link,${LIBMERA_RT_PATH}")

target_compile_options(${EXE_NAME} PUBLIC ${CXX} ${LDFLAGS} ${OpenMP_CXX_FLAGS} -lm -lrt -O3 -DNDEBUG)
target_compile_features(${EXE_NAME} PUBLIC  cxx_std_17)
# Local include and 3rdparty
target_include_directories(${EXE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${EXE_NAME} PUBLIC ${TVM_ROOT}/3rdparty/asio/asio/include)
include_directories(SYSTEM $ENV{OECORE_NATIVE_SYSROOT} $ENV{SDK}/sysroots/aarch64-poky-linux/usr/include)

# Link directories and libraries
target_link_directories(${EXE_NAME} PRIVATE ${LIBMERA_RT_PATH})

target_link_libraries(${EXE_NAME} PRIVATE
  mera2_runtime
  mera2_plan_io
  drp_tvm_rt
  pthread
  turbojpeg
  jpeg
)

# Use spdlog if present
if(EXISTS "${TVM_ROOT}/3rdparty/spdlog/include")
  target_include_directories(${EXE_NAME} PUBLIC ${TVM_ROOT}/3rdparty/spdlog/include)
endif()

# OpenCV
find_package(OpenCV REQUIRED)
if(OpenCV_FOUND)
  target_include_directories(${EXE_NAME} PUBLIC ${OpenCV_INCLUDE_DIRS})
  target_link_libraries(${EXE_NAME} PRIVATE ${OpenCV_LIBS})
endif()

