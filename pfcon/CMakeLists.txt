
cmake_minimum_required (VERSION 2.8)

SET(CMAKE_C_COMPILER /usr/bin/gcc)

IF(NOT PREFIX)
    set(CMAKE_INSTALL_PREFIX .)
ELSE()
    set(CMAKE_INSTALL_PREFIX ${PREFIX})
ENDIF()
add_compile_options("-Wall")



find_package(PkgConfig)
pkg_search_module(MONGOCPKG REQUIRED libmongoc-static-1.0)



# 添加头文件
include_directories(.)
include_directories(../unifuzz)
include_directories(../afl-client)

# 添加mongoc的头文件路径
include_directories(${MONGOCPKG_INCLUDE_DIRS})


# 链接库文件
#LINK_DIRECTORIES(${MONGOCPKG_INCLUDE_DIRS})
 

# 设置目标二进制文件目录, 为build目录.
set (EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

# 构建可执行程序
add_executable(pfcon pfcon.c)

TARGET_LINK_LIBRARIES(pfcon map unifuzzdg ${MONGOCPKG_CFLAGS}  ${MONGOCPKG_LDFLAGS})
#install(TARGETS pfcon RUNTIME DESTINATION binc )



#get_property(dirs DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
#message(">>> include_dirs=${dirs}") #打印一下目录情况


