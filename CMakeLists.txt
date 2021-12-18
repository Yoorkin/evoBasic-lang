cmake_minimum_required(VERSION 3.13)
project(evoBasic)

set(CMAKE_CXX_STANDARD 20)

file(GLOB UTILS_SOURCE ${CMAKE_SOURCE_DIR}/src/utils/*.cpp)
file(GLOB VM_SOURCE ${CMAKE_SOURCE_DIR}/src/vm/*.cpp)

file(GLOB COMPILER_INCLUDE ${CMAKE_SOURCE_DIR}/src/compiler ${CMAKE_SOURCE_DIR}/src/compiler/semantic)
file(GLOB UTILS_INCLUDE ${CMAKE_SOURCE_DIR}/src/utils)
file(GLOB VM_INCLUDE ${CMAKE_SOURCE_DIR}/src/vm)

add_subdirectory(${CMAKE_SOURCE_DIR}/src/compiler ${CMAKE_BINARY_DIR}/ecc)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/vm ${CMAKE_BINARY_DIR}/evm)
add_subdirectory(test ${CMAKE_BINARY_DIR}/test_dir)


