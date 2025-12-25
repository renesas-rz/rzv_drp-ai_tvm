# CMakeList.txt : CMake projects in app_deeplabv3_cam
cmake_minimum_required (VERSION 3.24)
set(CMAKE_CXX_STANDARD 17)

if(NOT DEFINED APP_NAME)
  message(FATAL_ERROR "-DAPP_NAME is required. \n")
endif()

project(${APP_NAME} LANGUAGES C CXX)
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

# Setting and including TVM Directory
set(TVM_ROOT "$ENV{TVM_ROOT}")
include_directories(${TVM_ROOT}/tvm/include)
include_directories(${TVM_ROOT}/setup/include)
include_directories(${TVM_ROOT}/tvm/3rdparty/dlpack/include)
include_directories(${TVM_ROOT}/tvm/3rdparty/dmlc-core/include)
include_directories(${TVM_ROOT}/tvm/3rdparty/compiler-rt)
include_directories(./)
include_directories(${TVM_ROOT}/apps)
include_directories(${TVM_ROOT}/apps/include)
include_directories(${TVM_ROOT}/how-to/sample_app_v2h/common_files)
include_directories(${TVM_ROOT}/how-to/sample_app_v2h/common_files/package)
#include_directories(SYSTEM $ENV{OECORE_NATIVE_SYSROOT} ${POKY_PATH}/usr/include )

set (LIBMERA_RT_PATH ${TVM_ROOT}/obj/build_runtime/v2h/lib)
add_definitions(-DMERA_DRP_RUNTIME)
set(EXE_NAME ${PROJECT_NAME}) 

file(GLOB SRC * ${TVM_ROOT}/how-to/sample_app_v2h/common_files/* ${TVM_ROOT}/apps/MeraDrpRuntimeWrapper.* ${TVM_ROOT}/apps/PreRuntime.h ${TVM_ROOT}/apps/PreRuntimeV2H.cpp)
if("${APP_NAME}" STREQUAL "app_yolov2_cam")
    list(APPEND SRC
        "${CMAKE_CURRENT_LIST_DIR}/../../app_yolox_cam/src/image_yolox.cpp"
    )
endif()

include_directories(${CMAKE_SYSROOT}/usr/src/debug/gstreamer1.0-plugins-base/1.22.12/gst-libs/gst/gl)
list(APPEND SRC ${CMAKE_SYSROOT}/usr/src/debug/gstreamer1.0-plugins-base/1.22.12/gst-libs/gst/gl/xdg-shell-client-protocol.c)

set(PRODUCT $ENV{PRODUCT})

if(PRODUCT STREQUAL "V2H")
    add_definitions(-DV2H)
    message("-- PRODUCT [V2H]")
elseif(PRODUCT STREQUAL "V2N")
    add_definitions(-DV2H -DV2N)
    message("-- PRODUCT [V2N]")
else()
    message("-- PRODUCT [???]")
endif()

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,${LIBMERA_RT_PATH} -Wl,-rpath-link,${LIBMERA_RT_PATH}")
add_executable(${EXE_NAME} ${SRC})


target_compile_definitions(${EXE_NAME} PRIVATE IMAGE_MODE)
target_compile_definitions(${EXE_NAME} PRIVATE V2H)
target_compile_definitions(${EXE_NAME} PRIVATE DEBUG_LOG)
target_compile_definitions(${EXE_NAME} PRIVATE NDEBUG)
target_compile_features(${EXE_NAME} PUBLIC cxx_std_17)

#link_directories(${POKY_PATH}/usr/lib64/)
#include_directories("include")
target_include_directories(${EXE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${EXE_NAME} PUBLIC ${TVM_ROOT}/3rdparty/asio/asio/include)
target_include_directories(${EXE_NAME} PUBLIC ${TVM_ROOT}/3rdparty/spdlog/include)



target_link_directories(${EXE_NAME} PRIVATE ${LIBMERA_RT_PATH})

find_package(OpenCV REQUIRED)
if(OpenCV_FOUND)
    target_include_directories(${EXE_NAME} PUBLIC ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(${EXE_NAME} PRIVATE ${OpenCV_LIBS})
endif()

target_link_libraries(${EXE_NAME} PRIVATE
  mera2_runtime
  mera2_plan_io
  drp_tvm_rt
  pthread
  turbojpeg
  jpeg
  EGL 
  GLESv2
  wayland-cursor 
  wayland-egl 
  wayland-client
  mmngr 
  mmngrbuf
)
target_compile_options(${EXE_NAME} PRIVATE -O3 -mtune=cortex-a55 -Wall)
