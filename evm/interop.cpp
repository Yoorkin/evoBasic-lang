#include "interop.h"
#include "loader.h"
#include "processor.h"
#include "runtime.h"

namespace interop {



    ProtectedCell::ProtectedCell(GarbageCollector *gc, interop::Instance *ins) : gc(gc){
        obj = new SharedObj();
        obj->ins = ins;
        obj->shared_count = 1;
        gc->addRoot(&(obj->ins));
    }

    ProtectedCell::ProtectedCell(ProtectedCell &&that){
        this->gc = that.gc;
        this->obj = that.obj;
        that.obj = nullptr;
    }

    ProtectedCell::ProtectedCell(const ProtectedCell &that){
        this->gc = that.gc;
        this->obj = that.obj;
        obj->shared_count++;
    }

    ProtectedCell::~ProtectedCell() {
        if(obj!=nullptr){
            obj->shared_count--;
            if(obj->shared_count==0){
                gc->removeRoot(&(obj->ins));
                delete obj;
            }
        }
    }


    void Value::pushToStack(Processor *processor) const {
        auto& stack = processor->getOperand();
        switch (kind) {
            case Boolean:
                stack.push<uint8_t>(val.boolean_val);
                break;
            case I8:
                stack.push<int8_t>(val.i8_val);
                break;
            case I16:
                stack.push<int16_t>(val.i16_val);
                break;
            case I32:
                stack.push<int32_t>(val.i32_val);
                break;
            case I64:
                stack.push<int64_t>(val.i64_val);
                break;
            case U8:
                stack.push<uint8_t>(val.u8_val);
                break;
            case U16:
                stack.push<uint16_t>(val.u16_val);
                break;
            case U32:
                stack.push<uint32_t>(val.u32_val);
                break;
            case U64:
                stack.push<uint64_t>(val.u64_val);
                break;
            case F32:
                stack.push<float>(val.f32_val);
                break;
            case F64:
                stack.push<double>(val.f64_val);
                break;
            case Ref:
                stack.push<interop::Instance*>(((ProtectedCell&)ref_val.value()).get());
                processor->getLoader().getGC()->addRoot(stack.ptrToPeek<interop::Instance*>());
                break;
            default:
                throw std::invalid_argument("");
        }
    }

    Agent::Agent(Loader *loader){
        processor = new Processor(loader);
    }

    uint32_t getInstanceSize(interop::Instance *ins){
        auto size = ins->klass->getInstanceMemorySize();
        if(auto spec_ary = dynamic_cast<runtime::SpecializedArray*>(ins->klass)){
            size += ((interop::ArrayInstance*)ins)->length * runtime::getRuntimeSize(spec_ary->getElementType());
        }
        return size;
    }


    interop::Instance *Agent::createUnprotectedInstance(runtime::Class *klass, std::list<Value> parameters){
        auto cell = processor->getLoader().getGC()->makeProtectedCell(processor->getLoader().getGC()->allocate(klass));
        
        cell.get()->klass = klass;
        processor->getOperand().push<Instance*>(cell.get());
        processor->getLoader().getGC()->addRoot(processor->getOperand().ptrToPeek<interop::Instance*>());
        for(auto iter = parameters.rbegin(); iter!=parameters.rend(); iter++){
            (*iter).pushToStack(processor);
        }

        processor->invokeCtor(klass->getCtor());
        processor->execute();
        return cell.get();
    }

    interop::ArrayInstance *Agent::createUnprotectedArray(runtime::SpecializedArray *array, int count){
        auto content_length = count * runtime::getRuntimeSize(array->getElementType());
        auto ins = (ArrayInstance*)processor->getLoader().getGC()->allocate(array, array->getInstanceMemorySize() + content_length);
        ins->base.klass = array;
        ins->length = count;
        memset((char*)ins + sizeof(ArrayInstance), 0, content_length);
        return ins;
    }

    interop::StringInstance *Agent::createUnprotectedString(unicode::string str){
        auto rune_count = str.length();
        auto rune_symbol = processor->getLoader().getGlobal()->find("Rune"_utf32);
        auto array = createArray(processor->getLoader().getSpecilizedArrayPool()->query(rune_symbol), rune_count);

        auto array_mem = (uint32_t*)((uint8_t*)array.get() + sizeof(ArrayInstance));
        for(auto codepoint : str){
            *array_mem = codepoint;
            array_mem++;
        }
        auto string_class = dynamic_cast<runtime::Class*>(processor->getLoader().getEBString());
        auto ins = (interop::StringInstance*)createUnprotectedInstance(string_class, {Value::fromRef(std::move(array))});
        //std::cout<<"\nlength:"<<ins->runes->length<<std::endl;
        return ins;
    }

    ProtectedCell Agent::createInstance(runtime::Class *klass, std::list<Value> parameters){
        return processor->getLoader().getGC()->makeProtectedCell(createUnprotectedInstance(klass,std::move(parameters)));
    }

    ProtectedCell Agent::createArray(runtime::SpecializedArray *array, int count){
        return processor->getLoader().getGC()->makeProtectedCell((Instance*)createUnprotectedArray(array,count));
    }

    ProtectedCell Agent::createString(unicode::string str){
        return processor->getLoader().getGC()->makeProtectedCell((Instance*)createUnprotectedString(str));
    }

    unicode::string Agent::fetchStringFromInstance(StringInstance *instance){
        auto runeArray = instance->runes;
        unicode::codepoint *ptr = (unicode::codepoint*)(((uint8_t*)runeArray) + sizeof(ArrayInstance));
        return unicode::string(ptr,runeArray->length);
    }




    template<class Return,class ...Args>
    void handler_call_parameter_expand(Return(*handler)(Args...),MemoryStack *operand){
        std::tuple<Args...> args{(operand->pop<Args>())...};
        Return ret = std::apply(*handler,std::move(args));
        operand->push<Return>(ret);
    }

    template<class ...Args>
    void handler_call_parameter_expand(void(*handler)(Args...),MemoryStack *operand){
        std::tuple<Args...> args{(operand->pop<Args>())...};
        std::apply(*handler,std::move(args));
    }


    inline void putRune(uint32_t value){
        std::cout<<unicode::string(&value,1);
        LOG(PutRune,"'"<<unicode::string(&value, 1)<<"'"<<std::endl)
    }

    inline auto isIteratorNotInRange(int32_t sep, int32_t end,int32_t beg,int32_t iter) -> uint8_t {
        LOG(IterInRangeIntrinsic,"beg " << beg << ", end "<< end << ", sep "<<sep<<" , iter "<<iter<<std::endl)
        if(sep == 0) return false;
        else if(sep > 0) return beg > end || iter > end;
        else if(sep < 0) return beg < end || iter < end; 
        else throw "";
    }

    inline void debugInt(int32_t value){
        std::cout<<"                          # "<<value<<std::endl;
    }

    inline void debugBool(uint8_t value){
        std::cout<<"                          # "<<(value==0?"false":"true")<<std::endl;
    }

    inline void debugLong(int64_t value){
        std::cout<<"                          # "<<value<<std::endl;
    }


    void Agent::callIntrinsic(Intrinsic intrinsic, Processor *processor){
        using enum Intrinsic;
        switch(intrinsic){
            case ItNotInRange:
                handler_call_parameter_expand(isIteratorNotInRange, &processor->getOperand());
                break;
            case DebugBool:
                handler_call_parameter_expand(debugBool, &processor->getOperand());
                break;
            case DebugInt:
                handler_call_parameter_expand(debugInt, &processor->getOperand());
                break;
            case DebugLong:
                handler_call_parameter_expand(debugLong, &processor->getOperand());
                break;
            case PutRune:
                handler_call_parameter_expand(putRune, &processor->getOperand());
                break;
            case BooleanToString:{
                bool boolean = processor->getOperand().pop<uint8_t>();
                auto ins = processor->getLoader().getInteropAgent()->createString(boolean ? "True"_utf32:"False"_utf32);
                processor->getOperand().push(ins.get());
                processor->OpAddRoot<Instance*>();
                break;
            }
            case ByteToString:{
                auto value = processor->getOperand().pop<uint8_t>();
                auto ins = processor->getLoader().getInteropAgent()->createString(unicode::to_string(value));
                processor->getOperand().push(ins.get());
                processor->OpAddRoot<Instance*>();
                break;
            }
            case ShortToString:{
                auto value = processor->getOperand().pop<int16_t>();
                auto ins = processor->getLoader().getInteropAgent()->createString(unicode::to_string(value));
                processor->getOperand().push(ins.get());
                processor->OpAddRoot<Instance*>();
                break;
            }
            case IntegerToString:{
                auto value = processor->getOperand().pop<int32_t>();
                //std::cout<<std::endl<<"@ "<< unicode::to_string(value) <<std::endl;
                auto ins = processor->getLoader().getInteropAgent()->createString(unicode::to_string(value));
                processor->getOperand().push(ins.get());
                processor->OpAddRoot<Instance*>();
                break;
            }
            case LongToString:{
                auto value = processor->getOperand().pop<int64_t>();
                auto ins = processor->getLoader().getInteropAgent()->createString(unicode::to_string(value));
                processor->getOperand().push(ins.get());
                processor->OpAddRoot<Instance*>();
                break;
            }
            case SingleToString:{
                auto value = processor->getOperand().pop<float>();
                auto ins = processor->getLoader().getInteropAgent()->createString(unicode::to_string(value));
                processor->getOperand().push(ins.get());
                processor->OpAddRoot<Instance*>();
                break;
            }
            case DoubleToString:{
                auto value = processor->getOperand().pop<double>();
                auto ins = processor->getLoader().getInteropAgent()->createString(unicode::to_string(value));
                processor->getOperand().push(ins.get());
                processor->OpAddRoot<Instance*>();
                break;
            }
            case DebugObjAddr: {
                auto val = processor->getOperand().pop<interop::Instance*>();
                std::cout<< "# " << val << std::endl;
                break;
            }
            case Trap:{
                auto count = processor->getOperand().pop<uint32_t>();
                processor->OpRemoveRoot<Instance*>();
                auto ins = processor->getOperand().pop<interop::Instance*>();
                break;
            }
            case StringToCStr:{
                processor->OpRemoveRoot<Instance*>();
                auto ins = processor->getOperand().pop<Instance*>();
                auto str = fetchStringFromInstance((StringInstance*)ins);
                auto cstr = unicode::toPlatform(str);
                auto byte_symbol = processor->getLoader().getGlobal()->find("Byte"_utf32);
                auto ary = createArray(processor->getLoader().getSpecilizedArrayPool()->query(byte_symbol),cstr.size());
                auto ptr = (char*)((uint8_t*)ary.get() + sizeof(ArrayInstance));
                for(int i=0; i<cstr.size(); i++){
                    *ptr = cstr[i];
                    ptr++;
                }
                processor->getOperand().push<Instance*>(ary.get());
                processor->OpAddRoot<Instance*>();
                break;
            }
            case Pin:{
                processor->OpRemoveRoot<Instance*>();
                auto ins = processor->getOperand().pop<Instance*>();
                processor->getLoader().getGC()->pin(ins);
                break;
            }
            case Unpin:{
                processor->OpRemoveRoot<Instance*>();
                auto ins = processor->getOperand().pop<Instance*>();
                processor->getLoader().getGC()->unpin(ins);
                break;
            }
            case EnableGC:{
                break;
            }
            case DisableGC:{
                break;
            }
            case AryPtr:{
                processor->OpRemoveRoot<Instance*>();
                auto ins = processor->getOperand().pop<Instance*>();
                if(ins->pined==0){
                
                }
                else{
                    auto ptr = (uintptr_t)((uint8_t*)ins + sizeof(Instance));
                    processor->getOperand().push<uintptr_t>(ptr);
                }
                break;
            }
            case ObjPtr:{
                processor->OpRemoveRoot<Instance*>();
                auto ins = processor->getOperand().pop<Instance*>();
                if(ins->pined==0){
                    auto ptr = (uintptr_t)((uint8_t*)ins + sizeof(Instance));
                    processor->getOperand().push<uintptr_t>(ptr);
                }
                else{
                    processor->getOperand().push<uintptr_t>((uintptr_t)ins);
                }
                break;
            }
            case RefPtr:{
                processor->OpRemoveRoot<Instance*>();
                auto itp = processor->getOperand().pop<InteriorPointer>();
                if(getInteriorPointerInstance(itp)->pined==0){
                    auto ins = createInstance(processor->getLoader().getEBObjectUnpinnedException(),{});
                    processor->handleException(ins);
                }
                else{
                    processor->getOperand().push<uintptr_t>((uintptr_t)itp.ptr);
                }
                break;
            }
            default: throw "";
        }
    }

    std::map<unicode::string,Intrinsic> intrinsic_map = {
        {"DebugInt"_utf32,Intrinsic::DebugInt},
        {"IsIteratorNotInRange"_utf32,Intrinsic::ItNotInRange},
        {"Len"_utf32,Intrinsic::Len},
        {"DebugLong"_utf32,Intrinsic::DebugLong},
        {"PutRune"_utf32,Intrinsic::PutRune},
        {"BooleanToString"_utf32,Intrinsic::BooleanToString},
        {"ByteToString"_utf32,Intrinsic::ByteToString},
        {"UShortToString"_utf32,Intrinsic::UShortToString},
        {"ShortToString"_utf32,Intrinsic::ShortToString},
        {"UIntegerToString"_utf32,Intrinsic::UIntegerToString},
        {"IntegerToString"_utf32,Intrinsic::IntegerToString},
        {"ULongToString"_utf32,Intrinsic::ULongToString},
        {"LongToString"_utf32,Intrinsic::LongToString},
        {"SingleToString"_utf32,Intrinsic::SingleToString},
        {"DoubleToString"_utf32,Intrinsic::DoubleToString},
        {"GetCallStackTrace"_utf32,Intrinsic::GetCallStackTrace},
        {"DebugObjAddr"_utf32,Intrinsic::DebugObjAddr},
        {"Trap"_utf32,Intrinsic::Trap},
        {"StringToCStr"_utf32,Intrinsic::StringToCStr},
        {"Pin"_utf32,Intrinsic::Pin},
        {"Unpin"_utf32,Intrinsic::Unpin},
        {"DisableGC"_utf32,Intrinsic::DisableGC},
        {"EnableGC"_utf32,Intrinsic::EnableGC},
        {"AryPtr"_utf32,Intrinsic::AryPtr},
        {"ObjPtr"_utf32,Intrinsic::ObjPtr},
        {"RefPtr"_utf32,Intrinsic::RefPtr}
    };

    Intrinsic Agent::getInstrinsicByName(unicode::string name){
        auto target = intrinsic_map.find(name);
        if(target == intrinsic_map.end())return Intrinsic::NotFound;
        else return target->second;
    }


}
