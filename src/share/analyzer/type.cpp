//
// Created by yorkin on 7/17/21.
//

#include "type.h"
#include "semantic.h"

#include <utility>
#include <vector>
#include <map>
#include <string>
#include <utils/logger.h>
#include <utils/utils.h>
#include <utils/enums.h>
#include <utils/nullSafe.h>
#include <loader/bytecode.h>

using namespace std;
namespace evoBasic::type{

    vector AccessFlagString = {"Public","Private","Friend","Protected"};
    vector FunctionFlagString = {"Method","Static","Virtual","Override"};


    void write_ptr(stringstream &stream,data::ptr ptr){
        stream.write((const char*)&ptr,vm::Data::ptr.getSize());
    }
    
    void strToLowerByRef(string& str){
        transform(str.begin(),str.end(),str.begin(),[](unsigned char c){ return std::tolower(c); });
    }


    void Module::addImport(Symbol *child) {
        if(auto mod = dynamic_cast<Module*>(child)){
            importedModule.push_back(mod);
        }
        else{
            PANIC;
        }
    }


    bool Class::equal(Prototype *ptr) {
        return ptr==this;
    }



    bool Enumeration::equal(Prototype *ptr) {
        return ptr==this;
    }


    Enumeration::Enumeration() : Class(SymbolKind::Enum) {
        setByteLength(vm::Data::ptr.getSize());
    }


    void Enumeration::add(Symbol *symbol) {
        ASSERT(symbol->getKind() != SymbolKind::EnumMember,"symbol is not a EnumMember");
        Domain::add(symbol);
    }


    Parameter::Parameter(std::string name,Prototype *prototype, bool isByval, bool isOptional ,bool isParamArray)
            : Variable(SymbolKind::Parameter), is_byval(isByval), is_optional(isOptional), is_param_array(isParamArray){
        setName(move(name));
        if(isParamArray)is_byval = true;
        if(prototype)setPrototype(prototype);
        toggleParameter();
    }

    bool Parameter::isByval() {
        return is_byval;
    }

    bool Parameter::isOptional() {
        return is_optional;
    }

    bool Parameter::isParamArray() {
        return is_param_array;
    }

    data::ptr Parameter::getRealByteLength() {
        if(isByval()){
            return Variable::getRealByteLength();
        }
        else{
            return vm::Data::ptr.getSize();
        }
    }

    bool Parameter::equal(Parameter *ptr) {
        if(ptr->isByval()!=isByval())return false;
        if(ptr->isParamArray()!=isParamArray())return false;
        if(ptr->isOptional()!=isOptional())return false;
        if(!ptr->getPrototype()->equal(getPrototype()))return false;
        return true;
    }

    ast::Argument *Parameter::getDefaultArgument(){
        NotNull(default_argument);
        return default_argument;
    }

    void Parameter::setDefaultArgument(ast::Argument *argument) {
        ASSERT(!is_optional,"setInitial failed.Parameter should be optional");
        this->default_argument = argument;
    }


    Prototype *Function::getRetSignature() {
        return this->ret_signature;
    }

    void Function::setRetSignature(Prototype *ptr){
        NotNull(ptr);
        ret_signature = ptr;
    }

    bool Function::equal(Prototype *ptr) {
        if(auto function = ptr->as<Function*>()){
            auto param_count = this->getArgsSignature().size();
            auto function_param_count = function->getArgsSignature().size();
            if(param_count!=function_param_count)return false;
            for(int i=0;i<param_count;i++){
                if(!getArgsSignature()[i]->equal(function->getArgsSignature()[i]))return false;
            }
            auto opt_count = this->getArgsOptions().size();
            auto function_opt_count = function->getArgsOptions().size();
            if(opt_count != function_opt_count)return false;
            for(int i=0;i<opt_count;i++){
                auto opt = getArgsOptions()[i];
                auto function_opt_index = function->findOptionIndex(opt->getName());
                if(!function_opt_index.has_value())return false;
                auto function_opt = function->getArgsOptions()[function_opt_index.value()];
                if(!opt->equal(function_opt))return false;
            }

            if(getRetSignature() && function->getRetSignature()){
                if(!getRetSignature()->equal(function->getRetSignature()))return false;
            }
            else if(getRetSignature()==nullptr xor function->getRetSignature()==nullptr){
                return false;
            }

            return true;
        }
        else return false;
    }

    void Function::add(Symbol *symbol){
        switch(symbol->getKind()){
            case SymbolKind::Parameter:
                if(symbol->as<Parameter*>()->getName()!="self"){
                    auto param = symbol->as<Parameter*>();
                    if(param->isOptional()){
                        args_options.push_back(param);
                        option_map.insert({param->getName(),args_options.size()-1});
                    }
                    else if(param->isParamArray()){
                        param_array = param;
                    }
                    else{
                        args_signature.push_back(param);
                    }
                    addMemoryLayout(param);
                }
                break;
            case SymbolKind::Variable:
                addMemoryLayout(symbol->as<Variable*>());
                break;
            case SymbolKind::TmpDomain:
                symbol->setName(Format() << "#" << tmp_domain_count);
                tmp_domain_count++;
                break;
        }
        Domain::add(symbol);
    }

    std::vector<Parameter*> &Function::getArgsSignature() {
        return args_signature;
    }

    Function::Function() : Domain(SymbolKind::Function){}

    std::vector<Parameter *> &Function::getArgsOptions() {
        return args_options;
    }

    optional<int> Function::findOptionIndex(const std::string &name) {
        auto target = option_map.find(name);
        if(target == option_map.end())return {};
        return target->second;
    }

    Parameter *Function::getParamArray() {
        return param_array;
    }

    FunctionKind Function::getFunctionKind() {
        return FunctionKind::InterfaceFunction;
    }

    UserFunction::UserFunction(FunctionFlag flag):flag(flag){
        switch (flag) {
            case FunctionFlag::Method:
            case FunctionFlag::Virtual:
            case FunctionFlag::Override:
                self = new Parameter("self",nullptr, true,false);
                UserFunction::add(self);
                break;
            case FunctionFlag::Static:
                break;
        }
    }

    FunctionFlag UserFunction::getFunctionFlag() {
        return flag;
    }

    void UserFunction::setFunctionFlag(FunctionFlag flag) {
        this->flag = flag;
    }

    bool UserFunction::isStatic() {
        if(Symbol::isStatic())return true;
        switch (flag) {
            case FunctionFlag::Virtual:
            case FunctionFlag::Override:
            case FunctionFlag::Method: return false;
            case FunctionFlag::Static: return true;
        }
    }

    void UserFunction::setParent(Domain *parent) {
        Symbol::setParent(parent);
        if(self){
            self->setPrototype(parent);
        }
    }

    FunctionKind UserFunction::getFunctionKind() {
        return FunctionKind::UserFunction;
    }


    ExternalFunction::ExternalFunction(std::string library, std::string alias)
        : library(std::move(library)),alias(std::move(alias)){}

    FunctionKind ExternalFunction::getFunctionKind() {
        return FunctionKind::External;
    }

    std::string ExternalFunction::getLibName() {
        return library;
    }

    std::string ExternalFunction::getAlias() {
        return alias;
    }

    TemporaryDomain::TemporaryDomain(type::Domain *parent,Function *function)
            : Domain(SymbolKind::TmpDomain),parent_function(function){
        setParent(parent);
    }

    void TemporaryDomain::add(type::Symbol *symbol) {
        auto variable = symbol->as<Variable*>();
        NotNull(variable);
        parent_function->addMemoryLayout(variable);
        Domain::add(variable);
    }

    std::string Symbol::getName() {
        return this->name;
    }

    void Symbol::setName(std::string str) {
        strToLowerByRef(str);
        this->name=std::move(str);
    }

    Domain *Symbol::getParent() const {
        return parent;
    }

    void Symbol::setParent(Domain *parent) {
        this->parent = parent;
    }

    AccessFlag Symbol::getAccessFlag() {
        return this->access;
    }

    void Symbol::setAccessFlag(AccessFlag flag) {
        this->access = flag;
    }

    void Symbol::setLocation(Location *location){
        NotNull(location);
        location_ = location;
    }

    Location *Symbol::getLocation(){
        NotNull(location_);
        return location_;
    }

    std::string Symbol::mangling(char separator) {
        if(mangling_name.empty()){
            if(getParent()==nullptr || getParent()->getName() == "global" || getParent()->getKind() == SymbolKind::TmpDomain) {
                return mangling_name = getName();
            }
            else{
                return mangling_name = parent->mangling(separator) + separator + getName();
            }
        }
        else return mangling_name;
    }

    bool Symbol::isStatic() {
        return is_static;
    }

    void Symbol::setStatic(bool value) {
        is_static = value;
    }

    bool Symbol::isExtern() {
        return is_extern;
    }


    std::list<std::string> Symbol::getFullName() {
        list<string> ls;
        if(getParent()) ls = move(getParent()->getFullName());
        ls.push_back(getName());
        return ls;
    }


    Symbol *Domain::lookUp(const string &name) {
        Domain *ptr = static_cast<Domain*>(this);
        Domain *global = nullptr;
        while(ptr){
            auto p = ptr->find(name);
            if(p){
                return p;
            }
            auto tmp = ptr->getParent();
            if(!tmp)global = ptr;
            ptr = tmp;
        }
        if(global && global->name == name)
            return global;
        return nullptr;
    }

    Domain::iterator Domain::begin() {
        return iterator(childs.begin());
    }

    Domain::iterator Domain::end() {
        return iterator(childs.end());
    }

    void Domain::add(Symbol *symbol) {
        NotNull(symbol);
        ASSERT(symbol->getName() == "","symbol name is empty");
        childs.emplace(symbol->getName(),symbol);
        symbol->setParent(this->as<Domain*>());
    }

    Symbol *Domain::find(const string &name) {
        return findInDomainOnly(name);
    }

    Symbol *Domain::findInDomainOnly(const string &name) {
        auto target = childs.find(name);
        if(target == childs.end())return nullptr;
        return target->second;
    }

    void Domain::addMemoryLayout(Variable *variable) {
        if(variable->isParameter()){
            variable->setLayoutIndex(param_count++);
        }
        else{
            variable->setLayoutIndex(local_count++);
        }
        memory_layout.push_back(variable);
    }

    void Domain::updateMemoryLayout() {
        data::u32 size = 0;
        for(auto &variable:memory_layout){
            variable->setOffset(size);
            size += variable->getRealByteLength();
        }
        Prototype::setByteLength(size);
    }

    const std::vector<Variable *> &Domain::getMemoryLayout() {
        return memory_layout;
    }

    void Class::setExtend(Class *base) {
        NotNull(base);
        this->base_class = base;
    }

    void Class::setConstructor(Constructor *constructor) {
        NotNull(constructor);
        this->constructor = constructor;
        constructor->setParent(this);
    }

    void Class::addImpl(Interface *interface) {
        this->impl_interface.insert({interface->mangling(),interface});
    }

    Interface *Class::getImpl(std::string mangling_name) {
        auto target = impl_interface.find(mangling_name);
        if(target == impl_interface.end())return nullptr;
        return target->second;
    }

    Class::Class() : Record(SymbolKind::Class){}

    void Class::add(Symbol *symbol) {
        if(symbol->getKind() == SymbolKind::Variable)
            Record::add(symbol);
        else if(auto function = symbol->as<Function*>()){
            if(function->getFunctionKind() == FunctionKind::Constructor){
                this->constructor = function->as<Constructor*>();
            }
            else Domain::add(symbol);
        }
        else Domain::add(symbol);
    }

    Class *Class::getExtend() {
        return base_class;
    }

    Symbol *Class::find(const string &name) {
        auto target = Domain::findInDomainOnly(name);
        if(target)return target;
        if(getExtend()!=nullptr)
            return getExtend()->find(name);
        return nullptr;
    }

    void Class::updateMemoryLayout() {
        data::u32 size = 0;
        if(getExtend()) size = getExtend()->getByteLength();
        for(auto &variable:memory_layout){
            variable->setOffset(size);
            size += variable->getRealByteLength();
        }
        Prototype::setByteLength(size);
    }

    const std::map<std::string, Interface *> &Class::getImplMap() {
        return impl_interface;
    }

    void Class::generateClassInfo() {
        // generate vtable
        if(getExtend()){
            vtable = new VirtualTable(getExtend()->vtable);
            this->impl_vtables.insert(getExtend()->impl_vtables.begin(),getExtend()->impl_vtables.end());
        }
        else{
            vtable = new VirtualTable();
        }

        for(auto [name,interface]:impl_interface){
            impl_vtables.insert({name,new VirtualTable(interface->getVTable())});
        }

        auto try_fill_slot = [](UserFunction *function,VirtualTable *table)->bool{
            auto slot = table->findSlot(function->getName());
            if(slot.has_value()){
                table->fill(slot.value(),function);
                return true;
            }
            else{
                return false;
            }
        };

        for(auto member : *this){
            auto function = member->as<Function*>();
            if(!function)continue;
            switch(function->getFunctionKind()){
                case FunctionKind::InterfaceFunction:{
                    vtable->addSlot(function);
                    break;
                }
                case FunctionKind::UserFunction:{
                    if(auto user_function = function->as<UserFunction*>()){
                        switch (user_function->getFunctionFlag()) {
                            case FunctionFlag::Virtual:{
                                vtable->addSlot(function);
                                auto slot = vtable->findSlot(function->getName()).value();
                                vtable->fill(slot,user_function);
                                break;
                            }
                            case FunctionFlag::Override:{
                                auto succeed = false;
                                if(try_fill_slot(user_function,vtable)){
                                    succeed = true;
                                }
                                else {
                                    for(auto [interface_name,interface_vtable]:impl_vtables){
                                        if(try_fill_slot(user_function,interface_vtable)){
                                            succeed = true;
                                            break;
                                        }
                                    }
                                }
                                if(!succeed){
                                    Logger::error(user_function->getLocation(),"method does not override a method from base class or interface");
                                }
                                break;
                            }
                        }
                    }
                    break;
                }
                case FunctionKind::Operator:
                    // todo: operator overload support
                    break;
                case FunctionKind::Constructor:
                    break;
            }
        }

        is_abstract_class = false;
        if(!vtable->hasEmptySlot())is_abstract_class = true;
        for(auto [_,table]:impl_vtables){
            if(!table->hasEmptySlot())is_abstract_class = true;
        }
    }

    bool Class::isAbstract() {
        return is_abstract_class;
    }

    Constructor *Class::getConstructor() {
        return constructor;
    }



    void Record::add(Symbol *symbol) {
        auto field = symbol->as<Variable*>();
        NotNull(field);
        field->offset = getByteLength();
        addMemoryLayout(field);
        Domain::add(symbol);
        fields.push_back(field);
    }


    namespace primitive{

        VariantClass::VariantClass() {
            setName("variant");
            setAccessFlag(AccessFlag::Public);
        }

        Primitive::Primitive(std::string name, vm::Data data_kind)
          : Class(SymbolKind::Primitive),kind_(data_kind){
            setName(std::move(name));
            setAccessFlag(AccessFlag::Public);
            setByteLength(data_kind.getSize());
        }

        bool Primitive::equal(Prototype *ptr) {
            auto p = ptr->as<Primitive*>();
            return p && p->kind_.operator==(kind_);
        }

        vm::Data Primitive::getDataKind() {
            return this->kind_;
        }

    }



    bool Record::equal(Prototype *ptr) {
        auto record = ptr->as<Record*>();
        if(!record)return false;
        if(this->getByteLength() != record->getByteLength())return false;
        for(int i=0;i<fields.size();i++){
            auto l = this->fields[i]->getPrototype();
            auto r = record->fields[i]->getPrototype();
            if(!l->equal(r)){
                return false;
            }
        }
        return true;
    }

    const std::vector<Variable*> &Record::getFields() {
        return fields;
    }

    std::size_t Variable::getOffset() {
        //if(offset == -1)parent.lock()->as
        return offset;
    }

    void Variable::setOffset(std::size_t value) {
        this->offset = value;
    }

    Variable::Variable() : Symbol(SymbolKind::Variable){}

    Variable::Variable(SymbolKind kind) : Symbol(kind) {}

    bool Variable::isConstant() {
        return is_const;
    }

    void Variable::setConstant(bool value) {
        is_const = value;
    }

    Prototype *Variable::getPrototype() {
        return prototype;
    }

    void Variable::setPrototype(Prototype *ptr) {
        NotNull(ptr);
        this->prototype = ptr;
    }

    data::ptr Variable::getRealByteLength() {
        switch(prototype->getKind()){
            case SymbolKind::Primitive:
            case SymbolKind::Record:
            case SymbolKind::Array:
                return prototype->getByteLength();
            case SymbolKind::Class:
            case SymbolKind::Function:
                return vm::Data::ptr.getSize();
            case SymbolKind::Enum:
                return vm::Data(vm::Data::i32).getSize();
        }
        PANIC;
    }

    void Variable::setLayoutIndex(std::size_t index) {
        layout_index = index;
    }

    std::size_t Variable::getLayoutIndex(){
        return layout_index;
    }

    VariableKind Variable::getVariableKind() {
        switch(getParent()->getKind()){
            case SymbolKind::Class:
                if(isStatic())return VariableKind::StaticField;
                else          return VariableKind::Field;
            case SymbolKind::Record:
                return VariableKind::Field;
            case SymbolKind::Module:
                return VariableKind::StaticField;
            case SymbolKind::TmpDomain: // in function/sub/constructor
                if(isParameter()) return VariableKind::Parameter;
                else              return VariableKind::Local;
        }
        PANIC;
    }


    bool Variable::isParameter() {
        return is_parameter;
    }

    void Variable::toggleParameter() {
        is_parameter = !is_parameter;
    }


    Error::Error(): Prototype(SymbolKind::Error){}

    void Interface::add(Symbol *symbol) {
        ASSERT(symbol->getKind() != SymbolKind::Function,"symbol is not a function");
        vtable->addSlot(symbol->as<Function*>());
        Domain::add(symbol);
    }

    Interface::Interface() : Domain(SymbolKind::Interface){
        vtable = new VirtualTable();
    }

    VirtualTable *Interface::getVTable() {
        return vtable;
    }


    Array::Array(Prototype *element,data::u32 size)
        : Class(SymbolKind::Array),element_type(element),size_(size){
        setName(Format() << element->getName() << "[" << size << "]");
    }

    Prototype *Array::getElementPrototype() {
        return element_type;
    }


    bool Array::equal(Prototype *ptr) {
        auto array = ptr->as<Array*>();
        if(!array)return false;
        return array->getElementPrototype()->equal(element_type) && array->size_ == size_;
    }

    data::ptr Array::getByteLength() {
        return size_ * getElementPrototype()->getByteLength();
    }


    data::ptr Prototype::getByteLength() {
        return byte_length;
    }

    void Prototype::setByteLength(data::ptr value) {
        byte_length = value;
    }
    
    vector<string> Operator::KindString = {"Get","Compare","Times","Div","Plus","Minus","UnaryPlus","UnaryMinus","Invoke"};

    std::string Operator::getName() {
        auto str = Format();
        str<<KindString[(int)kind];
        for(auto parameter : getArgsSignature()){
            str<< '-' << parameter->getName();
        }
        return (string)str;
    }

    void Operator::setName(std::string) {
        PANIC;
    }

    void Operator::setOperatorKind(Kind kind) {
        this->kind = kind;
    }

    Operator::Kind Operator::getOperatorKind() {
        return kind;
    }

    FunctionKind Operator::getFunctionKind() {
        return FunctionKind::Operator;
    }

    FunctionKind Constructor::getFunctionKind() {
        return FunctionKind::Constructor;
    }

    Constructor::Constructor() {
        setAccessFlag(AccessFlag::Public);
    }


    VirtualTable::VirtualTable(VirtualTable *base)
        : slot(base->slot),slot_map(base->slot_map),base(base) {}

    void VirtualTable::addSlot(Function *function) {
        auto conflict = slot_map.find(function->getName());
        if(conflict==slot_map.end()){
            slot.emplace_back(function,nullptr);
            slot_map.insert({function->getName(),slot.size()-1});
        }
        else{
            auto tmp = slot[conflict->second].first;
            Logger::error({tmp->getLocation(),function->getLocation()}, Format() << "naming conflict with '"
                                                                                 << tmp->mangling('.') << "' and '" << function->mangling('.') << "'");
        }
    }

    std::optional<int> VirtualTable::findSlot(const string& name) {
        auto target = slot_map.find(name);
        if(target == slot_map.end())return {};
        return target->second;
    }

    void VirtualTable::fill(int slot, UserFunction *function) {
        auto target = this->slot[slot].first;
        if(!target->equal(function)){
            Logger::error({function->getLocation(),target->getLocation()},
                          Format() << "override method '"
                                <<target->mangling('.')<<"' and '"
                                <<function->mangling('.')
                                <<"' have different parameters or return type");
        }
        else this->slot[slot].second = function;
    }

    bool VirtualTable::hasEmptySlot() {
        for(auto [_,content] : slot){
            if(!content)return false;
        }
        return true;
    }



    DebugInfo * makeDebugInfo(std::string name,std::string type,
                              list<string> property = {},list<DebugInfo*> childs = {}){
        auto ret = new DebugInfo;
        Format fmt;
        fmt << name << " : " << type;
        for(auto p:property)fmt << " " << p;
        ret->text = fmt;
        ret->childs = std::move(childs);
        return ret;
    }

    DebugInfo * makeDebugInfo(std::string type,Domain *domain){
        list<string> p;
        list<DebugInfo*> info;
        if(domain->isStatic())p.emplace_back("Static");
        p.emplace_back(AccessFlagString[(int)domain->getAccessFlag()]);
        for(auto child : *domain){
            info.push_back(child->debug());
        }
        auto name = domain->getName();
        if(name.empty())name = "<Unnamed>";
        return makeDebugInfo(name,std::move(type),p,info);
    }

    DebugInfo *Error::debug() {
        return makeDebugInfo("Error","");
    }

    DebugInfo *Variable::debug() {
        list<string> p;
        if(isStatic())p.emplace_back("Static");
        p.emplace_back(AccessFlagString[(int)getAccessFlag()]);
        return makeDebugInfo(getName(),getPrototype()->mangling('.'),p);
    }

    DebugInfo *Module::debug() {
        return makeDebugInfo("Module",this);
    }

    DebugInfo *Record::debug() {
        return makeDebugInfo("Record",this);
    }

    DebugInfo *Function::debug() {
        return makeDebugInfo("Function",this);
    }

    DebugInfo *Constructor::debug() {
        return makeDebugInfo("Ctor",this);
    }

    DebugInfo *UserFunction::debug() {
        return makeDebugInfo("UserFunction",this);
    }

    DebugInfo *ExternalFunction::debug() {
        return makeDebugInfo("External",this);
    }

    DebugInfo *Class::debug() {
        auto ret = makeDebugInfo("Class",this);
        if(constructor) ret->childs.push_back(constructor->debug());
        return ret;
    }

    DebugInfo *Interface::debug() {
        return makeDebugInfo("Interface",this);
    }

    DebugInfo *primitive::Primitive::debug() {
        return makeDebugInfo("Primitive",this);
    }

    DebugInfo *Enumeration::debug() {
        return makeDebugInfo("Enum",this);
    }

    DebugInfo *EnumMember::debug() {
        return new DebugInfo{getName() + " = " + to_string(index)};
    }

    DebugInfo *Parameter::debug() {
        list<string> p;
        if(isStatic())p.emplace_back("Static");
        p.emplace_back(isByval() ? "Byval" : "Byref");
        p.emplace_back(AccessFlagString[(int)getAccessFlag()]);
        return makeDebugInfo(getName(),getPrototype()->mangling('.'),p);
    }

    DebugInfo *Array::debug() {
        return new DebugInfo{getName()};
    }

    std::list<std::string> Array::getFullName() {
        auto ret = element_type->getFullName();
        ret.push_front(to_string(size_));
        ret.push_front("array");
        return ret;
    }

    std::string Array::getName() {
        return Format()<<element_type->getName()<<'['<<to_string(size_)<<']';
    }

    DebugInfo *TemporaryDomain::debug() {
        return makeDebugInfo("",this);
    }

    void debugSymbolTable(DebugInfo *info,std::ostream &stream,string indent){
        stream << indent << info->text;
        if(!info->childs.empty()){
            stream << " {\n";
            for(auto i : info->childs){
                debugSymbolTable(i,stream,indent + "    ");
            }
            stream << indent << "}";
        }
        stream << '\n';
    }

    std::string debugSymbolTable(DebugInfo *info){
        std::stringstream stream;
        stream << "# SymbolTable \n";
        debugSymbolTable(info,stream,"");
        return stream.str();
    }

}
