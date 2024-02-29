cmake_minimum_required (VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
project(V2H_TVM_USB_SAMPLE)

set(TVM_ROOT $ENV{TVM_HOME})
include_directories(${TVM_ROOT}/include)
include_directories(${TVM_ROOT}/../apps)
include_directories(${TVM_ROOT}/3rdparty/dlpack/include)
include_directories(${TVM_ROOT}/3rdparty/dmlc-core/include)
include_directories(${TVM_ROOT}/3rdparty/compiler-rt)
include_directories(${TVM_ROOT}/../how-to/sample_app_v2h/src/camera)
include_directories(${TVM_ROOT}/../how-to/sample_app_v2h/src/package)

set(EXE_NAME sample_app_drpai_tvm_usbcam_yolox)
set(TVM_RUNTIME_LIB ${TVM_ROOT}/build_runtime/libtvm_runtime.so)

file(GLOB SOURCE *.cpp *.h */*.cpp */*.h  */*/*.cpp */*/*.h ../../../apps/MeraDrpRuntimeWrapper.* ../../../apps/PreRuntime.h ../../../apps/PreRuntimeV2H.cpp)
add_executable (${EXE_NAME}
${SOURCE}
)

target_compile_options(${EXE_NAME} PUBLIC ${CXX} ${LDFLAGS} ${OpenMP_CXX_FLAGS} -lm -lrt -O3 -DNDEBUG)
target_compile_options(${EXE_NAME} PRIVATE -Wall )
target_compile_definitions(${EXE_NAME} PRIVATE IMAGE_MODE)
target_compile_definitions(${EXE_NAME} PRIVATE V2H)
target_compile_definitions(${EXE_NAME} PRIVATE DEBUG_LOG)
target_compile_features(${EXE_NAME} PUBLIC  cxx_std_17)
link_directories("/usr/lib64/")
include_directories("include")
include_directories("/usr/include")
include_directories(SYSTEM $ENV{OECORE_NATIVE_SYSROOT} $ENV{SDK}/sysroots/aarcch64-poky-linux/usr/include)
TARGET_LINK_LIBRARIES(${EXE_NAME} pthread)
TARGET_LINK_LIBRARIES(${EXE_NAME} turbojpeg)
TARGET_LINK_LIBRARIES(${EXE_NAME} jpeg)

find_package(OpenCV REQUIRED)
if(OpenCV_FOUND)
    target_include_directories(${EXE_NAME} PUBLIC ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(${EXE_NAME} ${OpenCV_LIBS})
endif()

target_link_libraries(${EXE_NAME} ${TVM_RUNTIME_LIB})
target_link_libraries(${EXE_NAME} EGL GLESv2) 
target_link_libraries(${EXE_NAME} wayland-cursor wayland-egl wayland-client) 
target_link_libraries(${EXE_NAME} mmngr mmngrbuf) 

