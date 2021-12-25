# What is evoBasic

EvoBasic is a "evolution version" of classic visual basic,which aims to offer simple syntax and modern features.

It is an cross-platforms,object-oriented,strongly typed programming language, compiled to bytecode and run on evoBasic virtual machine.


# features

- Data encapsulation

- Polymorphism

- Inheritance

- First-class functions

# project status

- [x] lexer and parser: generate parse tree
- [x] semantic check: type check,generate symbol table and translate parse tree to abstract tree
- [x] IL generator: translate abstract tree to intermediate language
- [ ] executable file generator
- [ ] evoBasic vm

# How to build

build requirements:
- clang or MSVC with c++20 support
- cmake
- git

```
    git clone https://github.com/Yoorkin/evoBasic-lang.git
    cd evoBasic-lang
    cmake -B"build" .
    cd build
    make
```
