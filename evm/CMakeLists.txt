cmake_minimum_required(VERSION 3.15)

project(evm)

set(CMAKE_CXX_STANDARD 20)


if(WIN32) 
	add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/source-charset:utf-8>")
	add_compile_options("$<$<C_COMPILER_ID:MSVC>:/source-charset:utf-8>")
	set(DEPS_BIN_DIR "${CMAKE_SOURCE_DIR}/deps/bin/windows_x64")
elseif(LINUX)
	add_compile_options("$<$<CONFIG:Debug>:-g>")
	add_compile_options("$<$<CONFIG:Debug>:-O0>")
	set(DEPS_BIN_DIR "${CMAKE_SOURCE_DIR}/deps/bin/linux_64")
endif()

add_executable(evm 
main.cpp 
runtime.cpp
loader.cpp
unicode.cpp
processor.cpp
interop.cpp
backage.pb.cc 
ebffi.cpp
)

if(WIN32) 
	set_property(TARGET evm PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")
endif()

target_include_directories(evm PRIVATE "${CMAKE_SOURCE_DIR}/deps/include")

file(COPY "${DEPS_BIN_DIR}/" DESTINATION ${CMAKE_BINARY_DIR})

target_link_directories(evm PRIVATE ${DEPS_BIN_DIR})

target_link_libraries(evm PRIVATE protobuf icuuc libffi)

