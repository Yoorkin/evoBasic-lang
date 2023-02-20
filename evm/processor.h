#ifndef EVM_EXECUTION
#define EVM_EXECUTION
#include <atomic>
#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>
#include <stack>
#include <optional>
#include "interop.h"
#include "loader.h"
#include "bytecode.h"
#include "runtime.h"
#include "unicode.h"
#include "utils.h"

//#define DEBUG

#ifdef DEBUG
    #define DEBUG_INSTRUCTION(x) LOG(IL,"       ! "#x<<std::endl);
    #define DEBUG_FUNCTION_ENTER(x) LOG(Frame, " >> enter "<< x);
    #define DEBUG_FUNCTION_EXIT(x) LOG(Frame, " >> exit " << x);
    #define DEBUG_STACKFRAME_PUSH(x) LOG(Frame, " >> stackframe push "<<x<<std::endl);
    #define DEBUG_STACKFRAME_POP(x) LOG(Frame, " >> stackframe pop "<<x<<std::endl);
#else
    #define DEBUG_INSTRUCTION(x) 
    #define DEBUG_FUNCTION_ENTER(x)
    #define DEBUG_FUNCTION_EXIT(x)
    #define DEBUG_STACKFRAME_PUSH(x)
    #define DEBUG_STACKFRAME_POP(x)
#endif

#define LOG_INST(x) LOG(Instruction,"(" << call_stack.back().getHostedFunction()->qualifiedName()\
                                        <<",offset " << call_stack.back().ip - call_stack.back().getHostedFunction()->getBlock()\
                                        << ",line " << call_stack.back().line << ") " << x <<"\n")


inline bool isSubtypeOf(runtime::Class* a, runtime::Class* b) {
    while (a != nullptr) {
        if (a == b) return true;
        a = a->getBaseClass();
    }
    return false;
}

inline bool isInstanceOf(interop::Instance* instance, runtime::Class* klass) {
    runtime::Class* ins_class = instance->klass;
    return isSubtypeOf(ins_class, klass);
}

class CallEnv;

class ExceptionHandler{
    struct Handler{
        CallEnv *stack_frame = nullptr;
        uint8_t *entry = nullptr;
        runtime::Class *exception_class = nullptr;
    };
    std::stack<Handler> handlers;  
public:

    inline std::optional<Handler> search(runtime::Class *exception_class){
        while (handlers.size() > 0) {
            Handler handler = handlers.top();
            handlers.pop();
            if (isSubtypeOf(exception_class, handler.exception_class)) {
                return {handler};
            }
        }
        return {};
    }

    inline void push(runtime::Class *exception_class, CallEnv *stack_frame, uint8_t *entry){
        Handler handler;
        handler.exception_class = exception_class;
        handler.stack_frame = stack_frame;
        handler.entry = entry;
        handlers.push(handler);
    }

    inline void pop(){
        handlers.pop();
    }

};

class CallEnv{
    MemoryStack &frame;
    runtime::HostedFunction *hosted = nullptr;
    uint8_t *memory = nullptr;
    std::list<uintptr_t> live_set;
public:

    uint32_t line;

    uint8_t *ip = nullptr;

    inline CallEnv(runtime::HostedFunction *hosted, std::list<uintptr_t> live_set, uint8_t *memory, MemoryStack &frame)
        : hosted(hosted), live_set(live_set), memory(memory), ip(hosted->getBlock()), frame(frame){

        if(hosted->getLineNumberTable()->getNumberCount() > 0){
            line = hosted->getLineNumberTable()->getLineNumber(0).line;
        }

        LOG(CallEnv,"enter "<<hosted->qualifiedName() << std::endl)
        
    }

    inline CallEnv(CallEnv &&x) : frame(x.frame),hosted(x.hosted),memory(x.memory),ip(x.ip){
        x.hosted = nullptr;
    }

    inline const std::list<uintptr_t> &getLiveSet(){ return live_set; }

    inline ~CallEnv(){
        if(hosted!=nullptr){
            frame.pop(hosted->getParamMemorySize() + hosted->getLocalMemorySize());
            LOG(CallEnv, "exit " << hosted->qualifiedName() << std::endl)
        }
    }

    inline uint8_t *getMemory() const { return memory; }
    inline runtime::HostedFunction *getHostedFunction() const { return hosted; }

};

class Processor{
    Loader &loader;

    MemoryStack operand,frame;
    ExceptionHandler exception_handler;
    std::list<CallEnv> call_stack;
    bool fata_error_occur = false;

    bool arrayAccessCheck(interop::ArrayInstance *instance, int subscript);
    bool nullPointerCheck(interop::Instance *instance);
    bool optionalParameterCheck(CallEnv &env, uint16_t index);

    template<class T>
    T consume(){
        auto r = read<T>(call_stack.back().ip);
        call_stack.back().ip += sizeof(T);
        return r;
    }

public:
    template<class T>
    void OpAddRoot() {}
    template<>
    void OpAddRoot<interop::Instance*>() {
        loader.getGC()->addRoot(operand.ptrToPeek<interop::Instance*>());
    }
    template<>
    void OpAddRoot<interop::InteriorPointer>() {
        auto itp = operand.ptrToPeek<interop::InteriorPointer>();
        loader.getGC()->addRoot((uintptr_t)itp, Reference::fromInterior(itp));
    }
    template<>
    void OpAddRoot<interop::Delegate>(){
        OpAddRoot<interop::Instance*>();
    }


    template<class T>
    void OpRemoveRoot() {}
    template<>
    void OpRemoveRoot<interop::Instance*>() {
        loader.getGC()->removeRoot(operand.ptrToPeek<interop::Instance*>());
    }
    template<>
    void OpRemoveRoot<interop::InteriorPointer>() {
        auto itp = operand.ptrToPeek<interop::InteriorPointer>();
        loader.getGC()->removeRoot((uintptr_t)itp);
    }
    template<>
    void OpRemoveRoot<interop::Delegate>(){
        OpRemoveRoot<interop::Instance*>();
    }
private:

    template<class T>
    void OpStarg(bool ref){
        auto idx = operand.pop<uint16_t>();
        OpRemoveRoot<T>();
        auto val = operand.pop<T>();
        auto hosted = call_stack.back().getHostedFunction();
        if(hosted->getParameterByIndex(idx)->getKind() == runtime::ParameterKind::Optional){
            optionalParameterCheck(call_stack.back(),idx);
        }
        auto offset = hosted->getParameterByIndex(idx)->getOffset();
        auto address = (T*)(call_stack.back().getMemory() + offset);
        *address = val;
        LOG_INST("starg." << genericTypeToString<T>() << " " << idx << "    "  << "->" << "?")
    }

    template<class T>
    void OpLdarg(bool ref){
        auto idx = operand.pop<uint16_t>();
        auto hosted = call_stack.back().getHostedFunction();
        if(hosted->getParameterByIndex(idx)->getKind() == runtime::ParameterKind::Optional){
            optionalParameterCheck(call_stack.back(),idx);
        }
        auto offset = hosted->getParameterByIndex(idx)->getOffset();
        auto val = *((T*)(call_stack.back().getMemory() + offset));
        LOG_INST("ldarg." << genericTypeToString<T>() << " " << idx);
        operand.push<T>(val);
        OpAddRoot<T>();
    }

    template<class T>
    void OpStloc(bool ref){
        auto idx = operand.pop<uint16_t>();
        OpRemoveRoot<T>();
        auto val = operand.pop<T>();
        auto offset = call_stack.back().getHostedFunction()->getLocalOffset(idx);
        auto base = call_stack.back().getMemory();
        auto address = (T*)(base+offset);
        *address = val;
        LOG_INST("stloc." << genericTypeToString<T>() << " " << idx)
    }

    template<class T>
    void OpLdloc(bool ref){
        auto idx = operand.pop<uint16_t>();
        LOG_INST("ldloc." << genericTypeToString<T>() << " " << idx)
        auto offset = call_stack.back().getHostedFunction()->getLocalOffset(idx);
        auto base = call_stack.back().getMemory();
        auto val = *((T*)(base + offset));
        operand.push<T>(val);
        OpAddRoot<T>();
    }

    template<class T>
    void OpStfld(bool ref){
        auto tok = consume<token_t>();
        uint8_t hint = operand.pop<uint8_t>();
        auto fld = (runtime::Variable*)call_stack.back().getHostedFunction()->getTable().query(tok);
        LOG_INST("stfld." << genericTypeToString<T>() << " " << fld->qualifiedName())
        if(hint==1){ // ins
            OpRemoveRoot<interop::Instance*>();
            auto ins = operand.pop<interop::Instance*>();
            OpRemoveRoot<T>();
            auto val_ptr = operand.popAndGetTop(fld->getLength());
            if (nullPointerCheck(ins)) {
                auto dst = (uint8_t*)ins + fld->getOffset();
                memcpy(dst,val_ptr,fld->getOffset());
            }        
        }
        else if(hint==2){// hld
            OpRemoveRoot<interop::InteriorPointer>();
            auto itp = operand.pop<interop::InteriorPointer>();
            OpRemoveRoot<T>();
            auto val_ptr = operand.popAndGetTop(fld->getLength());
            auto dst = itp.ptr + fld->getOffset();
            memcpy(dst,val_ptr,fld->getLength());
        }
        else if(hint==3){// record
            throw "";
        }
    }

    template<class T>
    void OpLdfld(bool ref){
        auto tok = consume<token_t>();
        uint8_t hint = operand.pop<uint8_t>();
        auto fld = (runtime::Variable*)call_stack.back().getHostedFunction()->getTable().query(tok);
        LOG_INST("ldfld." << genericTypeToString<T>() << " " << fld->qualifiedName())
        if(hint==1){ // ins
            OpRemoveRoot<interop::Instance*>();
            auto ins = operand.pop<interop::Instance*>();
            if (nullPointerCheck(ins)) {
                auto ptr = (uint8_t*)ins + fld->getOffset();
                operand.pushFromPtr(ptr, fld->getLength());
                OpAddRoot<T>();
            }                    
        }
        else if(hint==2){ // hld
            OpRemoveRoot<interop::InteriorPointer>();
            auto itp = operand.pop<interop::InteriorPointer>();
            auto ptr = itp.ptr + fld->getOffset();
            operand.pushFromPtr(ptr, fld->getLength());
            OpAddRoot<T>();
        }
        else if(hint==3){ // record
            auto record = dynamic_cast<runtime::Record*>(fld->parent);
            auto ptr = operand.popAndGetTop(record->getMemorySize());
            operand.moveFromPtr(ptr,fld->getLength());
        }
    }

    template<class T>
    void OpStsfld(bool ref){
        auto tok = consume<token_t>();
        OpRemoveRoot<T>();
        auto val = operand.pop<T>();
        auto fld = (runtime::Variable*)call_stack.back().getHostedFunction()->getTable().query(tok);
        *((T*)fld->getStaticAddress()) = val;
        LOG_INST("stsfld." << genericTypeToString<T>() << " " << fld->qualifiedName())
    }

    template<class T>
    void OpLdsfld(bool ref){
        auto tok = consume<token_t>();
        auto fld = (runtime::Variable*)call_stack.back().getHostedFunction()->getTable().query(tok);
        auto val = *((T*)fld->getStaticAddress());
        operand.push<T>(val);
        OpAddRoot<T>();
        LOG_INST("ldsfld." << genericTypeToString<T>() << " " << fld->qualifiedName())
    }

    template<class T>
    void OpStelem(bool ref){
        auto tok = consume<token_t>();
        auto idx = operand.pop<int32_t>();
        OpRemoveRoot<interop::Instance*>();
        auto ins = operand.pop<interop::ArrayInstance*>();
        OpRemoveRoot<T>();
        auto val = operand.pop<T>();
        if (nullPointerCheck((interop::Instance*)ins) & arrayAccessCheck(ins, idx)) {
            auto base = (uint8_t*)ins + sizeof(interop::ArrayInstance);
            auto element_sym = call_stack.back().getHostedFunction()->getTable().query(tok);
            *((T*)(base + idx * runtime::getRuntimeSize(element_sym))) = val;
            LOG_INST("stelem." << genericTypeToString<T>() << " " << element_sym->qualifiedName())
        }
    }

    template<class T>
    void OpStelemr(bool ref){
        auto tok = consume<token_t>();
        OpRemoveRoot<T>();
        auto val = operand.pop<T>();    
        auto idx = operand.pop<int32_t>();
        OpRemoveRoot<interop::Instance*>();
        auto ins = operand.pop<interop::ArrayInstance*>();
        if (nullPointerCheck((interop::Instance*)ins) & arrayAccessCheck(ins, idx)) {
            auto base = (uint8_t*)ins + sizeof(interop::ArrayInstance);
            auto element_sym = call_stack.back().getHostedFunction()->getTable().query(tok);
            *((T*)(base + idx * runtime::getRuntimeSize(element_sym))) = val;
            LOG_INST("stelemr." << genericTypeToString<T>() << " " << element_sym->qualifiedName())
        }
    }

    template<class T>
    void OpLdelem(bool ref){
        auto tok = consume<token_t>();
        auto idx = operand.pop<int32_t>();
        OpRemoveRoot<interop::Instance>();
        auto ins = operand.pop<interop::ArrayInstance*>();
        if (nullPointerCheck((interop::Instance*)ins) & arrayAccessCheck(ins, idx)) {
            auto element_sym = call_stack.back().getHostedFunction()->getTable().query(tok);
            auto base = (uint8_t*)ins + sizeof(interop::ArrayInstance);
            auto val = *((T*)(base + idx * runtime::getRuntimeSize(element_sym)));
            operand.push<T>(val);
            OpAddRoot<T>();
            LOG_INST("ldelem." << genericTypeToString<T>() << " " << element_sym->qualifiedName())
        }
    }

    template<class T>
    void OpDup(bool ref){
        auto val = operand.peek<T>();
        operand.push<T>(val);
        OpAddRoot<T>();
        LOG_INST("dup." << genericTypeToString<T>())
    }

    template<class T>
    void OpPush(bool ref){
        auto val = consume<T>();
        operand.push<T>(val);
        OpAddRoot<T>();
        LOG_INST("push." << genericTypeToString<T>())
    }

    template<class T>
    void OpPop(){
        OpRemoveRoot<T>();
        operand.pop<T>();
        LOG_INST("pop." << genericTypeToString<T>());
    }

    template<>
    void OpPop<interop::RecordOpaque>(){
        auto tok = consume<token_t>();
        auto rcd = dynamic_cast<runtime::Record*>(call_stack.back().getHostedFunction()->getTable().query(tok));
        operand.popAndGetTop(rcd->getMemorySize());
    }

    template<class T>
    void OpStore(bool ref){
        OpRemoveRoot<interop::InteriorPointer>();
        auto itp = operand.pop<interop::InteriorPointer>();
        OpRemoveRoot<T>();
        auto val = operand.pop<T>();
        *((T*)itp.ptr) = val;
        LOG_INST("store." << genericTypeToString<T>())
    }

    template<class T>
    void OpLoad(bool ref){
        OpRemoveRoot<interop::InteriorPointer>();
        auto itp = operand.pop<interop::InteriorPointer>();
        auto val = *((T*)itp.ptr);
        operand.push<T>(val);
        OpAddRoot<T>();
        LOG_INST("load." << genericTypeToString<T>())
    }

    template<class T>
    void OpConvert(){
        auto val = operand.pop<T>();
        switch (consume<uint8_t>()) {
            case bytecode::t_i8: 
                operand.push<int8_t>((uint8_t)val); 
                LOG_INST("convert." << genericTypeToString<T>() << " int8_t")
                break;
            case bytecode::t_i16: 
                operand.push<int16_t>((int16_t)val); 
                LOG_INST("convert." << genericTypeToString<T>() << " int16_t")
                break;
            case bytecode::t_i32:
                operand.push<int32_t>((int32_t)val);
                LOG_INST("convert." << genericTypeToString<T>() << " int32_t")
                break;
            case bytecode::t_i64:
                operand.push<int64_t>((int64_t)val);
                LOG_INST("convert." << genericTypeToString<T>() << " int64_t")
                break;
            case bytecode::t_u8: 
                operand.push<uint8_t>((uint8_t)val);
                LOG_INST("convert." << genericTypeToString<T>() << " uint8_t")
                break;
            case bytecode::t_u16:
                operand.push<uint16_t>((uint16_t)val);
                LOG_INST("convert." << genericTypeToString<T>() << " uint16_t")
                break;
            case bytecode::t_u32:
                operand.push<uint32_t>((uint32_t)val);
                LOG_INST("convert." << genericTypeToString<T>() << " uint32_t")
                break;
            case bytecode::t_u64: 
                operand.push<uint64_t>((uint64_t)val);
                LOG_INST("convert." << genericTypeToString<T>() << " uint64_t")
                break;
            case bytecode::t_f32:
                operand.push<float>((float)val);
                LOG_INST("convert." << genericTypeToString<T>() << " float")
                break;
            case bytecode::t_f64:
                operand.push<double>((double)val);
                LOG_INST("convert." << genericTypeToString<T>() << " double")
                break;
        }
    }

    template<class T>
    void OpNeg(){
        auto val = operand.pop<T>();
        operand.push<T>(-val);   
        LOG_INST("neg." << genericTypeToString<T>())
    }

    template<class T>
    void OpAdd(){
        auto rhs = operand.pop<T>();
        auto lhs = operand.pop<T>();
        operand.push<T>(lhs + rhs);
        LOG_INST("add." << genericTypeToString<T>())
    }

    template<class T>
    void OpSub(){
        auto rhs = operand.pop<T>();
        auto lhs = operand.pop<T>();
        operand.push<T>(lhs - rhs);
        LOG_INST("sub." << genericTypeToString<T>())
    }

    template<class T>
    void OpMul(){
        auto rhs = operand.pop<T>();
        auto lhs = operand.pop<T>();
        operand.push<T>(lhs * rhs);
        LOG_INST("mul." << genericTypeToString<T>())
    }

    template<class T>
    void OpDiv(){
        auto rhs = operand.pop<T>();
        auto lhs = operand.pop<T>();
        LOG_INST("div." << genericTypeToString<T>())
        if (rhs == 0) {
            auto ins = loader.getInteropAgent()->createInstance(loader.getEBDivideByZeroException(),{});
            handleException(std::move(ins));
        }
        else {
            operand.push<T>(lhs / rhs);
        }
    }

    template<class T>
    void OpEQ(){
        auto rhs = operand.pop<T>();
        auto lhs = operand.pop<T>();
        operand.push<uint8_t>(lhs == rhs);
        LOG_INST("eq." << genericTypeToString<T>())
    }

    template<class T>
    void OpNE(){
        auto rhs = operand.pop<T>();
        auto lhs = operand.pop<T>();
        operand.push<uint8_t>(lhs != rhs);
        LOG_INST("ne." << genericTypeToString<T>())
    }

    template<class T>
    void OpLT(){
        auto rhs = operand.pop<T>();
        auto lhs = operand.pop<T>();
        operand.push<uint8_t>(lhs < rhs);
        LOG_INST("lt." << genericTypeToString<T>())
    }

    template<class T>
    void OpGT(){
        auto rhs = operand.pop<T>();
        auto lhs = operand.pop<T>();
        operand.push<uint8_t>(lhs > rhs);
        LOG_INST("gt." << genericTypeToString<T>())
    }

    template<class T>
    void OpLE(){
        auto rhs = operand.pop<T>();
        auto lhs = operand.pop<T>();
        operand.push<uint8_t>(lhs <= rhs);
        LOG_INST("le." << genericTypeToString<T>())
    }

    template<class T>
    void OpGE(){
        auto rhs = operand.pop<T>();
        auto lhs = operand.pop<T>();
        operand.push<uint8_t>(lhs >= rhs);
        LOG_INST("ge." << genericTypeToString<T>())
    }

    template<class T>
    void OpMod(){
        auto rhs = operand.pop<T>();
        auto lhs = operand.pop<T>();
        operand.push<T>(lhs % rhs);
        LOG_INST("mod." << genericTypeToString<T>())
    }
    
public:
    void popArgsFromOperand(runtime::HostedFunction *hosted, uint8_t *frame, std::list<Reference> &live_set);
    void invokeStaticMethod(runtime::Method *method);
    void invokeVirtualMethod(runtime::VirtualMethod *method);
    void invokeCtor(runtime::Ctor *ctor);
    void invokeMethod(runtime::Method *method);

    inline Loader &getLoader(){ return loader; }
    inline MemoryStack &getOperand(){ return operand; }
    inline MemoryStack &getFrame(){ return frame; }
    inline ExceptionHandler &getExceptionHandler(){ return exception_handler; }
    inline std::list<CallEnv> &getCallStack(){ return call_stack; }

    void handleException(interop::ProtectedCell exception_cell);
    void execute(runtime::Method *static_method = nullptr);

    inline Processor(Loader *loader) : loader(*loader), operand(2048), frame(2048){}
};

#endif