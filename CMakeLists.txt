cmake_minimum_required(VERSION 3.13)
project(evoBasic)

set(CMAKE_CXX_STANDARD 20)

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("-Wno-everything")


set(EVO_SHARE_SOURCE_DIR "${CMAKE_SOURCE_DIR}/src/share")
set(EVO_LIB_DIR  "${CMAKE_BINARY_DIR}/lib")
set(EVO_BIN_DIR "${CMAKE_BINARY_DIR}/bin")

include_directories(${EVO_SHARE_SOURCE_DIR})
add_subdirectory(${CMAKE_SOURCE_DIR}/src/ecc ${CMAKE_BINARY_DIR}/ecc)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/evm ${CMAKE_BINARY_DIR}/vm)
# add_subdirectory(test ${CMAKE_BINARY_DIR}/test_dir)


link_directories(${EVO_LIB_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${EVO_LIB_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${EVO_BIN_DIR})


