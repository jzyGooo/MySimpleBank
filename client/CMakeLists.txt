cmake_minimum_required(VERSION 2.8.12.2)
project(BankingClient)

# 设置C++标准
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# 查找libcurl包
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})

# 包含项目头文件目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/extern)

# 收集所有源文件
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
)

# 创建可执行文件
add_executable(banking_client ${SOURCES})

# 链接库
target_link_libraries(banking_client ${CURL_LIBRARIES})

# 需要安装nlohmann/json到extern目录下
# 如果还没有，可以添加下载命令
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/extern/json.hpp")
    message(STATUS "Downloading nlohmann/json...")
    file(DOWNLOAD
        "https://github.com/nlohmann/json/releases/download/v3.11.2/json.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/extern/json.hpp"
        SHOW_PROGRESS
    )
endif()