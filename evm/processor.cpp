#include "processor.h"
#include "runtime.h"
#include "utils.h"
#include "interop.h"
#include "gc.h"

std::list<uintptr_t> mapLiveSet(std::list<Reference> live_set){
    std::list<uintptr_t> ret;
    for(auto r : live_set)ret.push_back(r.getID());
    return ret;
}

void Processor::popArgsFromOperand(runtime::HostedFunction *hosted, uint8_t *frame, std::list<Reference> &live_set){
    if(hosted->getParamArray() != nullptr){
        loader.getGC()->removeRoot(operand.ptrToPeek<interop::Instance*>());
        auto param_array_ref = operand.pop<interop::ArrayInstance*>();
        auto dst = frame + hosted->getParamArray()->getOffset();
        seqcpy(dst, (uint8_t*)(&param_array_ref), sizeof(void*));
    }

    if(hosted->getOptionalParameters().size() != 0){
        auto option_count = operand.pop<uint8_t>();
        while(option_count--){
            auto option = operand.pop<runtime::OptionalParameter*>();
            *(frame + option->getFlagOffset()) = 1; 
            // 在从operand上拷贝数据到栈帧之前设置option flag。
            // 因为当option参数是按引用传递时，flag与value共用内存空间。
            auto dst_ptr = frame + option->getOffset();
            if(option->getEvalKind() == runtime::EvaluationKind::Byref){
                OpRemoveRoot<interop::InteriorPointer>();
            }
            else if(runtime::instancesOf<runtime::Class>(option->getType())){
                OpRemoveRoot<interop::Instance*>();
            }
            seqcpy(dst_ptr, operand.popAndGetTop(option->getLength()), option->getLength());
        }
    }

    for(int i = 0; i < hosted->getNormalParameters().size(); i++){
        auto &parameter = hosted->getNormalParameters()[i];
        
        auto dst_ptr = frame + parameter->getOffset();
        if(parameter->getEvalKind() == runtime::EvaluationKind::Byref){
            OpRemoveRoot<interop::InteriorPointer>();
        }
        else if(runtime::instancesOf<runtime::Class>(parameter->getType())){ //byval
            OpRemoveRoot<interop::Instance*>();
        }
        auto src_ptr = operand.popAndGetTop(parameter->getLength());
        seqcpy(dst_ptr, src_ptr, parameter->getLength());
    }

    for(auto ref_offset : hosted->getStackFrameRefOffsets()){
        live_set.push_back(Reference::fromRefPtr((interop::Instance**)(frame + ref_offset)));
    }

    for(auto itp_offset : hosted->getStackFrameInteriorPointerOffsets()){
        live_set.push_back(Reference::fromInterior((interop::InteriorPointer*)(frame + itp_offset)));
    }

}

void Processor::invokeStaticMethod(runtime::Method *method){
    auto frame_size = method->getParamMemorySize() + method->getLocalMemorySize();
    auto memory = getFrame().borrow(frame_size);
    memset(memory, 0, frame_size);
    std::list<Reference> live_set;
    popArgsFromOperand(method,memory,live_set);
    call_stack.push_back(CallEnv(method,mapLiveSet(live_set),memory,getFrame()));

    for(Reference live : live_set){
        loader.getGC()->addRoot(live.getID(),live);
    }
}

void Processor::invokeVirtualMethod(runtime::VirtualMethod *method){
    auto memory = getFrame().borrow(method->getSelfImpl()->getParamMemorySize());
    memset(memory, 0, method->getSelfImpl()->getParamMemorySize());
    std::list<Reference> live_set;
    popArgsFromOperand(method->getSelfImpl(), memory, live_set);

    // self pointer
    loader.getGC()->removeRoot(operand.ptrToPeek<interop::Instance*>());
    auto instance = getOperand().pop<interop::Instance*>();

    if (nullPointerCheck(instance)) {
        *((interop::Instance**)memory) = instance; // 设置参数栈第一个参数为实例的引用
        auto ftn = instance->klass->dispatchMethod(method->getVTableOffset());
        auto local_memory = getFrame().borrow(ftn->getLocalMemorySize()); // 分配虚函数表中真正调用的函数的栈帧内存
        memset(local_memory, 0, ftn->getLocalMemorySize());
        call_stack.push_back(CallEnv(ftn,mapLiveSet(live_set),memory,getFrame()));

        for(Reference live : live_set){
            loader.getGC()->addRoot(live.getID(),live);
        }
    }
}

void Processor::invokeCtor(runtime::Ctor *ctor){
    auto frame_size = ctor->getParamMemorySize() + ctor->getLocalMemorySize();
    auto memory = getFrame().borrow(frame_size);
    memset(memory, 0, frame_size);
    std::list<Reference> live_set;
    popArgsFromOperand(ctor, memory, live_set);
    
    // self pointer
    loader.getGC()->removeRoot(operand.ptrToPeek<interop::Instance*>());
    auto instance = getOperand().pop<interop::Instance*>();

    *((interop::Instance**)memory) = instance;// 设置参数栈第一个参数为实例的引用
    call_stack.push_back(CallEnv(ctor, mapLiveSet(live_set), memory, getFrame()));

    for(Reference live : live_set){
        loader.getGC()->addRoot(live.getID(),live);
    }
}

void Processor::invokeMethod(runtime::Method *method){
    auto frame_size = method->getParamMemorySize() + method->getLocalMemorySize();
    auto memory = getFrame().borrow(frame_size);
    memset(memory, 0, frame_size);
    std::list<Reference> live_set;
    popArgsFromOperand(method, memory, live_set);

    // self pointer
    loader.getGC()->removeRoot(operand.ptrToPeek<interop::Instance*>());
    auto instance = getOperand().pop<interop::Instance*>();

    if (nullPointerCheck(instance)) {
        *((interop::Instance**)memory) = instance;
        call_stack.push_back(CallEnv(method, mapLiveSet(live_set), memory, getFrame()));

        for(Reference live : live_set){
            loader.getGC()->addRoot(live.getID(),live);
        }
    }
}

#define ForEachIntegralType(Op) \
    switch (consume<uint8_t>()) {\
        case bytecode::t_boolean: Op<uint8_t>(); break;\
        case bytecode::t_i8: Op<int8_t>(); break;\
        case bytecode::t_i16: Op<int16_t>(); break;\
        case bytecode::t_i32: Op<int32_t>(); break;\
        case bytecode::t_i64: Op<int64_t>(); break;\
        case bytecode::t_u8: Op<uint8_t>(); break;\
        case bytecode::t_u16: Op<uint16_t>(); break;\
        case bytecode::t_u32: Op<uint32_t>(); break;\
        case bytecode::t_u64: Op<uint64_t>(); break;\
        default: throw std::invalid_argument("unexpedted type");\
    }

#define ForEachArithmeticType(Op) \
    switch (consume<uint8_t>()) {\
        case bytecode::t_boolean: Op<uint8_t>(); break;\
        case bytecode::t_i8: Op<int8_t>(); break;\
        case bytecode::t_i16: Op<int16_t>(); break;\
        case bytecode::t_i32: Op<int32_t>(); break;\
        case bytecode::t_i64: Op<int64_t>(); break;\
        case bytecode::t_u8: Op<uint8_t>(); break;\
        case bytecode::t_u16: Op<uint16_t>(); break;\
        case bytecode::t_u32: Op<uint32_t>(); break;\
        case bytecode::t_u64: Op<uint64_t>(); break;\
        case bytecode::t_f32: Op<float>(); break;\
        case bytecode::t_f64: Op<double>(); break;\
        default: throw std::invalid_argument("unexpedted type");\
    }

#define ForEachTypeWithOrd(Op) \
    auto typ = consume<uint8_t>();\
    switch (typ) {\
        case bytecode::t_boolean: Op<uint8_t>(); break;\
        case bytecode::t_i8: Op<int8_t>(); break;\
        case bytecode::t_i16: Op<int16_t>(); break;\
        case bytecode::t_i32: Op<int32_t>(); break;\
        case bytecode::t_i64: Op<int64_t>(); break;\
        case bytecode::t_u8: Op<uint8_t>(); break;\
        case bytecode::t_u16: Op<uint16_t>(); break;\
        case bytecode::t_u32: Op<uint32_t>(); break;\
        case bytecode::t_u64: Op<uint64_t>(); break;\
        case bytecode::t_f32: Op<float>(); break;\
        case bytecode::t_f64: Op<double>(); break;\
        case bytecode::t_emconst: Op<void*>(); break;\
        case bytecode::t_ref: Op<interop::Instance*>(); break;\
        default: throw std::invalid_argument("unexpedted type");\
    }


#define ForEachType(Op) \
    auto typ = consume<uint8_t>();\
    switch (typ) {\
        case bytecode::t_boolean: Op<uint8_t>(); break;\
        case bytecode::t_i8: Op<int8_t>(); break;\
        case bytecode::t_i16: Op<int16_t>(); break;\
        case bytecode::t_i32: Op<int32_t>(); break;\
        case bytecode::t_i64: Op<int64_t>(); break;\
        case bytecode::t_u8: Op<uint8_t>(); break;\
        case bytecode::t_u16: Op<uint16_t>(); break;\
        case bytecode::t_u32: Op<uint32_t>(); break;\
        case bytecode::t_u64: Op<uint64_t>(); break;\
        case bytecode::t_f32: Op<float>(); break;\
        case bytecode::t_f64: Op<double>(); break;\
        case bytecode::t_emconst: Op<void*>(); break;\
        case bytecode::t_ref: Op<interop::Instance*>(); break;\
        case bytecode::t_hdl: Op<interop::InteriorPointer>(); break;\
        case bytecode::t_record: Op<interop::RecordOpaque>(); break;\
        default: throw std::invalid_argument("unexpedted type");\
    }

#define ForEachTypeWithRefFlag(Op) \
    auto typ = consume<uint8_t>();\
    switch (typ) {\
        case bytecode::t_boolean: Op<uint8_t>(false); break;\
        case bytecode::t_i8: Op<int8_t>(false); break;\
        case bytecode::t_i16: Op<int16_t>(false); break;\
        case bytecode::t_i32: Op<int32_t>(false); break;\
        case bytecode::t_i64: Op<int64_t>(false); break;\
        case bytecode::t_u8: Op<uint8_t>(false); break;\
        case bytecode::t_u16: Op<uint16_t>(false); break;\
        case bytecode::t_u32: Op<uint32_t>(false); break;\
        case bytecode::t_u64: Op<uint64_t>(false); break;\
        case bytecode::t_f32: Op<float>(false); break;\
        case bytecode::t_f64: Op<double>(false); break;\
        case bytecode::t_emconst: Op<void*>(false); break;\
        case bytecode::t_ref: Op<interop::Instance*>(true); break;\
        case bytecode::t_hdl: Op<interop::InteriorPointer>(true); break;\
        case bytecode::t_record: Op<interop::RecordOpaque>(false); break;\
        default: throw std::invalid_argument("unexpedted type");\
    }

bool Processor::arrayAccessCheck(interop::ArrayInstance *instance, int subscript){
    if(subscript < 0 || subscript >= instance->length){
        auto ins = loader.getInteropAgent()->createInstance(loader.getEBOutOfRangeException(), {
            interop::Value::fromI32(subscript),
            interop::Value::fromI32(instance->length)
        });
        handleException(std::move(ins));
        return false;
    }
    return true;
}

bool Processor::nullPointerCheck(interop::Instance *instance){
    if(instance==nullptr){
        auto str = loader.getInteropAgent()->createString("?"_utf32);
        auto ins = loader.getInteropAgent()->createInstance(loader.getEBNullPointerException(), {
            interop::Value::fromRef(std::move(str))
        });
        handleException(std::move(ins));
        return false;
    }
    return true;
}

bool Processor::optionalParameterCheck(CallEnv &env, uint16_t index){
    auto parameter = dynamic_cast<const runtime::OptionalParameter*>(env.getHostedFunction()->getParameterByIndex(index));
    if (parameter->getKind() == runtime::ParameterKind::Optional) {
        auto flag_offset = parameter->getFlagOffset();
        if(*(env.getMemory() + flag_offset)==0){
            auto &str = env.getHostedFunction()->getParameterByIndex(index)->name;
            auto str_ins = loader.getInteropAgent()->createString(str);
            auto ins = loader.getInteropAgent()->createInstance(loader.getEBOptionMissingException(), {
                interop::Value::fromRef(std::move(str_ins))
            });
            handleException(std::move(ins));
            return false;
        }
    }
    return true;
}

void Processor::handleException(interop::ProtectedCell cell){
    auto handler = getExceptionHandler().search(cell.get<interop::ExceptionInstance*>()->base.klass);

    unicode::string trace;
    int i = 0;
    for (auto iter = getCallStack().cbegin(); iter != getCallStack().cend(); iter++) {
        i++;
        trace += "    In function '"_utf32;
        if (auto ctor = dynamic_cast<runtime::Ctor*>(iter->getHostedFunction()))
            trace += ctor->getClass()->qualifiedName() + ".constructor"_utf32;
        else
            trace += iter->getHostedFunction()->qualifiedName();
        
        trace += "' line "_utf32 + unicode::to_string(iter->line)
            + (i != getCallStack().size() ? ",\n"_utf32 : ".\n"_utf32);
    }

    if(handler.has_value()){
        while(&getCallStack().back() != handler.value().stack_frame){
            getCallStack().pop_back();
        }
        getCallStack().back().ip = handler.value().entry;
        cell.get<interop::ExceptionInstance*>()->name = loader.getInteropAgent()->createString(cell.get<interop::ExceptionInstance*>()->base.klass->name)
                                                                                    .get<interop::StringInstance*>();
        cell.get<interop::ExceptionInstance*>()->trace = loader.getInteropAgent()->createString(trace).get<interop::StringInstance*>();

        getOperand().push(cell.get<interop::ExceptionInstance*>());
        OpAddRoot<interop::Instance*>();
    }
    else{
        std::cout<<"Unexpected "<<cell.get<interop::ExceptionInstance*>()->base.klass->name<<": "
                    <<loader.getInteropAgent()->fetchStringFromInstance(cell.get<interop::ExceptionInstance*>()->message)<<"\n";
        std::cout<<trace<<"Stop."<<std::endl;
        fata_error_occur = true;
    }  
}

void Processor::execute(runtime::Method *static_method){
    if(static_method!=nullptr){
        invokeStaticMethod(static_method);
    }

    auto top_frame = &call_stack.back();
    bool exit = false;

    while (!exit && !fata_error_occur) {
        //getLoader().getGC()->minorGC();

        auto offset = call_stack.back().ip - call_stack.back().getHostedFunction()->getBlock();
        call_stack.back().line = call_stack.back().getHostedFunction()->getLineNumberTable()->determineLine(offset);//这个地方太慢了
        
        auto code = consume<uint8_t>();
        switch (code) {
            /*
                code x = "case bytecode::" ++ x ++ ":{\n\n\tbreak;\n}\n"
                putStr $ foldl1 (++) (fmap (code . unpack) (Data.Text.splitOn (pack ",") (pack "bytecode1,bytecode2,..."))
            */
            case bytecode::ldsftn:{
                auto token = consume<uint32_t>();
                auto sftn = dynamic_cast<runtime::Method*>(call_stack.back().getHostedFunction()->getTable().query(token));
                operand.push(sftn);
                LOG_INST("ldsftn" << " " << sftn->qualifiedName())
                break;
            }
            case bytecode::ldvftn:{
                auto token = consume<uint32_t>();
                auto vftn = dynamic_cast<runtime::VirtualMethod*>(call_stack.back().getHostedFunction()->getTable().query(token));
                operand.push(vftn);
                LOG_INST("ldvftn " << vftn->qualifiedName())
                break;
            }
            case bytecode::ldftn:{
                auto token = consume<uint32_t>();
                auto ftn = dynamic_cast<runtime::Method*>(call_stack.back().getHostedFunction()->getTable().query(token));
                operand.push(ftn);
                LOG_INST("ldftn" << ftn->qualifiedName())
                break;
            }
            case bytecode::ldctor:{
                auto token = consume<uint32_t>();
                auto sym = call_stack.back().getHostedFunction()->getTable().query(token);
                auto ctor = dynamic_cast<runtime::Ctor*>(sym);
                operand.push<runtime::Ctor*>(ctor);
                LOG_INST("ldctor " << ctor->getClass()->qualifiedName() << ".Constructor")
                break;
            }
            case bytecode::ldforeign:{
                auto token = consume<uint32_t>();
                auto ff = dynamic_cast<runtime::ForeignEntry*>(call_stack.back().getHostedFunction()->getTable().query(token));
                operand.push(ff);
                LOG_INST("ldforeign " << ff->qualifiedName())
                break;
            }
            case bytecode::callmethod:{
                auto ftn = dynamic_cast<runtime::Method*>(operand.pop<runtime::Symbol*>());
                invokeMethod(ftn);
                LOG_INST("callmethod" << ftn->qualifiedName())
                break;
            }
            case bytecode::callvirtual:{
                auto vftn = dynamic_cast<runtime::VirtualMethod*>(operand.pop<runtime::Symbol*>());
                invokeVirtualMethod(vftn);
                LOG_INST("callvirtual " << vftn->qualifiedName())
                break;
            }
            case bytecode::callstatic:{
                auto ftn = dynamic_cast<runtime::Method*>(operand.pop<runtime::Symbol*>());
                invokeStaticMethod(ftn);
                LOG_INST("callstatic " << ftn->qualifiedName())
                break;
            }
            case bytecode::newobj:{
                auto tok = consume<token_t>();
                auto klass = dynamic_cast<runtime::Class*>(call_stack.back().getHostedFunction()->getTable().query(tok));
                auto ins = loader.getGC()->allocate(klass);
                //设置ClassInstance.Klass
                ins->klass = klass;
                operand.push<interop::Instance*>(ins);
                OpAddRoot<interop::Instance*>();
                LOG_INST("newobj " << klass->qualifiedName())
                break;
            }
            case bytecode::callctor:{
                auto ctor = operand.pop<runtime::Ctor*>();
                invokeCtor(ctor);
                LOG_INST("callctor " << ctor->getClass()->qualifiedName())
                break;
            }
            case bytecode::callforeign:{
                LOG_INST("callforeign ")
                auto fe = getOperand().pop<runtime::ForeignEntry*>();
                getLoader().getFFI()->call(fe, this);
                break;
            }
            case bytecode::callintrinsic:{
                auto tok = consume<token_t>();
                auto name = dynamic_cast<TextToken*>(call_stack.back().getHostedFunction()->getTable().getToken(tok))->getText();
                auto intrinsic = loader.getInteropAgent()->getInstrinsicByName(name);
                LOG_INST("callintrinsic " << name);
                if(intrinsic!=interop::Intrinsic::NotFound){
                    loader.getInteropAgent()->callIntrinsic(intrinsic, this);
                }
                else{
                    auto msg = loader.getInteropAgent()->createString("Intrinsic '"_utf32 + name + "' not found."_utf32);
                    auto ins = loader.getInteropAgent()->createInstance(loader.getEBEvmInternalException(), {
                        interop::Value::fromRef(std::move(msg))
                    });
                    handleException(std::move(ins));
                }
                break;
            }
            case bytecode::starg:{
                ForEachTypeWithRefFlag(OpStarg)
                break;
            }
            case bytecode::ldarg:{
                ForEachTypeWithRefFlag(OpLdarg)
                break;
            }
            case bytecode::ldarga:{
                auto idx = operand.pop<uint16_t>();
                auto hosted = call_stack.back().getHostedFunction();
                if(hosted->getParameterByIndex(idx)->getKind() == runtime::ParameterKind::Optional){
                    optionalParameterCheck(call_stack.back(),idx);
                }
                auto offset = call_stack.back().getHostedFunction()->getParameterByIndex(idx)->getOffset();
                auto address = call_stack.back().getMemory() + offset;
                operand.push<interop::InteriorPointer>(interop::makeInteriorPointer(address));
                OpAddRoot<interop::InteriorPointer>();
                LOG_INST("ldarga " << idx);
                break;
            }
            case bytecode::stloc:{
                ForEachTypeWithRefFlag(OpStloc);
                break;
            }
            case bytecode::ldloc:{
                ForEachTypeWithRefFlag(OpLdloc);
                break;
            }
            case bytecode::testopt:{
                auto count = operand.pop<uint8_t>();
                bool result = true;
                while(count--){
                    auto idx = operand.pop<uint16_t>();
                    if(result==true){
                        auto optional = dynamic_cast<const runtime::OptionalParameter*>(call_stack.back().getHostedFunction()->getParameterByIndex(idx));
                        auto offset = optional->getFlagOffset();
                        auto base = call_stack.back().getMemory();
                        if(*(base + offset)==0)result = false;
                    }
                }
                operand.push<uint8_t>(result);
                LOG_INST("testopt")
                break;
            }
            case bytecode::ldloca:{
                auto idx = operand.pop<uint16_t>();
                auto offset = call_stack.back().getHostedFunction()->getLocalOffset(idx);
                auto address = call_stack.back().getMemory() + offset;
                operand.push<interop::InteriorPointer>(interop::makeInteriorPointer(address));
                OpAddRoot<interop::InteriorPointer>();
                LOG_INST("ldloca " << idx)
                break;
            }
            case bytecode::stfld:{
                ForEachTypeWithRefFlag(OpStfld);
                break;
            }
            case bytecode::ldfld:{
                ForEachTypeWithRefFlag(OpLdfld);
                break;
            }
            case bytecode::ldflda:{
                auto tok = consume<token_t>();
                uint8_t hint = operand.pop<uint8_t>();
                auto fld = (runtime::Variable*)call_stack.back().getHostedFunction()->getTable().query(tok);
                LOG_INST("ldflda " << fld->qualifiedName())
                if(hint==1){ // ins
                    OpRemoveRoot<interop::Instance*>();
                    auto ins = operand.pop<interop::Instance*>();
                    auto offset = fld->getOffset();
                    operand.push<interop::InteriorPointer>(interop::makeInteriorPointer(ins,offset));
                    OpAddRoot<interop::InteriorPointer>();              
                }
                else if(hint==2){ // hld
                    OpRemoveRoot<interop::InteriorPointer>();
                    auto itp = operand.pop<interop::InteriorPointer>();
                    auto ptr = itp.ptr + fld->getOffset();
                    operand.push<interop::InteriorPointer>(interop::makeInteriorPointer(ptr));
                    OpAddRoot<interop::InteriorPointer>();
                }
                else if(hint==3){ // record
                    throw "";
                }
                break;
            }
            case bytecode::stsfld:{
                ForEachTypeWithRefFlag(OpStsfld);
                break;
            }
            case bytecode::ldsfld:{
                ForEachTypeWithRefFlag(OpLdsfld);
                break;
            }
            case bytecode::ldsflda:{
                auto tok = consume<token_t>();
                auto fld = (runtime::Variable*)call_stack.back().getHostedFunction()->getTable().query(tok);
                auto addr = fld->getStaticAddress();
                operand.push<interop::InteriorPointer>(interop::makeInteriorPointer(addr));
                OpAddRoot<interop::InteriorPointer>();
                LOG_INST("ldsflda " << fld->qualifiedName())
                break;
            }
            case bytecode::packopt:{
                auto tok = consume<token_t>();
                operand.push<token_t>(tok);
                LOG_INST("packopt " << call_stack.back().getHostedFunction()->getTable().query(tok)->qualifiedName())
                break;
            }
            case bytecode::stelem:{
                ForEachTypeWithRefFlag(OpStelem);
                break;
            }
            case bytecode::stelemr:{
                ForEachTypeWithRefFlag(OpStelemr);
                break;
            }
            case bytecode::ldelem:{
                ForEachTypeWithRefFlag(OpLdelem);
                break;
            }
            case bytecode::ldelema:{
                LOG_INST("ldelema")
                auto tok = consume<token_t>();
                auto idx = operand.pop<int32_t>();
                OpRemoveRoot<interop::Instance*>();
                auto ins = operand.pop<interop::ArrayInstance*>();
                if (nullPointerCheck((interop::Instance*)ins) & arrayAccessCheck(ins, idx)) {
                    auto base_offset = sizeof(interop::ArrayInstance);
                    auto element_sym = call_stack.back().getHostedFunction()->getTable().query(tok);
                    uint32_t offset = base_offset + idx * runtime::getRuntimeSize(element_sym);
                    operand.push<interop::InteriorPointer>(interop::makeInteriorPointer((interop::Instance*)ins,offset));
                    OpAddRoot<interop::InteriorPointer>();
                }
                break;
            }
            case bytecode::newarray:{
                LOG_INST("newarray")
                auto tok = consume<token_t>();
                auto array_length = operand.pop<int32_t>();
                auto element_sym = call_stack.back().getHostedFunction()->getTable().query(tok);
                auto ins = loader.getInteropAgent()->createUnprotectedArray(loader.getSpecilizedArrayPool()->query(element_sym), array_length);
                operand.push<interop::ArrayInstance*>(ins);
                OpAddRoot<interop::Instance*>();
                break;
            }
            case bytecode::arraylength:{
                //deprecated
                DEBUG_INSTRUCTION(arraylength)
                auto ref = operand.pop<uint8_t*>();
                int32_t length = *((int32_t*)(ref - sizeof(int32_t)));
                operand.push<int32_t>(length);
                break;
            }
            case bytecode::jif:{
                auto offset = consume<uint32_t>();
                auto cond = operand.pop<uint8_t>();
                LOG_INST("jif " << offset)
                if(cond) call_stack.back().ip = call_stack.back().getHostedFunction()->getBlock() + offset;
                break;
            }
            case bytecode::br:{
                auto offset = consume<uint32_t>();
                LOG_INST("br " << offset)
                call_stack.back().ip = call_stack.back().getHostedFunction()->getBlock() + offset;
                break;
            }
            case bytecode::ret:{
                LOG_INST("ret")
                if(&call_stack.back() == top_frame) exit = true;
                for(auto live_id : call_stack.back().getLiveSet()){
                    loader.getGC()->removeRoot(live_id);
                }
                call_stack.pop_back();
                break;
            }
            case bytecode::nop:{
                break;
            }
            case bytecode::dup:{
                ForEachTypeWithRefFlag(OpDup);
                break;
            }
            case bytecode::push:{
                ForEachTypeWithRefFlag(OpPush);
                break;
            }
            case bytecode::store:{
                ForEachTypeWithRefFlag(OpStore);
                break;
            }
            case bytecode::load:{
                ForEachTypeWithRefFlag(OpLoad);
                break;
            }
            case bytecode::ldnothing:{
                LOG_INST("ldnothing")
                operand.push((interop::Instance*)nullptr);
                OpAddRoot<interop::Instance*>();
                break;
            }
            case bytecode::convert:{
                ForEachArithmeticType(OpConvert);
                break;
            }
            case bytecode::castClass:{
                LOG_INST("castClass")
                auto src_tok = consume<token_t>();
                auto dst_tok = consume<token_t>();
                OpRemoveRoot<interop::Instance*>();
                auto ins = (interop::Instance*)operand.pop<uint8_t*>();
                auto target_class = (runtime::Class*)call_stack.back().getHostedFunction()->getTable().query(dst_tok);
                if(isInstanceOf(ins,target_class)){
                    operand.push<interop::Instance*>(ins);
                    OpAddRoot<interop::Instance*>();
                }
                else{
                    auto source_class = (runtime::Class*)call_stack.back().getHostedFunction()->getTable().query(src_tok);
                    auto src_str = loader.getInteropAgent()->createString(source_class->name);
                    auto dst_str = loader.getInteropAgent()->createString(target_class->name);
                    auto ins = loader.getInteropAgent()->createInstance(loader.getEBConverstionException(),{
                                                                                interop::Value::fromRef(std::move(src_str)),
                                                                                interop::Value::fromRef(std::move(dst_str))});
                    handleException(std::move(ins));
                }
                break;
            }
            case bytecode::instanceof:{
                LOG_INST("instanceof")
                auto tok = consume<token_t>();
                OpRemoveRoot<interop::Instance*>();
                auto ins = operand.pop<interop::Instance*>();
                auto klass = (runtime::Class*)call_stack.back().getHostedFunction()->getTable().query(tok);
                operand.push<uint8_t>(isInstanceOf(ins,klass));
                break;
            }
            case bytecode::throw_:{
                LOG_INST("throw")
                OpRemoveRoot<interop::Instance*>();
                auto ins = operand.pop<interop::Instance*>();
                handleException(getLoader().getGC()->makeProtectedCell(ins));
                break;
            }
            case bytecode::enter:{
                LOG_INST("enter")
                auto tok = consume<token_t>();
                auto offset = consume<uint32_t>();
                auto exception_class = (runtime::Class*)call_stack.back().getHostedFunction()->getTable().query(tok);
                exception_handler.push(exception_class, &call_stack.back(), call_stack.back().getHostedFunction()->getBlock() + offset);
                break;
            }
            case bytecode::leave:{
                LOG_INST("leave")
                auto tok = consume<token_t>();
                auto exception_class = (runtime::Class*)call_stack.back().getHostedFunction()->getTable().query(tok);
                exception_handler.pop();
                break;
            }
            case bytecode::add:{
                ForEachArithmeticType(OpAdd);
                break;
            }
            case bytecode::sub:{
                ForEachArithmeticType(OpSub);
                break;
            }
            case bytecode::mul:{
                DEBUG_INSTRUCTION(mul)
                ForEachArithmeticType(OpMul);
                break;
            }
            case bytecode::div:{
                ForEachArithmeticType(OpDiv);
                break;
            }
            case bytecode::and_:{
                LOG_INST("and")
                auto rhs = operand.pop<uint8_t>();
                auto lhs = operand.pop<uint8_t>();
                operand.push<uint8_t>(lhs & rhs);
                break;
            }
            case bytecode::or_:{
                LOG_INST("or")
                auto rhs = operand.pop<uint8_t>();
                auto lhs = operand.pop<uint8_t>();
                operand.push<uint8_t>(lhs | rhs);
                break;
            }
            case bytecode::xor_:{
                LOG_INST("xor")
                auto rhs = operand.pop<uint8_t>();
                auto lhs = operand.pop<uint8_t>();
                operand.push<uint8_t>(lhs xor rhs);
                break;
            }
            case bytecode::eq:{
                ForEachTypeWithOrd(OpEQ);
                break;
            }
            case bytecode::ne:{
                ForEachTypeWithOrd(OpNE);
                break;
            }
            case bytecode::lt:{
                ForEachArithmeticType(OpLT);
                break;
            }
            case bytecode::gt:{
                ForEachArithmeticType(OpGT);
                break;
            }
            case bytecode::le:{
                ForEachArithmeticType(OpLE);
                break;
            }
            case bytecode::ge:{
                ForEachArithmeticType(OpGE);
                break;
            }
            case bytecode::neg:{
                ForEachArithmeticType(OpNeg);
                break;
            }
            case bytecode::not_:{
                LOG_INST("not")
                auto rhs = operand.pop<uint8_t>();
                operand.push(!rhs);
                break;
            }
            case bytecode::ldstr:{
                auto tok = consume<token_t>();
                auto utf8str = dynamic_cast<TextToken*>(call_stack.back().getHostedFunction()->getTable().getToken(tok))->getText();
                auto ins = loader.getInteropAgent()->createUnprotectedString(utf8str);
                operand.push(ins);
                OpAddRoot<interop::Instance*>();
                LOG_INST("ldstr " << utf8str)
                break;
            }
            case bytecode::ldoptinfo:{
                LOG_INST("ldoptinfo")
                auto tok = consume<token_t>();
                auto option = dynamic_cast<runtime::OptionalParameter*>(call_stack.back().getHostedFunction()->getTable().query(tok));
                operand.push(option);
                break;
            }
            case bytecode::ldenumc:{
                LOG_INST("ldenumc")
                auto tok = consume<token_t>();
                auto constant = dynamic_cast<runtime::EnumConstant*>(call_stack.back().getHostedFunction()->getTable().query(tok));
                operand.push<runtime::EnumConstant*>(constant);
                break;
            }
            case bytecode::mod:{
                ForEachIntegralType(OpMod);
                break;
            }
            case bytecode::wrapctor:{
                auto ctor = operand.pop<runtime::Ctor*>();
                interop::Delegate dlg;
                dlg.kind = interop::DelegateKind::Ctor;
                dlg.function = ctor;
                operand.push<interop::Delegate>(dlg);
                OpAddRoot<interop::Delegate>();
                break;
            }
            case bytecode::wrapforeign:{
                auto ff = operand.pop<runtime::ForeignEntry*>();
                interop::Delegate dlg;
                dlg.kind = interop::DelegateKind::Ctor;
                dlg.function = ff;
                operand.push<interop::Delegate>(dlg);
                OpAddRoot<interop::Delegate>();
                break;
            }
            case bytecode::wrapsftn:{
                auto sftn = operand.pop<runtime::Method*>();
                interop::Delegate dlg;
                dlg.kind = interop::DelegateKind::Ctor;
                dlg.function = sftn;
                operand.push<interop::Delegate>(dlg);
                OpAddRoot<interop::Delegate>();
                break;
            }
            case bytecode::wrapftn:{
                OpRemoveRoot<interop::Instance*>();
                auto ins = operand.pop<interop::Instance*>();
                auto ftn = operand.pop<runtime::Method*>();
                interop::Delegate dlg;
                dlg.kind = interop::DelegateKind::Ctor;
                dlg.function = ftn;
                dlg.instance = ins;
                operand.push<interop::Delegate>(dlg);
                OpAddRoot<interop::Delegate>();
                break;
            }
            case bytecode::wrapvftn:{
                OpRemoveRoot<interop::Instance*>();
                auto ins = operand.pop<interop::Instance*>();
                auto vftn = operand.pop<runtime::VirtualMethod*>();
                interop::Delegate dlg;
                dlg.kind = interop::DelegateKind::Ctor;
                dlg.function = vftn;
                dlg.instance = ins;
                operand.push<interop::Delegate>(dlg);
                OpAddRoot<interop::Delegate>();
                break;
            }
            case bytecode::calldlg:{
                OpRemoveRoot<interop::Delegate>();
                auto dlg = operand.pop<interop::Delegate>();
                switch(dlg.kind){
                    case interop::DelegateKind::Ctor:{
                        auto ctor = dynamic_cast<runtime::Ctor*>(dlg.function);
                        auto klass = dynamic_cast<runtime::Class*>(ctor->parent);
                        auto ins = loader.getGC()->allocate(klass);
                        //设置ClassInstance.Klass
                        ins->klass = klass;
                        operand.push<interop::Instance*>(ins);
                        OpAddRoot<interop::Instance*>();
                        invokeCtor(ctor);
                        break;
                    }
                    case interop::DelegateKind::Foreign:{
                        auto fe = dynamic_cast<runtime::ForeignEntry*>(dlg.function);
                        getLoader().getFFI()->call(fe, this);
                        break;
                    }
                    case interop::DelegateKind::Ftn:{
                        auto ftn = dynamic_cast<runtime::Method*>(dlg.function);
                        invokeMethod(ftn);
                        break;
                    }
                    case interop::DelegateKind::VFtn:{
                        auto vftn = dynamic_cast<runtime::VirtualMethod*>(dlg.function);
                        invokeVirtualMethod(vftn);
                        break;
                    }
                    case interop::DelegateKind::SFtn:{
                        auto ftn = dynamic_cast<runtime::Method*>(dlg.function);
                        invokeStaticMethod(ftn);
                        break;
                    }
                }
                break;
            }
            case bytecode::pop:{
                ForEachType(OpPop);
                break;
            }
            default: throw std::invalid_argument("unexpected bytecode " + std::to_string(code));
        }
    }
}

