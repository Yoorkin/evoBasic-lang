//
// Created by yorkin on 7/17/21.
//

#include "type.h"
#include "nullSafe.h"
#include "semantic/semantic.h"
#include <utility>
#include <vector>
#include <map>
#include <string>
#include "logger.h"

using namespace std;
namespace evoBasic::type{

    vector AccessFlagString = {"Public","Private","Friend","Protected"};
    vector FunctionFlagString = {"Method","Static","Virtual","Override"};

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

    std::string Enumeration::debug(int indent) {
        stringstream str;
        for(int i=0;i<indent;i++)str<<indent_unit;
        str<<getName()<<" : "<<AccessFlagString[(int)getAccessFlag()]<<" Enum{\n";
        for(auto p = begin();p!=end();p++){
            str<<p->debug(indent+1);
        }
        for(int i=0;i<indent;i++)str<<indent_unit;
        str<<"}\n";
        return str.str();
    }

    void Enumeration::add(Symbol *symbol) {
        ASSERT(symbol->getKind() != SymbolKind::EnumMember,"symbol is not a EnumMember");
        Domain::add(symbol);
    }

    Parameter::Parameter(std::string name,Prototype *prototype, bool isByval, bool isOptional ,bool isParamArray)
            : Variable(SymbolKind::Argument), is_byval(isByval), is_optional(isOptional),is_param_array(isParamArray){
        setName(move(name));
        if(isParamArray)is_byval = true;
        if(prototype)setPrototype(prototype);
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

    std::string Parameter::debug(int indent) {
        stringstream str;
        for(int i=0;i<indent;i++)str<<indent_unit;
        str << (isByval() ? "ByVal":"ByRef") << ' ';
        str << getName() << " As " << getPrototype()->getName();
        return str.str();
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

    Prototype *Function::getRetSignature() {
        return this->ret_signature;
    }

    void Function::setRetSignature(Prototype *ptr){
        NotNull(ptr);
        ret_signature = ptr;
    }


    std::string Function::debug(int indent) {
        stringstream str;
        for(int i=0;i<indent;i++)str<<indent_unit;
        str<<this->getName()<<" : "<<AccessFlagString[(int)getAccessFlag()]<<" Function(";
        for(const auto& arg:this->args_signature){
            str<<arg->debug(0);
            if(&arg!=&args_signature.back())str << ',';
        }
        str<<')';
        if(ret_signature)
            str << " As " << ret_signature->getName();
        str<<"{\n";
        for(auto p : *this){
            p->debug(indent+1);
        }
        for(int i=0;i<indent;i++)str<<indent_unit;
        str<<"}\n";
        return str.str();
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
            return true;
        }
        else return false;
    }

    void Function::add(Symbol *symbol){
        switch(symbol->getKind()){
            case SymbolKind::Argument:
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
                symbol->setName(format()<<"#"<<tmp_domain_count);
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
        return FunctionKind::Function;
    }

    UserFunction::UserFunction(FunctionFlag flag,ast::Function *function_node)
        :function_node(function_node),flag(flag){
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

    ast::Function* UserFunction::getFunctionNode() {
        return this->function_node;
    }


    std::string UserFunction::debug(int indent) {
        stringstream str;
        for(int i=0;i<indent;i++)str<<indent_unit;
        str<<this->getName()<<" : "<<AccessFlagString[(int)getAccessFlag()];
        str<<" "<<FunctionFlagString[(int)getFunctionFlag()];
        str<<" Function(";
        for(const auto& arg:this->getArgsSignature()){
            str<<arg->debug(0);
            if(&arg!=&getArgsSignature().back())str<<',';
        }
        str<<')';
        if(getRetSignature())
            str<<" As "<<getRetSignature()->getName();
        str<<"{\n";
        for(auto p : *this){
            p->debug(indent+1);
        }
        for(int i=0;i<indent;i++)str<<indent_unit;
        str<<"}\n";
        return str.str();
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

    ExternalFunction::ExternalFunction(std::string library, std::string name)
        : library(std::move(library)),name(std::move(name)){}

    FunctionKind ExternalFunction::getFunctionKind() {
        return FunctionKind::External;
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

    std::string TemporaryDomain::debug(int indent) {
        stringstream str;
        for(int i=0;i<indent;i++)str<<indent_unit;
        str<< getName() <<" : "<<AccessFlagString[(int)getAccessFlag()]<<" Domain{\n";
        for(auto p = begin();p!=end();p++){
            str<<p->debug(indent+1);
        }
        for(int i=0;i<indent;i++)str<<indent_unit;
        str<<"}\n";
        return str.str();
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
            if(parent==nullptr || parent->getName() == "global") {
                return mangling_name = getName();
            }
            else{
                return mangling_name = parent->mangling() + separator + getName();
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


    std::string Module::debug(int indent) {
        stringstream str;
        for(int i=0;i<indent;i++)str<<indent_unit;
        str<<this->getName()<<" : "<<AccessFlagString[(int)getAccessFlag()]<<" Module{\n";
        for(auto p = begin();p!=end();p++){
            str<<p->debug(indent+1);
        }
        for(int i=0;i<indent;i++)str<<indent_unit;
        str<<"}\n";
        return str.str();
    }


    std::string Class::debug(int indent) {
        stringstream str;
        for(int i=0;i<indent;i++)str<<indent_unit;
        str<<getName()<<" : "<<AccessFlagString[(int)getAccessFlag()]<<" Class{\n";
        for(auto p = begin();p!=end();p++){
            str<<p->debug(indent+1);
        }
        for(int i=0;i<indent;i++)str<<indent_unit;
        str<<"}\n";
        return str.str();
    }

    void Class::setExtend(Class *base) {
        NotNull(base);
        this->base_class = base;
    }

    void Class::setConstructor(Constructor *constructor) {
        NotNull(constructor);
        this->constructor = constructor;
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
                case FunctionKind::Function:{
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

        std::string Primitive::debug(int indent) {
            std::stringstream str;
            for(int i=0;i<indent;i++)str<<indent_unit;
            str<<getName()<<" : "<<AccessFlagString[(int)getAccessFlag()]<<" Primitive("<<kind_.toString()<<")\n";
            return str.str();
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

    std::string Record::debug(int indent) {
        stringstream str;
        for(int i=0;i<indent;i++)str<<indent_unit;
        str<<this->getName()<<" : "<<AccessFlagString[(int)getAccessFlag()]<<" Record{\n";
        for(auto p = begin();p!=end();p++){
            str<<p->debug(indent+1);
        }
        for(int i=0;i<indent;i++)str<<indent_unit;
        str<<"}\n";
        return str.str();
    }

    const std::vector<Variable*> &Record::getFields() {
        return fields;
    }


    std::string Variable::debug(int indent) {
        stringstream str;
        for(int i=0;i<indent;i++)str << indent_unit;
        str << this->getName() << " : "<<AccessFlagString[(int)getAccessFlag()]
            <<(isStatic()?" Static":"")
            <<" Variable(";
        if(getPrototype()) str << getPrototype()->getName();
        else str << " ? ";
        str << ")\n";
        return str.str();
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
        }
        PANIC;
    }


    std::string EnumMember::debug(int indent) {
        stringstream str;
        for(int i=0;i<indent;i++)str<<indent_unit;
        str<<this->getName()<<" : "<<this->index<<'\n';
        return str.str();
    }

    std::string Error::debug(int indent) {
        return getName();
    }

    Error::Error(): Prototype(SymbolKind::Error){}

    void Interface::add(Symbol *symbol) {
        ASSERT(symbol->getKind() != SymbolKind::Function,"symbol is not a function");
        vtable->addSlot(symbol->as<Function*>());
        Domain::add(symbol);
    }

    std::string Interface::debug(int indent) {
        stringstream str;
        for(int i=0;i<indent;i++)str<<indent_unit;
        str<<getName()<<" : "<<AccessFlagString[(int)getAccessFlag()]<<" Interface{\n";
        for(auto p = begin();p!=end();p++){
            str<<p->debug(indent+1);
        }
        for(int i=0;i<indent;i++)str<<indent_unit;
        str<<"}\n";
        return str.str();
    }

    Interface::Interface() : Domain(SymbolKind::Interface){
        vtable = new VirtualTable();
    }

    VirtualTable *Interface::getVTable() {
        return vtable;
    }

    Array::Array(Prototype *element,data::u32 size)
        : Class(SymbolKind::Array),element_type(element),size_(size){
        setName(format()<<element->getName()<<"["<<size<<"]");
    }

    Prototype *Array::getElementPrototype() {
        return element_type;
    }


    bool Array::equal(Prototype *ptr) {
        auto array = ptr->as<Array*>();
        if(!array)return false;
        return array->getElementPrototype()->equal(element_type) && array->size_ == size_;
    }

    std::string Array::debug(int indent) {
        return format()<<"Ptr<"<<element_type->getName()<<">";
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
        auto str = format();
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

    Constructor::Constructor(ast::Constructor *node) : constructor_node(node) {}

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
            Logger::error({tmp->getLocation(),function->getLocation()},format()<<"naming conflict with '"
                                        <<tmp->mangling('.')<<"' and '"<<function->mangling('.')<<"'");
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
                                format()<<"override method '"
                                <<target->mangling('.')<<"' and '"
                                <<function->mangling('.')
                                <<"' have different parameters");
        }
        else this->slot[slot].second = function;
    }

    bool VirtualTable::hasEmptySlot() {
        for(auto [_,content] : slot){
            if(!content)return false;
        }
        return true;
    }
}
