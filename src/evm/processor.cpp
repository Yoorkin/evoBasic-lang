//
// Created by yorkin on 1/14/22.
//

#include "processor.h"
#include <loader/bytecode.h>
namespace evoBasic::vm{

    template<template <typename P> class Operation>
    void forEachType(vm::Stack &operand, ExecutionEnv &env) {
        auto data = env.template consume<Bytecode>();
        switch (data) {
            case Bytecode::boolean: Operation<data::boolean>::call(operand, env); break;
            case Bytecode::i8:      Operation<data::i8>::call(operand, env);      break;
            case Bytecode::i16:     Operation<data::i16>::call(operand, env);     break;
            case Bytecode::i32:     Operation<data::i32>::call(operand, env);     break;
            case Bytecode::i64:     Operation<data::i64>::call(operand, env);     break;
            case Bytecode::f32:     Operation<data::f32>::call(operand, env);     break;
            case Bytecode::f64:     Operation<data::f64>::call(operand, env);     break;
            case Bytecode::u8:      Operation<data::u8>::call(operand, env);      break;
            case Bytecode::u16:     Operation<data::u16>::call(operand, env);     break;
            case Bytecode::u32:     Operation<data::u16>::call(operand, env);     break;
            case Bytecode::u64:     Operation<data::u16>::call(operand, env);     break;
        }
    }

    template<typename T>
    struct OpEQ {
        static void call(Stack &operand, ExecutionEnv &env) {
            T rhs = operand.pop<T>();
            T lhs = operand.pop<T>();
            operand.push<data::boolean>(rhs == lhs);
        }
    };

    template<typename T>
    struct OpNE {
        static void call(Stack &operand, ExecutionEnv &env) {
            T rhs = operand.pop<T>();
            T lhs = operand.pop<T>();
            operand.push<data::boolean>(rhs != lhs);
        }
    };

    template<typename T>
    struct OpLT {
        static void call(Stack &operand, ExecutionEnv &env) {
            T rhs = operand.pop<T>();
            T lhs = operand.pop<T>();
            operand.push<data::boolean>(rhs < lhs);
        }
    };

    template<typename T>
    struct OpGT {
        static void call(Stack &operand, ExecutionEnv &env) {
            T rhs = operand.pop<T>();
            T lhs = operand.pop<T>();
            operand.push<data::boolean>(rhs > lhs);
        }
    };

    template<typename T>
    struct OpLE {
        static void call(Stack &operand, ExecutionEnv &env) {
            T rhs = operand.pop<T>();
            T lhs = operand.pop<T>();
            operand.push<data::boolean>(rhs <= lhs);
        }
    };

    template<typename T>
    struct OpGE {
        static void call(Stack &operand, ExecutionEnv &env) {
            T rhs = operand.pop<T>();
            T lhs = operand.pop<T>();
            operand.push<data::boolean>(rhs >= lhs);
        }
    };

    template<typename T>
    struct OpAdd {
        static void call(Stack &operand, ExecutionEnv &env) {
            T rhs = operand.pop<T>();
            T lhs = operand.pop<T>();
            operand.push<T>(rhs + lhs);
        }
    };

    template<typename T>
    struct OpSub {
        static void call(Stack &operand, ExecutionEnv &env) {
            T rhs = operand.pop<T>();
            T lhs = operand.pop<T>();
            operand.push<T>(lhs - rhs);
        }
    };

    template<typename T>
    struct OpMul {
        static void call(Stack &operand, ExecutionEnv &env) {
            T rhs = operand.pop<T>();
            T lhs = operand.pop<T>();
            operand.push<T>(lhs * rhs);
        }
    };

    template<typename T>
    struct OpDiv {
        static void call(Stack &operand, ExecutionEnv &env) {
            T rhs = operand.pop<T>();
            T lhs = operand.pop<T>();
            operand.push<T>(lhs / rhs);
        }
    };


    template<typename T>
    struct OpFDiv {
        static void call(Stack &operand, ExecutionEnv &env) {
            T rhs = operand.pop<T>();
            T lhs = operand.pop<T>();
            operand.push<T>(lhs / rhs);
        }
    };

    template<typename T>
    struct OpNeg {
        static void call(Stack &operand, ExecutionEnv &env) {
            T rhs = operand.pop<T>();
            operand.push<T>(-rhs);
        }
    };

    template<typename T>
    struct OpLoad {
        static void call(Stack &operand, ExecutionEnv &env) {
            T* address = operand.pop<T*>();
            operand.push<T>(*address);
        }
    };

    template<typename T>
    struct OpStore {
        static void call(Stack &operand, ExecutionEnv &env) {
            T value = operand.pop<T>();
            T* address = operand.pop<data::ptr>();
            *address = value;
        }
    };

    template<typename T>
    struct OpPush {
        static void call(Stack &operand, ExecutionEnv &env) {
            auto value = env.template consume<T>();
            operand.push<T>(value);
        }
    };

    template<typename T>
    struct OpPop {
        static void call(Stack &operand, ExecutionEnv &env) {
            operand.pop<T>();
        }
    };

    template<typename T>
    struct OpDup{
        static void call(Stack &operand, ExecutionEnv &env){
            operand.dup<T>();
        }
    };

    template<typename T>
    struct OpLdarg{
        static void call(Stack &operand, ExecutionEnv &env){
            auto index = operand.pop<data::u16>();
            auto offset = env.getFunction()->getParamOffset(index);
            auto value = env.getFrame().read<T>(offset);
            operand.push(value);
        }
    };

    template<typename T>
    struct OpLdargr{
        static void call(Stack &operand, ExecutionEnv &env){
            auto index = operand.pop<data::u16>();
            auto offset = env.getFunction()->getParamOffset(index);
            auto address = env.getFrame().read<data::address>(offset);
            auto value = *reinterpret_cast<T*>(address);
            operand.push(value);
        }
    };

    template<typename T>
    struct OpStargr{
        static void call(Stack &operand, ExecutionEnv &env){
            auto value = operand.pop<T>();
            auto index = operand.pop<data::u16>();
            auto offset = env.getFunction()->getParamOffset(index);
            auto address = env.getFrame().read<data::address>(offset);
            *reinterpret_cast<T*>(address) = value;
        }
    };

    template<typename T>
    struct OpStarg{
        static void call(Stack &operand, ExecutionEnv &env){
            auto value = operand.pop<T>();
            auto index = operand.pop<data::u16>();
            auto offset = env.getFunction()->getParamOffset(index);
            env.getFrame().write<T>(offset,value);
        }
    };

    template<typename T>
    struct OpLdloc{
        static void call(Stack &operand, ExecutionEnv &env){
            auto index = operand.pop<data::u16>();
            auto offset = env.getFunction()->getLocalOffset(index);
            auto value = env.getFrame().read<T>(offset);
            operand.push(value);
        }
    };

    template<typename T>
    struct OpStloc{
        static void call(Stack &operand, ExecutionEnv &env){
            auto value = operand.pop<T>();
            auto index = operand.pop<data::u16>();
            auto offset = env.getFunction()->getLocalOffset(index);
            env.getFrame().write<T>(offset,value);
        }
    };

    template<typename T>
    struct OpLdfld{
        static void call(Stack &operand, ExecutionEnv &env){
            auto obj = operand.pop<ClassInstance*>();
            auto token_id = env.consume<il::TokenDef::ID>();
            auto field_slot = env.getFunction()->getTokenTable()->getRuntime<FieldSlot>(token_id);
            auto value = obj->read<T>(field_slot);
            operand.push(value);
        }
    };


    template<typename T>
    struct OpLdsfld{
        static void call(Stack &operand, ExecutionEnv &env){
            auto obj = operand.pop<ClassInstance*>();
            auto token_id = env.consume<il::TokenDef::ID>();
            auto field_slot = env.getFunction()->getTokenTable()->getRuntime<StaticFieldSlot>(token_id);
            auto value = obj->getClass()->read<T>(field_slot);
            operand.push(value);
        }
    };

    template<typename T>
    struct OpStfld{
        static void call(Stack &operand, ExecutionEnv &env){
            auto value = operand.pop<T>();
            auto obj = operand.pop<ClassInstance*>();
            auto token_id = env.consume<il::TokenDef::ID>();
            auto field_slot = env.getFunction()->getTokenTable()->getRuntime<FieldSlot>(token_id);
            obj->write<T>(field_slot,value);
        }
    };

    template<typename T>
    struct OpStsfld{
        static void call(Stack &operand, ExecutionEnv &env){
            auto value = operand.pop<T>();
            auto obj = operand.pop<ClassInstance*>();
            auto token_id = env.consume<il::TokenDef::ID>();
            auto field_slot = env.getFunction()->getTokenTable()->getRuntime<StaticFieldSlot>(token_id);
            obj->getClass()->write<T>(field_slot,value);
        }
    };

    template<typename Src>
    struct OpConv {
        static void call(Stack &operand, ExecutionEnv &env) {
            Src value = operand.pop<Src>();
            switch (env.template consume<Bytecode>()) {
                case Bytecode::boolean:
                    operand.push<data::boolean>(value);
                    break;
                case Bytecode::i8:
                    operand.push<data::i8>(value);
                    break;
                case Bytecode::i16:
                    operand.push<data::i16>(value);
                    break;
                case Bytecode::i32:
                    operand.push<data::i32>(value);
                    break;
                case Bytecode::i64:
                    operand.push<data::i64>(value);
                    break;
                case Bytecode::f32:
                    operand.push<data::f32>(value);
                    break;
                case Bytecode::f64:
                    operand.push<data::f64>(value);
                    break;
                case Bytecode::u8:
                    operand.push<data::u8>(value);
                    break;
                case Bytecode::u16:
                    operand.push<data::u16>(value);
                    break;
                case Bytecode::u32:
                    operand.push<data::u32>(value);
                    break;
                case Bytecode::u64:
                    operand.push<data::u64>(value);
                    break;
            }
        }
    };


    Processor::Processor(RuntimeContext *context,Function *function, size_t operand_size, size_t frame_size)
        : context(context),operand(operand_size),frame(frame_size){
        execution_stack.push(ExecutionEnv(function,frame.borrow(0)));
        frame_top += function->getStackFrameLength();
    }

    void Processor::run() {
        bool running = true;
        while(running){
            switch (getCurrentEnv().consume<Bytecode>()) {
                case Bytecode::Nop:
                    break;
                case Bytecode::Ret:
                    frame_top -= getCurrentEnv().getFunction()->getStackFrameLength();
                    execution_stack.pop();
                    if(execution_stack.empty())running = false;
                    break;
                case Bytecode::Callstatic:{
                    auto function = getCurrentEnv().consume<Function*>();
                    execution_stack.push(ExecutionEnv(function,frame.borrow(frame_top)));
                    frame_top += function->getStackFrameLength();
                    break;
                }
                case Bytecode::Call:{
                    auto function = getCurrentEnv().consume<Function*>();
                    auto ref = operand.top<ClassInstance*>();
                    execution_stack.push(ExecutionEnv(function,frame.borrow(frame_top)));
                    frame_top += function->getStackFrameLength();
                    break;
                }
                case Bytecode::CallVirt:{
                    auto slot = getCurrentEnv().consume<VirtualFtnSlot>();
                    auto ref = operand.top<ClassInstance*>();
                    auto function = ref->getClass()->virtualFunctionDispatch(slot);
                    execution_stack.push(ExecutionEnv(function,frame.borrow(frame_top)));
                    frame_top += function->getStackFrameLength();
                    break;
                }
                case Bytecode::Ldnull:
                    PANIC;
                    break;
                case Bytecode::And:
                    operand.push<data::boolean>(operand.pop<data::boolean>() && operand.pop<data::boolean>());
                    break;
                case Bytecode::Or:
                    operand.push<data::boolean>(operand.pop<data::boolean>() || operand.pop<data::boolean>());
                    break;
                case Bytecode::Xor:
                    operand.push<data::boolean>(operand.pop<data::boolean>() xor operand.pop<data::boolean>());
                    break;
                case Bytecode::Ldloca:{
                    auto index = operand.pop<data::u16>();
                    auto offset = getCurrentEnv().getFunction()->getLocalOffset(index);
                    auto address = getCurrentEnv().getFrame().address<char>(offset);
                    operand.push(address);
                    break;
                }
                case Bytecode::Ldarga:{
                    auto index = operand.pop<data::u16>();
                    auto offset = getCurrentEnv().getFunction()->getParamOffset(index);
                    auto address = getCurrentEnv().getFrame().address<char>(offset);
                    operand.push(address);
                    break;
                }
                case Bytecode::Ldarg:
                    forEachType<OpLdarg>(operand,getCurrentEnv());
                    break;
                case Bytecode::Ldargr:
                    forEachType<OpLdargr>(operand,getCurrentEnv());
                    break;
                case Bytecode::Starg:
                    forEachType<OpLdarg>(operand,getCurrentEnv());
                    break;
                case Bytecode::Stargr:
                    forEachType<OpStargr>(operand,getCurrentEnv());
                    break;
                case Bytecode::Ldloc:
                    forEachType<OpLdloc>(operand,getCurrentEnv());
                    break;
                case Bytecode::Stloc:
                    forEachType<OpStloc>(operand,getCurrentEnv());
                    break;
                case Bytecode::Ldelema:
                    PANIC;
                    break;
                case Bytecode::Not:
                    operand.push<data::boolean>(!operand.pop<data::boolean>());
                    break;
                case Bytecode::Ldvftn:{
                    getCurrentEnv().consume<data::Byte>();
                    auto id = getCurrentEnv().consume<il::TokenDef::ID>();
                    auto slot = getCurrentEnv().getFunction()->getTokenTable()->getRuntime<VirtualFtnSlot>(id);
                    operand.push(slot);
                    break;
                }
                case Bytecode::Ldsftn:{
                    getCurrentEnv().consume<data::Byte>();
                    auto id = getCurrentEnv().consume<il::TokenDef::ID>();
                    auto ftn = getCurrentEnv().getFunction()->getTokenTable()->getRuntime<Function>(id);
                    operand.push(ftn);
                    break;
                }
                case Bytecode::Ldftn:{
                    getCurrentEnv().consume<data::Byte>();
                    auto id = getCurrentEnv().consume<il::TokenDef::ID>();
                    auto ftn = getCurrentEnv().getFunction()->getTokenTable()->getRuntime<Function>(id);
                    operand.push(ftn);
                    break;
                }
                case Bytecode::Ldc:
                    PANIC;
                    break;
                case Bytecode::Newobj:
                    break;
                case Bytecode::Invoke:{
                    getCurrentEnv().consume<data::Byte>();
                    auto token_id = getCurrentEnv().consume<il::TokenDef::ID>();
                    auto rt = getCurrentEnv().getFunction()->getTokenTable()->getRuntime<Runtime>(token_id);
                    if(rt->getKind() == RuntimeKind::ForeignFunction){
                        //todo
                    }
                    else{
                        static_cast<Intrinsic*>(rt)->invoke(&operand);
                    }
                    break;
                }
                case Bytecode::Ldflda:{
                    auto obj = operand.pop<ClassInstance*>();
                    auto token_id = getCurrentEnv().consume<il::TokenDef::ID>();
                    auto field_slot = getCurrentEnv().getFunction()->getTokenTable()->getRuntime<FieldSlot>(token_id);
                    auto address = obj->address<data::Byte*>(field_slot);
                    operand.push(address);
                    break;
                }
                case Bytecode::Ldsflda:{
                    auto obj = operand.pop<ClassInstance*>();
                    auto token_id = getCurrentEnv().consume<il::TokenDef::ID>();
                    auto field_slot = getCurrentEnv().getFunction()->getTokenTable()->getRuntime<StaticFieldSlot>(token_id);
                    auto address = obj->getClass()->address<data::Byte*>(field_slot);
                    operand.push(address);
                    break;
                }
                case Bytecode::Ldfld:
                    forEachType<OpLdfld>(operand, getCurrentEnv());
                    break;
                case Bytecode::Ldsfld:
                    forEachType<OpLdsfld>(operand, getCurrentEnv());
                    break;
                case Bytecode::Stfld:
                    forEachType<OpStfld>(operand, getCurrentEnv());
                    break;
                case Bytecode::Stsfld:
                    forEachType<OpStsfld>(operand, getCurrentEnv());
                    break;
                case Bytecode::Ldelem:
                    PANIC;
                    break;
                case Bytecode::Stelem:
                    PANIC;
                    break;
                case Bytecode::Stelema:
                    PANIC;
                    break;
                case Bytecode::Add:
                    forEachType<OpAdd>(operand, getCurrentEnv());
                    break;
                case Bytecode::Sub:
                    forEachType<OpSub>(operand, getCurrentEnv());
                    break;
                case Bytecode::Mul:
                    forEachType<OpMul>(operand, getCurrentEnv());
                    break;
                case Bytecode::Div:
                    forEachType<OpDiv>(operand, getCurrentEnv());
                    break;
                case Bytecode::FDiv:
                    forEachType<OpFDiv>(operand, getCurrentEnv());
                    break;
                case Bytecode::EQ:
                    forEachType<OpEQ>(operand, getCurrentEnv());
                    break;
                case Bytecode::NE:
                    forEachType<OpNE>(operand, getCurrentEnv());
                    break;
                case Bytecode::LT:
                    forEachType<OpLT>(operand, getCurrentEnv());
                    break;
                case Bytecode::GT:
                    forEachType<OpGT>(operand, getCurrentEnv());
                    break;
                case Bytecode::LE:
                    forEachType<OpLE>(operand, getCurrentEnv());
                    break;
                case Bytecode::GE:
                    forEachType<OpGE>(operand, getCurrentEnv());
                    break;
                case Bytecode::Neg:
                    forEachType<OpNeg>(operand, getCurrentEnv());
                    break;
                case Bytecode::Pop:
                    forEachType<OpPop>(operand, getCurrentEnv());
                    break;
                case Bytecode::Dup:
                    forEachType<OpDup>(operand, getCurrentEnv());
                    break;
                case Bytecode::Jif:{
                    auto accept = operand.pop<data::boolean>();
                    auto target = getCurrentEnv().consume<data::u64>();
                    if(accept) getCurrentEnv().jump(target);
                    break;
                }
                case Bytecode::Br:{
                    auto target = getCurrentEnv().consume<data::u64>();
                    getCurrentEnv().jump(target);
                    break;
                }
                case Bytecode::Push:
                    forEachType<OpPush>(operand, getCurrentEnv());
                    break;
                case Bytecode::CastCls:
                    PANIC;
                    break;
                case Bytecode::Conv:
                    forEachType<OpConv>(operand, getCurrentEnv());
                    break;
            }
        }
    }

    ExecutionEnv &Processor::getCurrentEnv() {
        return execution_stack.top();
    };


    ExecutionEnv::ExecutionEnv(Function *function, Memory frame)
        : function(function),pc(function->getBlock()),frame(frame) {}
}
