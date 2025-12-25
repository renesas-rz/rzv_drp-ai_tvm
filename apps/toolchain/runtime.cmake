set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_SYSTEM_VERSION 1)

if(EXISTS "$ENV{SDK}/sysroots/cortexa55-poky-linux")
  set(CMAKE_SYSROOT "$ENV{SDK}/sysroots/cortexa55-poky-linux")
  set(CMAKE_FIND_ROOT_PATH "$ENV{SDK}/sysroots/cortexa55-poky-linux/usr/include/gnu")
else()
  set(CMAKE_SYSROOT "$ENV{SDK}/sysroots/aarch64-poky-linux")
  set(CMAKE_FIND_ROOT_PATH "$ENV{SDK}/sysroots/aarch64-poky-linux/usr/include/gnu")
endif()

set(CMAKE_CXX_COMPILER "$ENV{SDK}/sysroots/x86_64-pokysdk-linux/usr/bin/aarch64-poky-linux/aarch64-poky-linux-g++")
set(CMAKE_C_COMPILER "$ENV{SDK}/sysroots/x86_64-pokysdk-linux/usr/bin/aarch64-poky-linux/aarch64-poky-linux-gcc")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
