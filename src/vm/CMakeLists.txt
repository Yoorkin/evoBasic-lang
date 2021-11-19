cmake_minimum_required(VERSION 3.13)
project(evoVM)

set(CMAKE_CXX_STANDARD 20)

file(GLOB VM_SOURCE ${CMAKE_SOURCE_DIR}/src/vm/*.cpp)
include_directories(${CMAKE_SOURCE_DIR}/src/vm/ src/utils)

add_executable(evm ${VM_SOURCE} ${UTILS_SOURCE})
target_include_directories(evm PRIVATE ${VM_HEADER} ${UTILS_HEADER})
