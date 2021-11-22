cmake_minimum_required(VERSION 3.13)
project(evoBasicTest)

# GoogleTest requires at least C++11
set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
    cbTest
    semantic_test.cc
    codegen.cc
    ${UTILS_SOURCE}
    ${CMAKE_SOURCE_DIR}/src/compiler/ast.cpp
    ${CMAKE_SOURCE_DIR}/src/compiler/codeGen.cpp
    ${CMAKE_SOURCE_DIR}/src/compiler/context.cpp
    ${CMAKE_SOURCE_DIR}/src/compiler/ir.cpp
    ${CMAKE_SOURCE_DIR}/src/compiler/lexer.cpp
    ${CMAKE_SOURCE_DIR}/src/compiler/parser.cpp
    ${CMAKE_SOURCE_DIR}/src/compiler/semantic.cpp
    ${CMAKE_SOURCE_DIR}/src/compiler/token.cpp
    ${CMAKE_SOURCE_DIR}/src/compiler/type.cpp
    ${CMAKE_SOURCE_DIR}/src/compiler/visitor.cpp
        compileHelper.h)

target_include_directories(
    cbTest PRIVATE
    ${COMPILER_INCLUDE}
    ${VM_INCLUDE}
    ${UTILS_INCLUDE}
)

target_link_libraries(
        cbTest
        gtest_main
)

include(GoogleTest)
gtest_discover_tests(cbTest)
