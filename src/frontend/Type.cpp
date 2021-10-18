//
// Created by yorkin on 7/17/21.
//

#include"Type.h"
/*
 * 吹捧hack cpp宏的都斯德哥尔摩综合症
 * 拿勺子挖隧道 拿牙刷洗车自我感动
 * 8/10/2021
    namespace evoBasic::Type {
        STRING_ENUM_DEFINE(InstanceEnum, INSTANCE_ENUM_LIST);
        STRING_ENUM_DEFINE(DeclarationEnum, DECLARATION_ENUM_LIST);
        STRING_ENUM_DEFINE(FunctionEnum, FUNCTION_ENUM_LIST);
    }
 */
#include"Exception.h"
#include<utility>
#include<vector>
#include<map>
#include<string>

namespace evoBasic::Type{

    Member Member::Empty{};

    bool Member::operator==(const Member &rhs) const {
        return symbol==rhs.symbol;
    }

    Member Member::FromSymbol(AccessFlag flag, std::shared_ptr<Symbol> ptr) {
        Member ret;
        ret.symbol = ptr;
        ret.access = flag;
        ret.kind = SymbolMember;
        ret.name = ptr->getName();
        return ret;
    }

    Member Member::FromFunction(AccessFlag flag, std::shared_ptr<Function> function) {
        Member ret;
        ret.symbol = function;
        ret.access = flag;
        ret.kind = FunctionMember;
        ret.method = function->getMethodFlag();
        ret.name = function->getName();
        return ret;
    }

    Member Member::FromField(AccessFlag flag, std::string field_name, std::shared_ptr<Prototype> field_prototype) {
        Member ret;
        ret.symbol = field_prototype;
        ret.kind = FieldMember;
        ret.name = std::move(field_name);
        ret.access = flag;
        ret.method = MethodFlag::Normal;
        return ret;
    }


    void strToLowerByRef(string& str){
        transform(str.begin(),str.end(),str.begin(),[](unsigned char c){ return std::tolower(c); });
    }


    void Module::add(Member member) {
        this->members.insert(make_pair(member.symbol->getName(),member));
        member.symbol->setParent(static_pointer_cast<Module>(shared_from_this()));
    }

    void Module::addImport(shared_ptr<Symbol> child) {
        if(auto mod = dynamic_pointer_cast<Module>(child)){
            importedModule.push_back(mod);
        }
        else{
            throw "unimpl";
        }
    }

    void Class::add(Member member) {
        switch(member.kind){
            case Member::FunctionMember:
                if(member.method==MethodFlag::Virtual||member.method==MethodFlag::Override){
                    virtual_table.insert({member.name,member});
                }
                else{
                    this->members.insert({member.name,member});
                }
                break;
            case Member::SymbolMember:
                this->members.insert({member.name,member});
                break;
            case Member::FieldMember:
                this->members.insert(make_pair(member.name,member));
                this->memberPosition.insert(make_pair(member.name,layout.size()));
                this->layout.push_back(member.name);
                break;
        }
    }

    shared_ptr<Value> Class::create() {
//        auto ret = new Object();
//        ret->prototype = dynamic_pointer_cast<Class>(shared_from_this());
//        for(const auto& name:layout){
//            auto proto = dynamic_pointer_cast<Type::Symbol>(members.find(name)->second.symbol);
//            auto tmp = dynamic_pointer_cast<Type::Variable>(proto->newInstance());
//            ret->variables.push_back(tmp);
//        }
//        return shared_ptr<Object>(ret);
        throw "unimpl";
    }

    bool Class::equal(shared_ptr<Prototype> ptr) {
        return ptr.get()==this;
    }

    void Class::addImport(shared_ptr<Symbol> child) {
        throw "unimpl";
    }

    EnumInstance::EnumInstance(int value)
        :Instance(InstanceEnum::Enum_),value(value){}

    EnumInstance::EnumInstance()
        :Instance(InstanceEnum::Enum_){

    }

    void EnumInstance::setValue(int value) {
        throw "unimpl";
    }

    void EnumInstance::setPrototype(weak_ptr<Prototype> ptr) {
        Instance::setPrototype(ptr);
        this->value = this->prototype.lock()->getDefalutNumber();
    }

    int EnumInstance::getValue() {
        return value;
    }

    bool Enumeration::equal(shared_ptr<Prototype> ptr) {
        return ptr.get()==this;
    }

    shared_ptr<Value> Enumeration::create() {
        auto ret = new EnumInstance();
        ret->setPrototype(static_pointer_cast<Prototype>(shared_from_this()));
        ret->setValue(defaultValue);
        return shared_ptr<EnumInstance>(ret);
    }

    Enumeration::Enumeration() : Class(DeclarationEnum::Enum_) {

    }

    void Enumeration::addEnumMember(int value,std::string name) {
        this->stringToInt.insert(std::make_pair(name,value));
    }


    shared_ptr<Prototype> Function::getRetSignature() {
        return this->retSignature;
    }

    void Function::setRetSignature(std::shared_ptr<Prototype> ptr){
        retSignature = std::move(ptr);
    }


    void Function::addArgument(Function::Argument arg) {
        this->argsSignature.push_back(arg);
    }




    UserFunction::UserFunction(MethodFlag flag,shared_ptr<Node> implCodeTree)
        :implCodeTree(std::move(implCodeTree)),flag(flag){}

    std::shared_ptr<Node> UserFunction::getImplCodeTree() {
        return this->implCodeTree;
    }

    ExternalFunction::ExternalFunction(std::string library, std::string name)
        : library(std::move(library)),name(std::move(name)){}


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

    Member Domain::lookUp(const string &name) {
        std::shared_ptr<Domain> ptr = static_pointer_cast<Domain>(shared_from_this());
        while(ptr){
            auto p = ptr->find(name);
            if(p!=Member::Empty){
                return p;
            }
            ptr = ptr->getParent().lock();
        }
        return Member::Empty;
    }

    Member Module::find(const string &name) {
        auto ret = this->members.find(name);
        if(ret!=members.end()){
            return ret->second;
        }
        return Member::Empty;
    }



    Member Class::find(const string &name) {
        auto ret = this->members.find(name);
        if(ret!=members.end()){
            return ret->second;
        }
        return Member::Empty;
    }

    void Class::addInherit(std::shared_ptr<Class> base) {
        this->inherit_list.push_back(base);
    }

    void Record::add(Member member) {
        if(member.kind == Member::FieldMember){
            fields.insert({member.name,member});
        }
        else{
            throw "error";
        }
    }


    namespace primitive{
        shared_ptr<Value> VariantClass::create() {
            return make_shared<VariantInstance>();
        }

        VariantClass::VariantClass() {
            setName("variant");
        }

    }

    ValueKind Value::getKind() {
        return kind;
    }

    void Value::setKind(ValueKind kind) {
        this->kind = kind;
    }

    Member Record::find(const string &name) {
        throw "unimpl";
    }

    std::shared_ptr<Value> Record::create() {
        throw "unimpl";
    }

    bool Record::equal(std::shared_ptr<Prototype> ptr) {
        throw "unimpl";
    }

    VariantInstance::VariantInstance(): Instance(InstanceEnum::Variant) {

    }
}
