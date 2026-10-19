cmake_minimum_required(VERSION 3.11)

project(log_test)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRC_LIST
    AsyncLogging.cpp
    LogStream.cpp
    Timestamp.cpp
    testLog.cpp
)

add_executable(testLog ${SRC_LIST})

target_include_directories(testLog PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(testLog PRIVATE pthread)
