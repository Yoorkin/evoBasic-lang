//
// Created by yorkin on 1/14/22.
//

#include "processor.h"
#include <loader/bytecode.h>
namespace evoBasic::vm{

    template<template <typename P> class Operation>
    void forEachScalarType(vm::Stack &operand, ExecutionEnv &env) {
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
            case Bytecode::u32:     Operation<data::u32>::call(operand, env);     break;
            case Bytecode::u64:     Operation<data::u64>::call(operand, env);     break;
            case Bytecode::ref:     Operation<data::address>::call(operand,env);  break;
            case Bytecode::array:
            case Bytecode::record:
                PANIC;
        }
    }


    using OperationHandler = std::function<void(Stack&,ExecutionEnv&,data::u64)>;
    void forEachType(OperationHandler handler,vm::Stack &operand, ExecutionEnv &env){
        auto data = env.template consume<Bytecode>();
        switch(data){
            case Bytecode::boolean: handler(operand,env,sizeof(data::boolean)); break;
            case Bytecode::i8:      handler(operand,env,sizeof(data::i8));      break;
            case Bytecode::i16:     handler(operand,env,sizeof(data::i16));     break;
            case Bytecode::i32:     handler(operand,env,sizeof(data::i32));     break;
            case Bytecode::i64:     handler(operand,env,sizeof(data::i64));     break;
            case Bytecode::f32:     handler(operand,env,sizeof(data::f32));     break;
            case Bytecode::f64:     handler(operand,env,sizeof(data::f64));     break;
            case Bytecode::u8:      handler(operand,env,sizeof(data::u8));      break;
            case Bytecode::u16:     handler(operand,env,sizeof(data::u16));     break;
            case Bytecode::u32:     handler(operand,env,sizeof(data::u32));     break;
            case Bytecode::u64:     handler(operand,env,sizeof(data::u64));     break;
            case Bytecode::ref:     handler(operand,env,sizeof(data::Byte*));   break;
            case Bytecode::array:{
                env.template consume<data::Byte>();
                auto id = env.template consume<il::TokenDef::ID>();
                auto array = env.getFunction()->getTokenTable()->template getRuntime<Array>(id);
                handler(operand,env,array->getByteLength());
                break;
            }
            case Bytecode::record:{
                env.template consume<data::Byte>();
                auto id = env.template consume<il::TokenDef::ID>();
                auto record = env.getFunction()->getTokenTable()->template getRuntime<Record>(id);
                handler(operand,env,record->getByteLength());
                break;
            }
            default:PANIC;
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
            operand.push<data::boolean>(lhs < rhs);
        }
    };

    template<typename T>
    struct OpGT {
        static void call(Stack &operand, ExecutionEnv &env) {
            T rhs = operand.pop<T>();
            T lhs = operand.pop<T>();
            operand.push<data::boolean>(lhs > rhs);
        }
    };

    template<typename T>
    struct OpLE {
        static void call(Stack &operand, ExecutionEnv &env) {
            T rhs = operand.pop<T>();
            T lhs = operand.pop<T>();
            operand.push<data::boolean>(lhs <= rhs);
        }
    };

    template<typename T>
    struct OpGE {
        static void call(Stack &operand, ExecutionEnv &env) {
            T rhs = operand.pop<T>();
            T lhs = operand.pop<T>();
            operand.push<data::boolean>(lhs >= rhs);
        }
    };

    template<typename T>
    struct OpAdd {
        static void call(Stack &operand, ExecutionEnv &env) {
            T rhs = operand.pop<T>();
            T lhs = operand.pop<T>();
            operand.push<T>(lhs + rhs);
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

    void OpLoadHandler(Stack &operand, ExecutionEnv &env, data::u64 length) {
        auto src_address = reinterpret_cast<data::Byte*>(operand.pop<data::address>());
        operand.pushFrom(length,src_address);
    }

    void OpStoreHandler(Stack &operand, ExecutionEnv &env, data::u64 length) {
        auto dst_address = reinterpret_cast<data::Byte*>(operand.pop<data::address>());
        operand.popTo(length,dst_address);
    }

    void OpLdargHandler(Stack &operand, ExecutionEnv &env, data::u64 length) {
        auto index = operand.pop<data::u16>();
        auto offset = env.getFunction()->getParamOffset(index);
        operand.pushFrom(length,env.getFrame().getRawPtrAt(offset));
    }

    void OpStargHandler(Stack &operand, ExecutionEnv &env, data::u64 length) {
        auto index = operand.pop<data::u16>();
        auto offset = env.getFunction()->getParamOffset(index);
        operand.popTo(length,env.getFrame().getRawPtrAt(offset));
    }

    void OpLdlocHandler(Stack &operand, ExecutionEnv &env, data::u64 length) {
        auto index = operand.pop<data::u16>();
        auto offset = env.getFunction()->getLocalOffset(index);
        operand.pushFrom(length,env.getFrame().getRawPtrAt(offset));
    }

    void OpStlocHandler(Stack &operand, ExecutionEnv &env, data::u64 length) {
        auto index = operand.pop<data::u16>();
        auto offset = env.getFunction()->getLocalOffset(index);
        operand.popTo(length,env.getFrame().getRawPtrAt(offset));
    }

    void OpLdfldHandler(Stack &operand, ExecutionEnv &env, data::u64 length) {
        auto obj_base = operand.pop<data::Byte*>();
        env.consume<data::Byte>();
        auto token_id = env.consume<il::TokenDef::ID>();
        auto field_slot = env.getFunction()->getTokenTable()->getRuntime<FieldSlot>(token_id);
        auto src_address = obj_base + field_slot->getOffset();
        operand.pushFrom(length,src_address);
    }

    void OpStfldHandler(Stack &operand, ExecutionEnv &env, data::u64 length) {
        auto obj_base = operand.pop<data::Byte*>();
        env.consume<data::Byte>();
        auto token_id = env.consume<il::TokenDef::ID>();
        auto field_slot = env.getFunction()->getTokenTable()->getRuntime<FieldSlot>(token_id);
        auto src_address = obj_base + field_slot->getOffset();
        operand.popTo(length,src_address);
    }

    void OpStsfldHandler(Stack &operand, ExecutionEnv &env, data::u64 length) {
        env.consume<data::Byte>();
        auto token_id = env.consume<il::TokenDef::ID>();
        auto field_slot = env.getFunction()->getTokenTable()->getRuntime<StaticFieldSlot>(token_id);
        operand.popTo(length,field_slot->getAddress());
    }

    void OpLdsfldHandler(Stack &operand, ExecutionEnv &env, data::u64 length) {
        env.consume<data::Byte>();
        auto token_id = env.consume<il::TokenDef::ID>();
        auto field_slot = env.getFunction()->getTokenTable()->getRuntime<StaticFieldSlot>(token_id);
        operand.pushFrom(length,field_slot->getAddress());
    }

    void OpLdelemHandler(Stack &operand, ExecutionEnv &env, data::u64 element_length) {
        auto offset = operand.pop<data::i32>();
        data::Byte *ref = operand.pop<data::Byte*>();
        ref += offset * element_length;
        operand.pushFrom(element_length,reinterpret_cast<data::Byte*>(ref));
    }

    void OpLdelemaHandler(Stack &operand, ExecutionEnv &env, data::u64 element_length) {
        auto offset = operand.pop<data::i32>();
        auto ref = operand.pop<data::Byte*>();
        ref += offset * element_length;
        operand.push(ref);
    }

    void OpStelemHandler(Stack &operand, ExecutionEnv &env, data::u64 element_length) {
        auto offset = operand.pop<data::i32>();
        data::Byte *ref = operand.pop<data::Byte*>();
        ref += offset * element_length;
        operand.popTo(element_length,ref);
    }

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
        auto env = ExecutionEnv(function,frame.borrow(0));
        execution_stack.push(env);
        frame_top += function->getStackFrameLength();
    }

    void Processor::copyArgs(Function *function){
        auto frame_address = getCurrentEnv().getFrame().address(0);
        for(auto length : function->getParamsLength()){
            operand.popTo(length,frame_address);
            frame_address += length;
        }
    }


    void Processor::initLocals(ExecutionEnv &env){
        auto &local_frame = env.getLocalFrame();
        std::function<void(Runtime*,data::u16)> fill = [&local_frame, &fill](Runtime *runtime,data::u16 offset){
            switch(runtime->getKind()){
                case RuntimeKind::BuiltIn:{
                    auto builtin = static_cast<BuiltIn*>(runtime);
                    switch(builtin->getBuiltInKind()){
                        case BuiltInKind::u8:   local_frame.write<data::u8>(offset,0);  break;
                        case BuiltInKind::u16:  local_frame.write<data::u16>(offset,0); break;
                        case BuiltInKind::u32:  local_frame.write<data::u32>(offset,0); break;
                        case BuiltInKind::u64:  local_frame.write<data::u64>(offset,0); break;
                        case BuiltInKind::i8:   local_frame.write<data::i8>(offset,0);  break;
                        case BuiltInKind::i16:  local_frame.write<data::i16>(offset,0); break;
                        case BuiltInKind::i32:  local_frame.write<data::i32>(offset,0); break;
                        case BuiltInKind::i64:  local_frame.write<data::i64>(offset,0); break;
                        case BuiltInKind::f32:  local_frame.write<data::i64>(offset,0); break;
                        case BuiltInKind::f64:  local_frame.write<data::f64>(offset,0); break;
                    }
                    break;
                }
                case RuntimeKind::Array:{
                    auto array = static_cast<Array*>(runtime);
                    auto element_length = dynamic_cast<Sizeable*>(array->getElementRuntime())->getByteLength();
                    for(int i=0;i<array->getElementCount();i++){
                        fill(array->getElementRuntime(),offset);
                        offset += element_length;
                    }
                    break;
                }
                case RuntimeKind::Record:{
                    auto record = static_cast<Record*>(runtime);
                    for(auto [_,child] : record->getChilds()){
                        fill(child,offset);
                        offset += dynamic_cast<Sizeable*>(child)->getByteLength();
                    }
                    break;
                }
                case RuntimeKind::Class:{
                    local_frame.write<data::address>(offset,0);
                    break;
                }
                case RuntimeKind::Enum:{
                    PANIC;
                    break;
                }
            }
        };

        int i = 0;
        for(auto local : env.getFunction()->getLocals()){
            auto offset = env.getFunction()->getLocalOffset(i);
            fill(local,offset);
            i++;
        }
    }

    void Processor::run() {
        bool running = true;
        while(running){
            auto code = getCurrentEnv().consume<Bytecode>();
            switch (code) {
                case Bytecode::Nop:
                    break;
                case Bytecode::Ret:
                    frame_top -= getCurrentEnv().getFunction()->getStackFrameLength();
                    execution_stack.pop();
                    if(execution_stack.empty())running = false;
                    break;
                case Bytecode::Callstatic:{
                    auto function = operand.pop<Function*>();
                    execution_stack.push(ExecutionEnv(function,frame.borrow(frame_top)));
                    frame_top += function->getStackFrameLength();
                    copyArgs(function);
                    initLocals(getCurrentEnv());
                    break;
                }
                case Bytecode::Call:{
                    auto function = getCurrentEnv().consume<Function*>();
                    auto obj_base = operand.top<data::Byte*>();
                    execution_stack.push(ExecutionEnv(function,frame.borrow(frame_top)));
                    frame_top += function->getStackFrameLength();
                    copyArgs(function);
                    initLocals(getCurrentEnv());
                    break;
                }
                case Bytecode::CallVirt:{
                    auto slot = getCurrentEnv().consume<VirtualFtnSlot>();
                    auto klass = *((Class**)operand.top<data::Byte*>());
                    auto function = klass->virtualFunctionDispatch(slot);
                    execution_stack.push(ExecutionEnv(function,frame.borrow(frame_top)));
                    frame_top += function->getStackFrameLength();
                    copyArgs(function);
                    initLocals(getCurrentEnv());
                    break;
                }
                case Bytecode::Ldnull:
                    PANIC;
                    break;
                case Bytecode::And:
                    operand.push<data::boolean>(operand.pop<data::boolean>() & operand.pop<data::boolean>());
                    break;
                case Bytecode::Or:
                    operand.push<data::boolean>(operand.pop<data::boolean>() | operand.pop<data::boolean>());
                    break;
                case Bytecode::Xor:
                    operand.push<data::boolean>(operand.pop<data::boolean>() xor operand.pop<data::boolean>());
                    break;
                case Bytecode::Ldloca:{
                    auto index = operand.pop<data::u16>();
                    auto offset = getCurrentEnv().getFunction()->getLocalOffset(index);
                    auto address = getCurrentEnv().getFrame().address(offset);
                    operand.pushFrom(sizeof(data::Byte*),reinterpret_cast<data::Byte*>(&address));
                    break;
                }
                case Bytecode::Ldarga:{
                    auto index = operand.pop<data::u16>();
                    auto offset = getCurrentEnv().getFunction()->getParamOffset(index);
                    auto address = getCurrentEnv().getFrame().address(offset);
                    operand.push(address);
                    break;
                }
                case Bytecode::Ldarg:
                    forEachType(OpLdargHandler, operand, getCurrentEnv());
                    break;
                case Bytecode::Load:
                    forEachType(OpLoadHandler, operand, getCurrentEnv());
                    break;
                case Bytecode::Starg:
                    forEachType(OpStargHandler, operand, getCurrentEnv());
                    break;
                case Bytecode::Store:
                    forEachType(OpStoreHandler, operand, getCurrentEnv());
                    break;
                case Bytecode::Ldloc:
                    forEachType(OpLdlocHandler, operand, getCurrentEnv());
                    break;
                case Bytecode::Stloc:
                    forEachType(OpStlocHandler, operand, getCurrentEnv());
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
                    //todo
                    break;
                }
                case Bytecode::Intrinsic:{
                    auto index = getCurrentEnv().consume<data::Byte>();
                    intrinsic::getHandler((IntrinsicEnum)index)(&operand);
                    break;
                }
                case Bytecode::Ldflda:{
                    auto obj_base = operand.pop<data::Byte*>();
                    getCurrentEnv().consume<data::Byte>();
                    auto token_id = getCurrentEnv().consume<il::TokenDef::ID>();
                    auto field_slot = getCurrentEnv().getFunction()->getTokenTable()->getRuntime<FieldSlot>(token_id);
                    auto address = obj_base + field_slot->getOffset();
                    operand.push(address);
                    break;
                }
                case Bytecode::Ldsflda:{
                    getCurrentEnv().consume<data::Byte>();
                    auto token_id = getCurrentEnv().consume<il::TokenDef::ID>();
                    auto field_slot = getCurrentEnv().getFunction()->getTokenTable()->getRuntime<StaticFieldSlot>(token_id);
                    auto address = field_slot->getAddress();
                    operand.push(address);
                    break;
                }
                case Bytecode::Ldfld:
                    forEachType(OpLdfldHandler, operand, getCurrentEnv());
                    break;
                case Bytecode::Ldsfld:
                    forEachType(OpLdsfldHandler, operand, getCurrentEnv());
                    break;
                case Bytecode::Stfld:
                    forEachType(OpStfldHandler, operand, getCurrentEnv());
                    break;
                case Bytecode::Stsfld:
                    forEachType(OpStsfldHandler, operand, getCurrentEnv());
                    break;
                case Bytecode::Ldelem: {
                    forEachType(OpLdelemHandler, operand, getCurrentEnv());
                    break;
                }
                case Bytecode::Ldelema: {
                    forEachType(OpLdelemaHandler, operand, getCurrentEnv());
                    break;
                }
                case Bytecode::Stelem:{
                    forEachType(OpStelemHandler, operand, getCurrentEnv());
                    break;
                }
                case Bytecode::Add:
                    forEachScalarType<OpAdd>(operand, getCurrentEnv());
                    break;
                case Bytecode::Sub:
                    forEachScalarType<OpSub>(operand, getCurrentEnv());
                    break;
                case Bytecode::Mul:
                    forEachScalarType<OpMul>(operand, getCurrentEnv());
                    break;
                case Bytecode::Div:
                    forEachScalarType<OpDiv>(operand, getCurrentEnv());
                    break;
                case Bytecode::FDiv:
                    forEachScalarType<OpFDiv>(operand, getCurrentEnv());
                    break;
                case Bytecode::EQ:
                    forEachScalarType<OpEQ>(operand, getCurrentEnv());
                    break;
                case Bytecode::NE:
                    forEachScalarType<OpNE>(operand, getCurrentEnv());
                    break;
                case Bytecode::LT:
                    forEachScalarType<OpLT>(operand, getCurrentEnv());
                    break;
                case Bytecode::GT:
                    forEachScalarType<OpGT>(operand, getCurrentEnv());
                    break;
                case Bytecode::LE:
                    forEachScalarType<OpLE>(operand, getCurrentEnv());
                    break;
                case Bytecode::GE:
                    forEachScalarType<OpGE>(operand, getCurrentEnv());
                    break;
                case Bytecode::Neg:
                    forEachScalarType<OpNeg>(operand, getCurrentEnv());
                    break;
                case Bytecode::Pop:
                    forEachScalarType<OpPop>(operand, getCurrentEnv());
                    break;
                case Bytecode::Dup:
                    forEachScalarType<OpDup>(operand, getCurrentEnv());
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
                    forEachScalarType<OpPush>(operand, getCurrentEnv());
                    break;
                case Bytecode::CastCls:
                    PANIC;
                    break;
                case Bytecode::Conv:
                    forEachScalarType<OpConv>(operand, getCurrentEnv());
                    break;
            }
        }
    }

    ExecutionEnv &Processor::getCurrentEnv() {
        return execution_stack.top();
    };


    ExecutionEnv::ExecutionEnv(Function *function, Memory frame)
        : function(function),pc(function->getBlock()),frame(frame),locals_frame(frame.borrow(function->getParamsStackFrameLength())){}
}
