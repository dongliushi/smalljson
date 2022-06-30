project(smalljson)

cmake_minimum_required(VERSION 3.24)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(smalljson)

include_directories(${PROJECT_SOURCE_DIR}/smalljson)

add_executable(test 1.cc)

target_link_libraries(test PUBLIC smalljson gtest pthread)
