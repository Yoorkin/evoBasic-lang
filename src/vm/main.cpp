//
// Created by yorkin on 11/19/21.
//
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <fstream>
#include <filesystem>
#include <functional>
#include <map>
#include <stack>
#include "cmd.h"
#include "logger.h"
#include "formator.h"
#include "stack.h"
#include "bytecode.h"
using namespace std;
using namespace evoBasic;
namespace fs = std::filesystem;
using namespace evoBasic::vm;

Stack operand(1024);
Stack callstack(2048);
char *codes = nullptr;
data::ptr pc = 0;
stack<data::ptr> prv_address;

template<template <typename P> class Operation>
void forEachType(data::u8 hex){
    auto data = vm::Data::fromHex(hex);
    switch (data.getValue()) {
        case Data::boolean: Operation<data::boolean>();
        case Data::i8:      Operation<data::i8>();
        case Data::i16:     Operation<data::i16>();
        case Data::i32:     Operation<data::i32>();
        case Data::i64:     Operation<data::i64>();
        case Data::f32:     Operation<data::f32>();
        case Data::f64:     Operation<data::f64>();
        case Data::u8:      Operation<data::u8>();
        case Data::u16:     Operation<data::u16>();
        case Data::u32:     Operation<data::u16>();
        case Data::u64:     Operation<data::u16>();
    }
}

template<typename T>
struct OpEQ{
    void operator()(){
        T rhs = operand.pop<T>();
        T lhs = operand.pop<T>();
        operand.push<data::boolean>(rhs==lhs);
        pc += 1;
    }
};

template<typename T>
struct OpNE{
    void operator()(){
        T rhs = operand.pop<T>();
        T lhs = operand.pop<T>();
        operand.push<data::boolean>(rhs!=lhs);
        pc += 1;
    }
};

template<typename T>
struct OpLT{
    void operator()(){
        T rhs = operand.pop<T>();
        T lhs = operand.pop<T>();
        operand.push<data::boolean>(rhs<lhs);
        pc += 1;
    }
};

template<typename T>
struct OpGT{
    void operator()(){
        T rhs = operand.pop<T>();
        T lhs = operand.pop<T>();
        operand.push<data::boolean>(rhs>lhs);
        pc += 1;
    }
};

template<typename T>
struct OpLE{
    void operator()(){
        T rhs = operand.pop<T>();
        T lhs = operand.pop<T>();
        operand.push<data::boolean>(rhs<=lhs);
        pc += 1;
    }
};

template<typename T>
struct OpGE{
    void operator()(){
        T rhs = operand.pop<T>();
        T lhs = operand.pop<T>();
        operand.push<data::boolean>(rhs>=lhs);
        pc += 1;
    }
};

template<typename T>
struct OpAdd{
    void operator()(){
        T rhs = operand.pop<T>();
        T lhs = operand.pop<T>();
        operand.push<T>(rhs+lhs);
        pc += 2;
    }
};

template<typename T>
struct OpSub{
    void operator()(){
        T rhs = operand.pop<T>();
        T lhs = operand.pop<T>();
        operand.push<T>(lhs-rhs);
        pc += 2;
    }
};

template<typename T>
struct OpMul{
    void operator()(){
        T rhs = operand.pop<T>();
        T lhs = operand.pop<T>();
        operand.push<T>(lhs*rhs);
        pc += 2;
    }
};

template<typename T>
struct OpDiv{
    void operator()(){
        T rhs = operand.pop<T>();
        T lhs = operand.pop<T>();
        operand.push<T>(lhs/rhs);
        pc += 2;
    }
};


template<typename T>
struct OpFDiv{
    void operator()(){
        T rhs = operand.pop<T>();
        T lhs = operand.pop<T>();
        operand.push<T>(lhs/rhs);
        pc += 2;
    }
};

template<typename T>
struct OpNeg{
    void operator()(){
        T rhs = operand.pop<T>();
        operand.push<T>(-rhs);
        pc += 2;
    }
};

template<typename T>
struct OpLoad{
    void operator()(){
        T *address = operand.pop<data::ptr>();
        operand.push<T>(*address);
        pc += 2;
    }
};

template<typename T>
struct OpStore{
    void operator()(){
        T value = operand.pop<T>();
        T *address = operand.pop<data::ptr>();
        *address = value;
        pc += 2;
    }
};

template<typename T>
struct OpPush{
    void operator()(){
        T *address = (T*)(codes + pc + 2);
        operand.push<T>(*address);
        pc += 2 + sizeof(T);
    }
};

template<typename T>
struct OpPop{
    void operator()(){
        operand.pop<T>();
        pc += 2;
    }
};

template<typename Src>
struct OpCast{
    void operator()(){
        Src value = operand.pop<data::ptr>();
        switch(vm::Data::fromHex(*(codes + pc + 2)).getValue()){
            case Data::boolean:
                operand.push<data::boolean>((data::boolean)value);
                break;
            case Data::i8:
                operand.push<data::boolean>((data::boolean)value);
                break;
            case Data::i16:
                operand.push<data::boolean>((data::boolean)value);
                break;
            case Data::i32:
                operand.push<data::boolean>((data::boolean)value);
                break;
            case Data::i64:
                operand.push<data::boolean>((data::boolean)value);
                break;
            case Data::f32:
                operand.push<data::boolean>((data::boolean)value);
                break;
            case Data::f64:
                operand.push<data::boolean>((data::boolean)value);
                break;
            case Data::u8:
                operand.push<data::boolean>((data::boolean)value);
                break;
            case Data::u16:
                operand.push<data::boolean>((data::boolean)value);
                break;
            case Data::u32:
                operand.push<data::boolean>((data::boolean)value);
                break;
            case Data::u64:
                operand.push<data::boolean>((data::boolean)value);
                break;
        }
        pc += 3;
    }
};

template<typename T>
struct OpDup{
    void operator()(){
        operand.dup<T>();
        pc += 2;
    }
};

void loadAndRun(fstream stream){

    data::u8 hex = stream.get();
    data::u64 pc = 0;
    if(Bytecode::fromHex(hex).getValue() == Bytecode::Entrance){
        stream.read((char*)&pc,sizeof(pc));
    }

    do hex = stream.get();
    while(hex != Bytecode(Bytecode::ConstSegment).toHex());
    stream.get();

    codes = (char*)malloc(2048);
    auto ptr = codes;
    while(!stream.eof()){
        *ptr = stream.get();
        ptr++;
    }

    while(true){
        switch (codes[pc]-0x0F) {
            case Bytecode::Entrance:
                break;
            case Bytecode::MetaSegment:
                break;
            case Bytecode::ConstSegment:
                break;
            case Bytecode::CodeSegment:
                break;
            case Bytecode::Define:
                break;
            case Bytecode::Jmp:
                pc = *((data::ptr*)(codes + pc + 1));
                pc += 9;
                break;
            case Bytecode::Jif:
                if(operand.top<data::boolean>()) {
                    pc = *((data::ptr*)(codes + pc + 1));
                }
                operand.pop<data::boolean>();
                pc += 9;
                break;
            case Bytecode::EQ:
                forEachType<OpEQ>(*(codes + pc + 1));
                break;
            case Bytecode::NE:
                forEachType<OpNE>(*(codes + pc + 1));
                break;
            case Bytecode::LT:
                forEachType<OpLT>(*(codes + pc + 1));
                break;
            case Bytecode::GT:
                forEachType<OpGT>(*(codes + pc + 1));
                break;
            case Bytecode::LE:
                forEachType<OpLE>(*(codes + pc + 1));
                break;
            case Bytecode::GE:
                forEachType<OpGE>(*(codes + pc + 1));
                break;
            case Bytecode::Add:
                forEachType<OpAdd>(*(codes + pc + 1));
                break;
            case Bytecode::Sub:
                forEachType<OpSub>(*(codes + pc + 1));
                break;
            case Bytecode::Mul:
                forEachType<OpMul>(*(codes + pc + 1));
                break;
            case Bytecode::Div:
                forEachType<OpDiv>(*(codes + pc + 1));
                break;
            case Bytecode::FDiv:
                forEachType<OpFDiv>(*(codes + pc + 1));
                break;
            case Bytecode::Neg:
                forEachType<OpNeg>(*(codes + pc + 1));
                break;
            case Bytecode::And:
                operand.push<data::boolean>(operand.pop<data::boolean>() and operand.pop<data::boolean>());
                break;
            case Bytecode::Or:
                operand.push<data::boolean>(operand.pop<data::boolean>() or operand.pop<data::boolean>());
                break;
            case Bytecode::Xor:
                operand.push<data::boolean>(operand.pop<data::boolean>() xor operand.pop<data::boolean>());
                break;
            case Bytecode::Not:
                operand.push<data::boolean>(!operand.pop<data::boolean>());
                break;
            case Bytecode::Load:
                forEachType<OpLoad>(*(codes + pc + 1));
                break;
            case Bytecode::Store:
                forEachType<OpStore>(*(codes + pc + 1));
                break;
            case Bytecode::Push:
                forEachType<OpPush>(*(codes + pc + 1));
                break;
            case Bytecode::Pop:
                forEachType<OpPop>(*(codes + pc + 1));
                break;
            case Bytecode::Ret:
                pc = prv_address.top();
                prv_address.pop();
                if(pc == -1)return;
                break;
            case Bytecode::Invoke:
                prv_address.push(pc);
                pc = *((data::ptr*)(codes + pc + 1));
                break;
            case Bytecode::Cast:
                forEachType<OpCast>(*(codes + pc + 1));
                break;
            case Bytecode::Dup:
                forEachType<OpDup>(*(codes + pc + 1));
                break;
            case Bytecode::Stm:
                break;
            case Bytecode::Ldm:
                break;
            case Bytecode::Psm:
                break;
            case Bytecode::PushFrameBase:
                operand.push<data::ptr>(prv_address.top());
                pc += 1;
                break;
            case Bytecode::PushGlobalBase:
                break;
            case Bytecode::Nop:
                pc += 1;
                break;
        }
    }
}

int main(int argc,char *argv[]){
    vector<fs::path> files;

    CmdDistributor distributor;
    distributor.others([&](const string &file){
        if(fs::exists(file)) files.emplace_back(file);
        else Logger::error(format()<<"cannot find file '"<<file<<"'");
    });

    for(int i=1;i<argc;i++){
        distributor.distribute(argv[i]);
    }

    loadAndRun(files[0]);
}

