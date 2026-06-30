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
    set(SRC tutorial_app.cpp MeraDrpRuntimeWrapper.cpp PreRuntimeV2H.cpp)
    set(EXE_NAME tutorial_app)
else()
  set (LIBMERA_RT_PATH ${TVM_ROOT}/obj/build_runtime/v2m/lib)
  add_definitions(-DMERA_DRP_RUNTIME)
  
  # Base source files for V2M/V2MA/V2L
  set(SRC tutorial_app_v2ml.cpp MeraDrpRuntimeWrapper.cpp PreRuntime.cpp)
  set(EXE_NAME tutorial_app_v2ml)
  
  # tutorial_app_v2ml.cpp directly uses mmngr APIs.
  find_library(MMNGR_LIB mmngr PATHS /usr/lib /usr/lib64 /usr/lib/aarch64-linux-gnu)
  find_library(MMNGRBUF_LIB mmngrbuf PATHS /usr/lib /usr/lib64 /usr/lib/aarch64-linux-gnu)
  
  if(MMNGR_LIB AND MMNGRBUF_LIB)
    message(STATUS "mmngr found: ${MMNGR_LIB}")
    message(STATUS "mmngrbuf found: ${MMNGRBUF_LIB}")
    message(STATUS "Enabling mmngr support")
    
    # mmngr_alloc_in_user_ext() phard_addr argument type differs per environment:
    #   - RZ/V2M (dunfell)  : 64-bit physical address -> unsigned long*
    #   - RZ/V2L (scarthgap): 32-bit physical address -> unsigned int*
    # Define MMNGR_PHYS_ADDR_64BIT for V2M/V2MA so tutorial_app_v2ml.cpp selects the matching type.
    string(TOUPPER "$ENV{PRODUCT}" PRODUCT_UPPER)
    if(PRODUCT_UPPER MATCHES "V2M")
      add_definitions(-DMMNGR_PHYS_ADDR_64BIT)
      message(STATUS "mmngr phys addr type: 64-bit (unsigned long) for PRODUCT=$ENV{PRODUCT}")
    else()
      message(STATUS "mmngr phys addr type: 32-bit (unsigned int) for PRODUCT=$ENV{PRODUCT}")
    endif()

    include_directories(/usr/include)
  else()
    message(FATAL_ERROR "mmngr libraries are required for tutorial_app_v2ml.cpp")
  endif()
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

# Link mmngr libraries if available (V2M/V2MA/V2L only)
if(MMNGR_LIB AND MMNGRBUF_LIB AND NOT (V2H OR V2N))
  target_link_libraries(${EXE_NAME} PRIVATE ${MMNGR_LIB} ${MMNGRBUF_LIB})
  message(STATUS "Linking mmngr libraries")
endif()

if (V2H OR V2N)
    target_compile_definitions(${EXE_NAME} PUBLIC KDLDRPAI)
else()
    target_compile_definitions(${EXE_NAME} PUBLIC KDLCPUMODE)
endif()

target_compile_options(${EXE_NAME} PRIVATE -O3 -mtune=cortex-a55 -Wall -DDEBUG_LOG)
