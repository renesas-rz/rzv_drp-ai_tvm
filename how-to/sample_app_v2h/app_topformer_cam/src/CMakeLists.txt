# CMakeList.txt : CMake projects in app_topformer_cam
cmake_minimum_required (VERSION 3.10)
project (app_topformer_cam)
set(CMAKE_CXX_STANDARD 17)

# Setting and including TVM Directory
set(TVM_ROOT $ENV{TVM_HOME})
include_directories(./)
include_directories(${TVM_ROOT}/include)
include_directories(${TVM_ROOT}/../apps)
include_directories(${TVM_ROOT}/3rdparty/dlpack/include)
include_directories(${TVM_ROOT}/3rdparty/dmlc-core/include)
include_directories(${TVM_ROOT}/3rdparty/compiler-rt)
include_directories(${TVM_ROOT}/../how-to/sample_app_v2h/common_files)
include_directories(${TVM_ROOT}/../how-to/sample_app_v2h/common_files/package)

set(TVM_RUNTIME_LIB ${TVM_ROOT}/build_runtime/libtvm_runtime.so)
set(EXE_NAME app_topformer_cam)

file(GLOB SOURCE *.cpp *.h ${TVM_ROOT}/../how-to/sample_app_v2h/common_files/*.cpp ${TVM_ROOT}/../how-to/sample_app_v2h/common_files/*.h  ../../../../apps/MeraDrpRuntimeWrapper.* ../../../../apps/PreRuntime.h ../../../../apps/PreRuntimeV2H.cpp)
add_executable (${EXE_NAME}
${SOURCE}
)

target_compile_options(${EXE_NAME} PRIVATE -O3 -Ofast -mtune=cortex-a55 -Wall)
target_compile_definitions(${EXE_NAME} PRIVATE IMAGE_MODE)
target_compile_definitions(${EXE_NAME} PRIVATE V2H)
target_compile_definitions(${EXE_NAME} PRIVATE DEBUG_LOG)
target_compile_definitions(${EXE_NAME} PRIVATE NDEBUG)
target_compile_features(${EXE_NAME} PUBLIC cxx_std_17)

link_directories(${POKY_PATH}/usr/lib64/)
include_directories("include")
include_directories(SYSTEM $ENV{OECORE_NATIVE_SYSROOT} ${POKY_PATH}/usr/include )

TARGET_LINK_LIBRARIES(${EXE_NAME} pthread)
TARGET_LINK_LIBRARIES(${EXE_NAME} turbojpeg)
TARGET_LINK_LIBRARIES(${EXE_NAME} jpeg)
TARGET_LINK_LIBRARIES(${EXE_NAME} EGL GLESv2) 
TARGET_LINK_LIBRARIES(${EXE_NAME} wayland-cursor wayland-egl wayland-client) 
TARGET_LINK_LIBRARIES(${EXE_NAME} mmngr mmngrbuf) 

find_package(OpenCV REQUIRED)
if(OpenCV_FOUND)
    target_include_directories(${EXE_NAME} PUBLIC ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(${EXE_NAME} ${OpenCV_LIBS})
endif()

target_link_libraries(${EXE_NAME} ${TVM_RUNTIME_LIB})
