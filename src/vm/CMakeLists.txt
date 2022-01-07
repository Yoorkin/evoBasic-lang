cmake_minimum_required(VERSION 3.13)
project(evoVM)

set(CMAKE_CXX_STANDARD 20)

file(GLOB VM_SOURCE
    interpreter.cpp
    intrinsic.cpp
    invoke.cpp
    stack.cpp
)

message(STATUS compiler srouces:${COMPILER_SOURCE} ${VM_SOURCE} ${UTILS_SOURCE})

add_executable(evm ${VM_SOURCE} ${UTILS_SOURCE} ${COMPILER_SOURCE} main.cpp)
target_include_directories(evm PRIVATE ${VM_INCLUDE} ${UTILS_INCLUDE} ${COMPILER_INCLUDE})
