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

using namespace std;
namespace evoBasic::type{

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
        str<<getName()<<" : Enum{\n";
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

    Parameter::Parameter(std::string name,Prototype *prototype, bool isByval, bool isOptional)
            : Variable(SymbolKind::Argument), is_byval(isByval), is_optional(isOptional){
        setName(move(name));
        setPrototype(prototype);
    }

    bool Parameter::isByval() {
        return is_byval;
    }

    bool Parameter::isOptional() {
        return is_optional;
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

    Prototype *Function::getRetSignature() {
        return this->retSignature;
    }

    void Function::setRetSignature(Prototype *ptr){
        NotNull(ptr);
        retSignature = ptr;
    }


    std::string Function::debug(int indent) {
        stringstream str;
        for(int i=0;i<indent;i++)str<<indent_unit;
        str<<this->getName()<<" : Function(";
        for(const auto& arg:this->argsSignature){
            str<<arg->debug(0);
            if(&arg!=&argsSignature.back())str<<',';
        }
        str<<')';
        if(retSignature)
            str<<" As "<<retSignature->getName();
        str<<"{\n";
        for(auto p : *this){
            p->debug(indent+1);
        }
        for(int i=0;i<indent;i++)str<<indent_unit;
        str<<"}\n";
        return str.str();
    }

    bool Function::equal(Prototype *ptr) {
        return false;
    }

    void Function::add(Symbol *symbol){
        switch(symbol->getKind()){
            case SymbolKind::Argument:
                argsSignature.push_back(symbol->as<Parameter*>());
                addMemoryLayout(symbol->as<Variable*>());
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
        return argsSignature;
    }

    Function::Function() : Domain(SymbolKind::Function){}


    UserFunction::UserFunction(Function::Flag flag,ast::Function *function_node)
        :function_node(function_node),flag(flag){}

    ast::Function* UserFunction::getFunctionNode() {
        return this->function_node;
    }

    bool UserFunction::isStatic() {
        return is_static;
    }

    void UserFunction::setStatic(bool value) {
        is_static = value;
    }

    ExternalFunction::ExternalFunction(std::string library, std::string name)
        : library(std::move(library)),name(std::move(name)){}



    TemporaryDomain::TemporaryDomain(type::Domain *parent,UserFunction *function)
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
        str<< getName() <<" : Domain{\n";
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

    Domain *Symbol::getParent() {
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
        str<<this->getName()<<" : Module{\n";
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
        str<<getName()<<" : Class{\n";
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

    void Class::setConstructor(Function *constructor) {
        NotNull(constructor);
        this->constructor = constructor;
    }

    void Class::addImplementation(Interface *interface) {
        this->impl_interface.push_back(interface);
    }

    Class::Class() : Record(SymbolKind::Class){}

    void Class::add(Symbol *symbol) {
        if(symbol->getKind() == SymbolKind::Variable)
            Record::add(symbol);
        else
            Domain::add(symbol);
    }

    Class *Class::getExtend() {
        return base_class;
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
            str<<getName()<<" : Primitive("<<kind_.toString()<<")\n";
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
        str<<this->getName()<<" : Record{\n";
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
        str << this->getName() << " : Variable(";
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

    bool Variable::isStatic() {
        return is_static;
    }

    void Variable::setStatic(bool value) {
        is_static = value;
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
        Domain::add(symbol);
    }

    std::string Interface::debug(int indent) {
        stringstream str;
        for(int i=0;i<indent;i++)str<<indent_unit;
        str<<getName()<<" : Interface{\n";
        for(auto p = begin();p!=end();p++){
            str<<p->debug(indent+1);
        }
        for(int i=0;i<indent;i++)str<<indent_unit;
        str<<"}\n";
        return str.str();
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
}
