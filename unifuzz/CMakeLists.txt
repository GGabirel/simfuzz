
cmake_minimum_required (VERSION 2.8)

#SET(CMAKE_C_COMPILER /usr/bin/gcc)
#工程名
#project(unifuzzdg)

add_compile_options("-Wall")
IF (BUILD_TYPE MATCHES "Debug")
    message( "build type is Debug." )
    set ( CMAKE_C_FLAGS "-std=c11 -O3 -D_FILE_OFFSET_BITS=64 -g -DDEBUG -fpermissive")
    set ( CMAKE_CXX_FLAGS "-std=c++11 -O3 -D_FILE_OFFSET_BITS=64 -g -DDEBUG -fpermissive")
ELSE()
    message( "build type is Release." )
    set ( CMAKE_C_FLAGS "-std=c11 -O3 -D_FILE_OFFSET_BITS=64 -g -fpermissive")
    set ( CMAKE_CXX_FLAGS "-std=c++11 -O3 -D_FILE_OFFSET_BITS=64 -g -fpermissive")

ENDIF()

IF(NOT PREFIX)
    set(CMAKE_INSTALL_PREFIX .)
ELSE()
    set(CMAKE_INSTALL_PREFIX ${PREFIX})
ENDIF()


#调用PkgConfig
find_package(PkgConfig)
pkg_search_module(MONGOCPKG REQUIRED libmongoc-static-1.0)

# 添加头文件
include_directories(.)
include_directories(../pfcon)
include_directories(../afl-client)

# 添加mongoc的头文件路径
include_directories(${MONGOCPKG_INCLUDE_DIRS})

# 添加unifuzz/data-engine相关库源文件
set(UNIFUZZ_DG_SRC data-engine.cpp util.cpp unifuzz.cpp )
# 静态编译
add_library(unifuzzdg STATIC ${UNIFUZZ_DG_SRC})
# 设置目标二进制文件目录, 为build目录.
set (EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

# 构建可执行程序
#add_executable(run_dg run_data_engine.c)

#target_link_libraries(run_dg map unifuzzdg ${MONGOCPKG_CFLAGS} ${MONGOCPKG_LIBRARIES}  ${MONGOCPKG_LDFLAGS}  )



