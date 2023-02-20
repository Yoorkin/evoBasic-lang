#include "runtime.h"
#include "backage.pb.h"
#include "bytecode.h"
#include "interop.h"
#include "loader.h"
#include "unicode.h"
#include "ebffi.h"
#include <cstddef>
#include <cstdlib>
#include <exception>
#include <google/protobuf/generated_enum_reflection.h>
#include <google/protobuf/repeated_ptr_field.h>
#include <stdexcept>
#include <utility>

namespace runtime {

    uint32_t getRuntimeSize(Symbol *target){
        if(instancesOf<Class>(target)) // class 也包括 specializedArray
            return sizeof(void*);
        else if(auto pmt = dynamic_cast<Primitive*>(target))
            return pmt->getSize();
        else if(auto rcd = dynamic_cast<Record*>(target))
            return rcd->getMemorySize();
        else if(instancesOf<Enumeration>(target) || instancesOf<EnumConstant>(target))
            return sizeof(void*);
        else
            throw std::invalid_argument("");
    }

    uint32_t Primitive::getSize()const{
        switch(kind){
            case PrimitiveKind::Boolean: return 1;
            case PrimitiveKind::Byte:    return 1;
            case PrimitiveKind::Short:   return 2;
            case PrimitiveKind::UShort:  return 2;
            case PrimitiveKind::Rune:    return 4;
            case PrimitiveKind::Integer: return 4;
            case PrimitiveKind::UInteger:return 4;
            case PrimitiveKind::Long:    return 8;
            case PrimitiveKind::ULong:   return 8;
            case PrimitiveKind::Single:  return 4;
            case PrimitiveKind::Double:  return 8;
            default: throw std::invalid_argument("unexpected primitive kind in Primitive::getSize");
        }
    }

    unicode::string Symbol::qualifiedName() {
        if (instancesOf<Global>(parent) || parent == nullptr)return name;
        else return parent->qualifiedName() + "."_utf32 + name;
    }

    void Global::complete(){
        this->static_memory_size = 0;
        std::list<std::pair<uint32_t,Variable*>> variables;
        for(auto [_,child] : childern){
            if(auto variable = dynamic_cast<Variable*>(child)){
                variables.push_back({static_memory_size,variable});
                static_memory_size += getRuntimeSize(variable->getType());
            }
        }
        static_memory = (uint8_t*)malloc(static_memory_size);
        for(auto [offset,variable] : variables){
            variable->setStaticAddress(static_memory+offset);
        }
    }

    void Global::add(Symbol *symbol){
        if(runtime::isNamedMain(symbol))
            main = dynamic_cast<Method*>(symbol);
        symbol->parent = this;
        childern.insert({symbol->name,symbol});
        this->debug_childern.insert({symbol->debug_name,symbol});
    }

    uint8_t *Global::getStaticMemory(){
        if(static_memory)return static_memory;
        else throw "global static memory not initialized";
    }

    uint32_t Global::getStaticMemorySize(){
        if(static_memory_size!=-1)return static_memory_size;
        else throw "";
    }

    void Module::complete(){
        this->static_memory_size = 0;
        std::list<std::pair<uint32_t,Variable*>> variables;
        for(auto [_,child] : childern){
            if(auto variable = dynamic_cast<Variable*>(child)){
                variables.push_back({static_memory_size,variable});
                static_memory_size += getRuntimeSize(variable->getType());
            }
        }
        static_memory = (uint8_t*)malloc(static_memory_size);
        for(auto [offset,variable] : variables){
            variable->setStaticAddress(static_memory+offset);
        }
    }

    void Class::completeFieldAndVFtn(Class *base){
        if(isNamedObject(this)){
            this->instance_memory_size = 0;
        }
        else{
            base_class = base;
            this->instance_memory_size = base_class->getInstanceMemorySize();
            this->instance_ref_offsets = base_class->instance_ref_offsets;
            this->virtual_table_map = base_class->virtual_table_map;
            this->virtual_table = base_class->virtual_table;
        }
        this->static_memory_size = 0;
        
        std::list<std::pair<uint32_t,Variable*>> variables;

        for(auto [name,child] : childern){
            if(auto variable = dynamic_cast<Variable*>(child)){
                if(variable->getFlag() & bytecode::flag_static){
                    variables.push_back({static_memory_size,variable});
                    static_memory_size += getRuntimeSize(variable->getType());
                }
                else{
                    if(instancesOf<runtime::Class>(variable->getType())){
                        instance_ref_offsets.push_back(instance_memory_size);
                    }
                    auto size = getRuntimeSize(variable->getType());
                    variable->setOffset(instance_memory_size);
                    variable->setLength(size);
                    instance_memory_size += size;
                }
            }
            else if(auto method = dynamic_cast<Method*>(child)){
                method->complete();
                if(method->getFlag() & bytecode::flag_virtual){
                    auto offset = virtual_table.size();
                    virtual_table.push_back(method);
                    virtual_table_map[name] = offset;
                    childern[name] = new VirtualMethod(name,offset,method,this);
                }
                else if(method->getFlag() & bytecode::flag_override){
                    auto offset = virtual_table_map[name];
                    virtual_table[offset] = method;
                    childern[name] = new VirtualMethod(name,offset,method,this);
                }
            }
        }

        static_memory = (uint8_t*)malloc(static_memory_size);
        for(auto [offset,variable] : variables){
            variable->setStaticAddress(static_memory+offset);
        }
    }

    
    Ctor *Class::getCtor(){ 
        auto target = childern.find("#ctor"_utf32);
        if(target == childern.end()) 
            throw std::invalid_argument(unicode::toPlatform(name) + ".ctor not found");
        return dynamic_cast<Ctor*>(target->second); 
    }

    uint32_t Class::getInstanceMemorySize()const{
        if(instance_memory_size==-1)throw "";
        return instance_memory_size;
    }

    uint32_t Class::getStaticMemorySize()const{
        if(static_memory_size==-1)throw "";
        return static_memory_size;
    }

    std::list<Symbol*> PackageDefinedClass::getDependencies(){
        if(isNamedObject(this))return {};
        return {table.query(decl.basetoken())};
    }

    void PackageDefinedClass::complete(){
        if(isNamedObject(this)){
            this->completeFieldAndVFtn(nullptr);
        }
        else{
            this->completeFieldAndVFtn(dynamic_cast<Class*>(table.query(decl.basetoken())));
        }
        getCtor()->setClass(this);
        getCtor()->complete();
    }

    void SpecializedArray::complete(){
        completeFieldAndVFtn(array_base_type);
    }

    std::list<Symbol*> SpecializedArray::getDependencies(){
        return {array_base_type};
    }

    void Record::complete(){
        this->record_memory_size = 0;
        for(auto [_,child] : childern){
            auto variable = dynamic_cast<Variable*>(child);
            auto size = getRuntimeSize(variable->getType());
            variable->setOffset(record_memory_size);
            variable->setLength(size);
            record_memory_size += size;
        }
    }
    
    std::list<Record*> Record::getDependencies(){
        std::list<Record*> ls;
        for(auto [_,child] : childern)
            if(instancesOf<Record>(child))
                ls.push_back(dynamic_cast<Record*>(child));
        return ls;
    }

    uint32_t Record::getMemorySize()const{
        if(record_memory_size==-1)throw "record_memory_size undefined";
        return record_memory_size;
    }

    uint32_t Variable::getOffset(){
        if(memory_offset==-1)throw "offset undefined";
        return memory_offset;
    }

    void Variable::setOffset(uint32_t value){
        if(memory_offset==-1)memory_offset = value;
        else throw "set offset twice";
    }

    Symbol *Variable::getType(){
        return table.query(decl.typetoken());
    }


    void Function::complete(){
        param_memory_size = 0;

        if(!(flag & bytecode::flag_static)){
            implicit_self = new NormalParameter("#Self"_utf32, 0, EvaluationKind::Byval, this->parent, Range(0,sizeof(interop::Instance*)), this);
            param_memory_size += sizeof(interop::Instance*);
            stackframe_ref_offsets.push_back(0);
        }

        uint32_t param_index = 0;

        for(auto param : params){
            param_index++;
            Symbol *type = table.query(param.typetoken());

            if(param.flag() & bytecode::flag_optional){
                auto ctd_token = dynamic_cast<ConstructedToken*>(table.getToken(param.nametoken()));
                auto name = table.getToken(ctd_token->getSubToken(ctd_token->getSubTokenCount()-1))->toString();

                EvaluationKind eval_kind;
                uint32_t flag_offset;
                Range value_range;

                if(param.flag() & bytecode::flag_byval){
                    eval_kind = EvaluationKind::Byval;
                    flag_offset = param_memory_size;
                    value_range.begin = param_memory_size + 1;
                    value_range.length = getRuntimeSize(type);
                    if(instancesOf<runtime::Class>(type)){
                        stackframe_ref_offsets.push_back(param_memory_size + 1);
                    }
                    param_memory_size += 1 + value_range.length;
                }
                else if(param.flag() & bytecode::flag_byref){
                    eval_kind = EvaluationKind::Byref;
                    flag_offset = param_memory_size;
                    value_range.begin = param_memory_size;
                    value_range.length = sizeof(interop::InteriorPointer);
                    stackframe_interior_pointer_offsets.push_back(param_memory_size);
                    param_memory_size += value_range.length;
                }
                auto optional = new OptionalParameter(name, param_index, eval_kind, type, flag_offset, value_range, this);
                childern.insert({name,optional});
                optional_parameters.push_back(optional);
            }
            else if(param.flag() & bytecode::flag_paramArray){
                auto name = table.getToken(param.nametoken())->toString();
                auto offset = Range(param_memory_size, sizeof(interop::Instance*));
                stackframe_ref_offsets.push_back(param_memory_size);
                param_array = new ParamArrayParameter(name, param_index, EvaluationKind::Byval, type, offset, this);
                param_memory_size += sizeof(interop::Instance*);
            }
            else{
                EvaluationKind eval_kind;
                uint32_t size;
                if(param.flag() & bytecode::flag_byval){
                    eval_kind = EvaluationKind::Byval;
                    size = getRuntimeSize(type);
                    if(instancesOf<runtime::Class>(type)){
                        stackframe_ref_offsets.push_back(param_memory_size);
                    }
                }
                else if(param.flag() & bytecode::flag_byref){
                    eval_kind = EvaluationKind::Byref;
                    size = sizeof(interop::InteriorPointer);
                    stackframe_interior_pointer_offsets.push_back(param_memory_size);
                }
                auto name = table.getToken(param.nametoken())->toString();
                auto offset = Range(param_memory_size, size);
                normal_parameters.push_back(new NormalParameter(name, param_index, eval_kind, type, offset, this));
                param_memory_size += size;
            }
        }

        for(auto optional : optional_parameters){
            childern.insert({optional->name,optional});
        }

        if(return_type_token == 0){ // Ctor
            return_type = parent;
        }
        else{ // Method,ForeignFunction
            return_type = table.query(return_type_token);
        }
    }

    void HostedFunction::generateLineTable(const google::protobuf::RepeatedPtrField<Backage::LineNumber> &lineNumbers){
        std::vector<LineNumber> numbers;
        if(lineNumbers.size()>0){
            int begin = 0;
            for(int i=0; i<lineNumbers.size()-1; i++){
                numbers.push_back(LineNumber(lineNumbers[i].line(),begin,lineNumbers[i+1].offset()));
                begin = lineNumbers[i+1].offset();
            }
            numbers.push_back(LineNumber(lineNumbers[lineNumbers.size()-1].line(), begin, block.size()));
        }
        this->lineNumberTable = new LineNumberTable(numbers);
    }
    
    HostedFunction::HostedFunction(unicode::string name,TokenTable &table,const std::string &block,
                    const google::protobuf::RepeatedPtrField<Backage::ParameterDecl> &params,
                    const google::protobuf::RepeatedPtrField<Backage::LocalIndex> &locals,
                    const google::protobuf::RepeatedPtrField<Backage::LineNumber> &lineNumbers,
                    uint32_t return_type_token,
                    uint32_t flag) 
                    : Function(name, table, flag, params, return_type_token),locals(locals),block(block){
        generateLineTable(lineNumbers);
    }

    void HostedFunction::complete(){
        Function::complete();
        //计算local变量大小
        for(auto local : locals){
            auto type = table.query(local.typetoken());
            auto offset = local_memory_size + getParamMemorySize();
            local_offsets.push_back(offset);
            if(instancesOf<runtime::Class>(type)){
                stackframe_ref_offsets.push_back(offset);
            }
            local_memory_size += getRuntimeSize(type);
        }
    }

    unicode::string getPrimitiveKindString(PrimitiveKind kind){
        using enum PrimitiveKind;
        switch(kind){
            case Void:    return "#Void"_utf32;
            case Boolean: return "Boolean"_utf32;
            case Byte:    return "Byte"_utf32;
            case Short:   return "Short"_utf32;
            case UShort:  return "UShort"_utf32;
            case Rune:    return "Rune"_utf32;
            case Integer: return "Integer"_utf32;
            case UInteger:return "UInteger"_utf32;
            case Long:    return "Long"_utf32;
            case ULong:   return "ULong"_utf32;
            case Single:  return "Single"_utf32;
            case Double:  return "Double"_utf32;
            default: throw "unexpected primitive kind"_utf32;
        }
    }

}