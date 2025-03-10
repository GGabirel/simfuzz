
cmake_minimum_required (VERSION 2.8)

SET(CMAKE_C_COMPILER /usr/bin/gcc)

#工程名
project(unifuzzproject)


# 添加map库, PFUZZ库源文件
SET(LIB_MAP_SOURCE unifuzz/map.c)
include_directories(unifuzz)
# 静态编译map, PFUZZ库
add_library(map STATIC ${LIB_MAP_SOURCE})

# 设置目标二进制文件目录, 为build目录.
#set (EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})
add_subdirectory(unifuzz)
add_subdirectory(afl-client)
add_subdirectory(master-node)
add_subdirectory(pfcon)



