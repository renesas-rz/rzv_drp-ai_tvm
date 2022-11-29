cmake_minimum_required (VERSION 3.8)
project(V2MA_TVM_USB_SAMPLE)

set(TVM_ROOT $ENV{TVM_HOME})
include_directories(${TVM_ROOT}/include)
include_directories(${TVM_ROOT}/3rdparty/dlpack/include)
include_directories(${TVM_ROOT}/3rdparty/dmlc-core/include)
include_directories(${TVM_ROOT}/3rdparty/compiler-rt)

set(EXE_NAME sample_app_drpai_tvm_usbcam_http)
set(TVM_RUNTIME_LIB ${TVM_ROOT}/build_runtime/libtvm_runtime.so)

file(GLOB SOURCE *.cpp *.h */*.cpp */*.h  */*/*.cpp */*/*.h)
add_executable (${EXE_NAME}
${SOURCE}
)

target_compile_options(${EXE_NAME} PUBLIC ${CXX} ${LDFLAGS} ${OpenMP_CXX_FLAGS} -lm -lrt -O3 -DNDEBUG)
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
