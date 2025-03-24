cmake_minimum_required(VERSION 2.8.12.2)
project(BankingSystem)

# 设置 C++ 标准 (对于老版本的CMake，使用编译器标志方式)
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

# 显示当前使用的编译标志
message(STATUS "Compiler flags: ${CMAKE_CXX_FLAGS}")

# 查找 Redis 客户端库
find_library(HIREDIS_LIBRARY hiredis)
if(NOT HIREDIS_LIBRARY)
    message(FATAL_ERROR "hiredis library not found")
endif()

# 查找 Threads 库
find_package(Threads REQUIRED)

# 添加服务器源文件
set(SERVER_SRC_FILES
    ServerNWebSRC/main.cpp
    ServerNWebSRC/BankingApp.cpp
    ServerNWebSRC/AccountManager.cpp
    ServerNWebSRC/TransactionManager.cpp
    ServerNWebSRC/DepositManager.cpp
    ServerNWebSRC/HttpServer.cpp
    ServerNWebSRC/RedisClient.cpp
    ServerNWebSRC/Serializer.cpp
)

# 添加头文件路径
include_directories(${PROJECT_SOURCE_DIR}/ServerNWebINCLUDE)

# 构建服务器可执行文件
add_executable(banking_server ${SERVER_SRC_FILES})

# 链接服务器所需库
target_link_libraries(banking_server
    ${HIREDIS_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
)

# 安装服务器可执行文件
install(TARGETS banking_server DESTINATION bin)

# 安装头文件 - 如果需要提供给其他开发者使用
install(DIRECTORY ServerNWebINCLUDE/ DESTINATION include/banking)

# 添加一个选项用于构建客户端（默认关闭）
option(BUILD_CLIENT "Build the banking client" OFF)

# 如果选择构建客户端，添加客户端子目录
if(BUILD_CLIENT)
    message(STATUS "Building client application")
    add_subdirectory(client)
endif()