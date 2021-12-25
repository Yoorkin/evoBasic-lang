cmake_minimum_required(VERSION 3.13)
project(evoBasic)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-limit-debug-info -Wno-switch")

file(GLOB COMPILER_SOURCE
        token.cpp
        lexer.cpp
        parser.cpp
        type.cpp
        visitor.cpp
        context.cpp
        parseTree.cpp
        ast.cpp
        ilGen.cpp
        il.cpp
        semantic/defaultVisitor.cpp
        semantic/symbolCollector.cpp
        semantic/detailCollector.cpp
        semantic/typeAnalyzer.cpp
        semantic/semantic.cpp
        )


add_executable(ecc main.cpp ${COMPILER_SOURCE} ${UTILS_SOURCE} ../utils/enums.h)
MESSAGE(${COMPILER_SOURCE})
target_include_directories(ecc PRIVATE ${COMPILER_INCLUDE} ${UTILS_INCLUDE})