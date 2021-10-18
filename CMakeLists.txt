cmake_minimum_required(VERSION 3.13)
project(evoBasic)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -fno-limit-debug-info")
set(CMAKE_CXX_STANDARD 20)

file(GLOB FRONTEND_SOURCE ${CMAKE_SOURCE_DIR}/src/frontend/*)
file(GLOB UTILS_SOURCE ${CMAKE_SOURCE_DIR}/src/utils/*)
file(GLOB INTERPRETER_SOURCE ${CMAKE_SOURCE_DIR}/src/interpreter/*)

# add_subdirectory(test ${CMAKE_BINARY_DIR}/test_dir)

add_executable(evoBasic ${FRONTEND_SOURCE} ${UTILS_SOURCE} ${INTERPRETER_SOURCE} src/main.cpp src/interpreter/Intrinsics.cpp src/interpreter/Intrinsics.h src/interpreter/Interpreter.cpp src/interpreter/Interpreter.h)