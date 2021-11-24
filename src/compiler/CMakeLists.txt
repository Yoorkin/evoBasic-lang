cmake_minimum_required(VERSION 3.13)
project(evoBasic)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -fno-limit-debug-info")

add_executable(ecc ${COMPILER_SOURCE} ${UTILS_SOURCE})
MESSAGE(${COMPILER_SOURCE})
target_include_directories(ecc PRIVATE ${COMPILER_INCLUDE} ${UTILS_INCLUDE})