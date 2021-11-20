//
// Created by yorkin on 7/17/21.
//

#include "type.h"
#include "nullSafe.h"
#include "semantic.h"
#include <utility>
#include <vector>
#include <map>
#include <string>
using namespace std;
namespace evoBasic::type{

    void strToLowerByRef(string& str){
        transform(str.begin(),str.end(),str.begin(),[](unsigned char c){ return std::tolower(c); });
    }


    void Module::addImport(shared_ptr<Symbol> child) {
        if(auto mod = dynamic_pointer_cast<Module>(child)){
            importedModule.push_back(mod);
        }
        else{
            throw "unimpl";
        }
    }


    bool Class::equal(shared_ptr<Prototype> ptr) {
        return ptr.get()==this;
    }



    bool Enumeration::equal(shared_ptr<Prototype> ptr) {
        return ptr.get()==this;
    }


    Enumeration::Enumeration() : Class(DeclarationEnum::Enum_) {
        setByteLength(4);
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

    void Enumeration::add(std::shared_ptr<Symbol> symbol) {
        ASSERT(symbol->getKind() != DeclarationEnum::EnumMember,"symbol is not a EnumMember");
        Domain::add(symbol);
    }

    Argument::Argument(std::string name,std::shared_ptr<Prototype> prototype,bool isByval,bool isOptional)
            : Variable(DeclarationEnum::Argument), is_byval(isByval), is_optional(isOptional){
        setName(move(name));
        setPrototype(std::move(prototype));
    }

    bool Argument::isByval() {
        return is_byval;
    }

    bool Argument::isOptional() {
        return is_optional;
    }

    std::string Argument::debug(int indent) {
        stringstream str;
        for(int i=0;i<indent;i++)str<<indent_unit;
        str << (isByval() ? "ByVal":"ByRef") << ' ';
        str << getName() << " As " << getPrototype()->getName();
        return str.str();
    }

    data::ptr Argument::getRealByteLength() {
        if(isByval()){
            return Variable::getRealByteLength();
        }
        else{
            return vm::Data::ptr.getSize();
        }
    }

    shared_ptr<Prototype> Function::getRetSignature() {
        return this->retSignature;
    }

    void Function::setRetSignature(std::shared_ptr<Prototype> ptr){
        retSignature = std::move(ptr);
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

    bool Function::equal(shared_ptr<Prototype> ptr) {
        return false;
    }

    void Function::add(shared_ptr<Symbol> symbol){
        switch(symbol->getKind()){
            case DeclarationEnum::Argument:
                argsSignature.push_back(symbol->as_shared<Argument>());
                addMemoryLayout(symbol->as_shared<Variable>());
                break;
            case DeclarationEnum::Variable:
                addMemoryLayout(symbol->as_shared<Variable>());
                break;
            case DeclarationEnum::TmpDomain:
                symbol->setName(format()<<"#"<<tmp_domain_count);
                tmp_domain_count++;
                break;
        }
        Domain::add(symbol);
    }

    std::vector<std::shared_ptr<Argument>> &Function::getArgsSignature() {
        return argsSignature;
    }

    Function::Function() : Domain(DeclarationEnum::Function){}


    UserFunction::UserFunction(MethodFlag flag,ast::Function *function_node)
        :function_node(function_node),flag(flag){}

    ast::Function* UserFunction::getFunctionNode() {
        return this->function_node;
    }

    ExternalFunction::ExternalFunction(std::string library, std::string name)
        : library(std::move(library)),name(std::move(name)){}



    TemporaryDomain::TemporaryDomain(std::weak_ptr<type::Domain> parent,shared_ptr<UserFunction> function)
            : Domain(type::DeclarationEnum::TmpDomain),parent_function(function){
        setParent(std::move(parent));
    }

    void TemporaryDomain::add(std::shared_ptr<type::Symbol> symbol) {
        auto variable = symbol->as_shared<Variable>();
        NotNull(variable.get());
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

    std::weak_ptr<Domain> Symbol::getParent() {
        return parent;
    }

    void Symbol::setParent(std::weak_ptr<Domain> parent) {
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

    std::string Symbol::mangling() {
        if(mangling_name.empty()){
            if(parent.expired() || parent.lock()->getName() == "global") {
                return mangling_name = getName();
            }
            else{
                return mangling_name = parent.lock()->mangling() + '$' + getName();
            }
        }
        else return mangling_name;
    }


    std::shared_ptr<Symbol> Domain::lookUp(const string &name) {
        std::shared_ptr<Domain> ptr = static_pointer_cast<Domain>(shared_from_this());
        shared_ptr<Domain> global = nullptr;
        while(ptr){
            auto p = ptr->find(name);
            if(p){
                return p;
            }
            auto tmp = ptr->getParent().lock();
            if(!tmp)global = ptr;
            ptr = tmp;
        }
        if(global && global->name == name)
            return global;
        return {nullptr};
    }

    Domain::iterator Domain::begin() {
        return iterator(childs.begin());
    }

    Domain::iterator Domain::end() {
        return iterator(childs.end());
    }

    void Domain::add(std::shared_ptr<Symbol> symbol) {
        NotNull(symbol.get());
        ASSERT(symbol->getName() == "","symbol name is empty");
        childs.emplace(symbol->getName(),symbol);
        symbol->setParent(shared_from_this()->as_shared<Domain>());
    }

    std::shared_ptr<Symbol> Domain::find(const string &name) {
        auto target = childs.find(name);
        if(target == childs.end())return {nullptr};
        return target->second;
    }

    void Domain::addMemoryLayout(std::shared_ptr<Variable> variable) {
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

    void Class::setExtend(std::shared_ptr<Class> base) {
        this->base_class = std::move(base);
    }

    void Class::setConstructor(std::shared_ptr<Function> constructor) {
        this->constructor = std::move(constructor);
    }

    void Class::addImplementation(std::shared_ptr<Interface> interface) {
        this->impl_interface.push_back(interface);
    }

    Class::Class() : Record(DeclarationEnum::Class){
        setByteLength(4);
    }

    void Class::add(std::shared_ptr<Symbol> symbol) {
        if(symbol->getKind() == DeclarationEnum::Variable)
            Record::add(symbol);
        else
            Domain::add(symbol);
    }


    void Record::add(std::shared_ptr<Symbol> symbol) {
        auto field = symbol->as_shared<Variable>();
        NotNull(field.get());
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
          : Class(DeclarationEnum::Primitive),kind_(data_kind){
            setName(std::move(name));
            setAccessFlag(AccessFlag::Public);
            setByteLength(data_kind.getSize());
        }

        bool Primitive::equal(std::shared_ptr<Prototype> ptr) {
            auto p = ptr->as_shared<Primitive>();
            return p && p->kind_ == kind_;
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



    bool Record::equal(std::shared_ptr<Prototype> ptr) {
        auto record = ptr->as_shared<Record>();
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

    const std::vector<std::shared_ptr<Variable>> &Record::getFields() {
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

    Variable::Variable() : Symbol(DeclarationEnum::Variable){}

    Variable::Variable(DeclarationEnum kind) : Symbol(kind) {}

    bool Variable::isConstant() {
        return is_const;
    }

    void Variable::setConstant(bool value) {
        is_const = value;
    }

    std::shared_ptr<Prototype> Variable::getPrototype() {
        return prototype;
    }

    void Variable::setPrototype(std::shared_ptr<Prototype> ptr) {
        NotNull(ptr.get());
        this->prototype = ptr;
    }

    bool Variable::isGlobal() {
        return is_global;
    }

    void Variable::setGlobal() {
        is_global = true;
    }

    data::ptr Variable::getRealByteLength() {
        switch(prototype->getKind()){
            case DeclarationEnum::Primitive:
            case DeclarationEnum::Type:
            case DeclarationEnum::Array:
                return prototype->getByteLength();
            case DeclarationEnum::Class:
            case DeclarationEnum::Function:
                return vm::Data(vm::Data::u32).getSize();
        }
        return 0;
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

    Error::Error(): Prototype(DeclarationEnum::Error){}

    void Interface::add(std::shared_ptr<Symbol> symbol) {
        ASSERT(symbol->getKind() != DeclarationEnum::Function,"symbol is not a function");
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


    Array::Array(std::shared_ptr<Prototype> element,data::u32 size)
        : Class(DeclarationEnum::Array),element_type(element),size_(size){
        setName(format()<<element->getName()<<"["<<size<<"]");
    }

    std::shared_ptr<Prototype> Array::getElementPrototype() {
        return element_type;
    }


    bool Array::equal(std::shared_ptr<Prototype> ptr) {
        auto array = ptr->as_shared<Array>();
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
