
cmake_minimum_required (VERSION 2.8)

SET(CMAKE_C_COMPILER /usr/bin/gcc)

IF(NOT PREFIX)
    set(CMAKE_INSTALL_PREFIX .)
ELSE()
    set(CMAKE_INSTALL_PREFIX ${PREFIX})
ENDIF()
add_compile_options("-Wall")

# 添加map库源文件
#SET(LIB_MAP_SOURCE ../unifuzz/map.c)
# 静态编译map
#add_library(map STATIC ${LIB_MAP_SOURCE})


#调用PkgConfig
find_package(PkgConfig)
pkg_search_module(MONGOCPKG REQUIRED libmongoc-static-1.0)

#SET(UNIFUZZ_DIR ${PROJECT_SOURCE_DIR})



#编译/master-node=======
set(PREFIX     /usr/local)
set(BIN_PATH    ${PREFIX}/bin)
set(HELPER_PATH ${PREFIX}/lib/afl)
set(DOC_PATH    ${PREFIX}/share/doc/afl)
set(MISC_PATH   ${PREFIX}/share/afl)

set ( CMAKE_C_FLAGS " -std=gnu99  -O0 -march=native -funroll-loops -Wall -D_FORTIFY_SOURCE=2 -g -Wno-pointer-sign -DAFL_PATH=\\\"${HELPER_PATH}\\\" -DDOC_PATH=\\\"${DOC_PATH}\\\" -DBIN_PATH=\\\"${BIN_PATH}\\\" ")

set ( CMAKE_CXX_FLAGS " -std=c++11 -O0 -march=native -g -fpermissive -D_FILE_OFFSET_BITS=64 -Wall -D_FORTIFY_SOURCE=2 -Wno-pointer-sign -DAFL_PATH=\\\"${HELPER_PATH}\\\" -DDOC_PATH=\\\"${DOC_PATH}\\\" -DBIN_PATH=\\\"${BIN_PATH}\\\" ")

# 添加头文件
include_directories(.)
include_directories(../unifuzz)


# 添加mongoc的头文件路径
include_directories(${MONGOCPKG_INCLUDE_DIRS})
message(STATUS "=== 4MONGOCPKG_CFLAGS:${include_directories}")

# 设置目标二进制文件目录, 为build目录.
set (EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
# 构建可执行程序
add_executable(master-node master-node.c)
TARGET_LINK_LIBRARIES(master-node map  unifuzzdg dl ${MONGOCPKG_CFLAGS} ${MONGOCPKG_LIBRARIES}  ${MONGOCPKG_LDFLAGS} )
#install(TARGETS master-node RUNTIME DESTINATION bin )
#编译master-node完成===========
