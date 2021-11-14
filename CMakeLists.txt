cmake_minimum_required(VERSION 3.13)
project(evoBasic)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -fno-limit-debug-info")
set(CMAKE_CXX_STANDARD 20)

include_directories(src/frontend src/utils src/vm)

file(GLOB FRONTEND_SOURCE ${CMAKE_SOURCE_DIR}/src/frontend/*)
file(GLOB UTILS_SOURCE ${CMAKE_SOURCE_DIR}/src/utils/*)
file(GLOB VM_SOURCE ${CMAKE_SOURCE_DIR}/src/vm/*)

add_subdirectory(test ${CMAKE_BINARY_DIR}/test_dir)

add_executable(evoBasic ${FRONTEND_SOURCE} ${UTILS_SOURCE} ${VM_SOURCE} src/main.cpp)
target_compile_options(evoBasic PRIVATE -Wall)