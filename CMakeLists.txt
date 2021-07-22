cmake_minimum_required(VERSION 3.16)
project(evoBasic)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fno-limit-debug-info")
set(CMAKE_CXX_STANDARD 20)

add_executable(evoBasic main.cpp Lexer.cpp Lexer.h AST.cpp AST.h Parser.cpp Parser.h SymbolTable.cpp SymbolTable.h Logger.cpp Logger.h Type.cpp Type.h Semantic.cpp Semantic.h)