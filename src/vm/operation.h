#ifndef EVOBASIC_HEADER
#define EVOBASIC_HEADER
#include "stack.h"
#include "data.h"

namespace evoBasic{
    using PC = data::Byte*;

    template<template <typename P> class Operation>
    void forEachType(data::Byte hex, vm::Stack* operand, PC& pc) {
        auto data = (Bytecode)hex;
        switch (data) {
        case Bytecode::boolean: Operation<data::boolean>::call(operand, pc); break;
        case Bytecode::i8:      Operation<data::i8>::call(operand, pc);      break;
        case Bytecode::i16:     Operation<data::i16>::call(operand, pc);     break;
        case Bytecode::i32:     Operation<data::i32>::call(operand, pc);     break;
        case Bytecode::i64:     Operation<data::i64>::call(operand, pc);     break;
        case Bytecode::f32:     Operation<data::f32>::call(operand, pc);     break;
        case Bytecode::f64:     Operation<data::f64>::call(operand, pc);     break;
        case Bytecode::u8:      Operation<data::u8>::call(operand, pc);      break;
        case Bytecode::u16:     Operation<data::u16>::call(operand, pc);     break;
        case Bytecode::u32:     Operation<data::u16>::call(operand, pc);     break;
        case Bytecode::u64:     Operation<data::u16>::call(operand, pc);     break;
        }
    }

    template<typename T>
    struct OpEQ {
        static void call(vm::Stack* operand, PC& pc) {
            T rhs = operand->pop<T>();
            T lhs = operand->pop<T>();
            operand->push<data::boolean>(rhs == lhs);
            pc += 1;
        }
    };

    template<typename T>
    struct OpNE {
        static void call(vm::Stack* operand, PC& pc) {
            T rhs = operand->pop<T>();
            T lhs = operand->pop<T>();
            operand->push<data::boolean>(rhs != lhs);
            pc += 1;
        }
    };

    template<typename T>
    struct OpLT {
        static void call(vm::Stack* operand, PC& pc) {
            T rhs = operand->pop<T>();
            T lhs = operand->pop<T>();
            operand->push<data::boolean>(rhs < lhs);
            pc += 1;
        }
    };

    template<typename T>
    struct OpGT {
        static void call(vm::Stack* operand, PC& pc) {
            T rhs = operand->pop<T>();
            T lhs = operand->pop<T>();
            operand->push<data::boolean>(rhs > lhs);
            pc += 1;
        }
    };

    template<typename T>
    struct OpLE {
        static void call(vm::Stack* operand, PC& pc) {
            T rhs = operand->pop<T>();
            T lhs = operand->pop<T>();
            operand->push<data::boolean>(rhs <= lhs);
            pc += 1;
        }
    };

    template<typename T>
    struct OpGE {
        static void call(vm::Stack* operand, PC& pc) {
            T rhs = operand->pop<T>();
            T lhs = operand->pop<T>();
            operand->push<data::boolean>(rhs >= lhs);
            pc += 1;
        }
    };

    template<typename T>
    struct OpAdd {
        static void call(vm::Stack* operand, PC& pc) {
            T rhs = operand->pop<T>();
            T lhs = operand->pop<T>();
            operand->push<T>(rhs + lhs);
            pc += 2;
        }
    };

    template<typename T>
    struct OpSub {
        static void call(vm::Stack* operand, PC& pc) {
            T rhs = operand->pop<T>();
            T lhs = operand->pop<T>();
            operand->push<T>(lhs - rhs);
            pc += 2;
        }
    };

    template<typename T>
    struct OpMul {
        static void call(vm::Stack* operand, PC& pc) {
            T rhs = operand->pop<T>();
            T lhs = operand->pop<T>();
            operand->push<T>(lhs * rhs);
            pc += 2;
        }
    };

    template<typename T>
    struct OpDiv {
        static void call(vm::Stack* operand, PC& pc) {
            T rhs = operand->pop<T>();
            T lhs = operand->pop<T>();
            operand->push<T>(lhs / rhs);
            pc += 2;
        }
    };


    template<typename T>
    struct OpFDiv {
        static void call(vm::Stack* operand, PC& pc) {
            T rhs = operand->pop<T>();
            T lhs = operand->pop<T>();
            operand->push<T>(lhs / rhs);
            pc += 2;
        }
    };

    template<typename T>
    struct OpNeg {
        static void call(vm::Stack* operand, PC& pc) {
            T rhs = operand->pop<T>();
            operand->push<T>(-rhs);
            pc += 2;
        }
    };

    template<typename T>
    struct OpLoad {
        static void call(vm::Stack* operand, PC& pc) {
            T* address = operand->pop<data::ptr>();
            operand->push<T>(*address);
            pc += 2;
        }
    };

    template<typename T>
    struct OpStore {
        static void call(vm::Stack* operand, PC& pc) {
            T value = operand->pop<T>();
            T* address = operand->pop<data::ptr>();
            *address = value;
            pc += 2;
        }
    };

    template<typename T>
    struct OpPush {
        static void call(vm::Stack* operand, PC& pc) {
            T* address = (T*)(pc + 2);
            operand->push<T>(*address);
            pc += 2 + sizeof(T);
        }
    };

    template<typename T>
    struct OpPop {
        static void call(vm::Stack* operand, PC& pc) {
            operand->pop<T>();
            pc += 2;
        }
    };

    template<typename Src>
    struct OpCast {
        static void call(vm::Stack* operand, PC& pc) {
            Src value = operand->pop<data::ptr>();
            switch (Bytecode(*(pc + 2))) {
            case Bytecode::boolean:
                operand->push<data::boolean>((data::boolean)value);
                break;
            case Bytecode::i8:
                operand->push<data::boolean>((data::boolean)value);
                break;
            case Bytecode::i16:
                operand->push<data::boolean>((data::boolean)value);
                break;
            case Bytecode::i32:
                operand->push<data::boolean>((data::boolean)value);
                break;
            case Bytecode::i64:
                operand->push<data::boolean>((data::boolean)value);
                break;
            case Bytecode::f32:
                operand->push<data::boolean>((data::boolean)value);
                break;
            case Bytecode::f64:
                operand->push<data::boolean>((data::boolean)value);
                break;
            case Bytecode::u8:
                operand->push<data::boolean>((data::boolean)value);
                break;
            case Bytecode::u16:
                operand->push<data::boolean>((data::boolean)value);
                break;
            case Bytecode::u32:
                operand->push<data::boolean>((data::boolean)value);
                break;
            case Bytecode::u64:
                operand->push<data::boolean>((data::boolean)value);
                break;
            }
            pc += 3;
        }
    };
}

#endif