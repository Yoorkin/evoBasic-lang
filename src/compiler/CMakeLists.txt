cmake_minimum_required(VERSION 3.13)
project(evoBasic)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-limit-debug-info -Wno-switch")

add_executable(ecc ${COMPILER_SOURCE} ${UTILS_SOURCE} semantic/defaultVisitor.cpp semantic/defaultVisitor.h il.cpp il.h ilGen.cpp ilGen.h ../utils/i18n.h)
MESSAGE(${COMPILER_SOURCE})
target_include_directories(ecc PRIVATE ${COMPILER_INCLUDE} ${UTILS_INCLUDE})