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


    weak_ptr<Instantiatable> Variant::getPrototype() {
        return weak_ptr<Instantiatable>();//TODO
    }

    Variant::Variant(bool isConstant): Variable(InstanceEnum::Variant){}

    Object::Object() : Variable(InstanceEnum::Object) {}

    weak_ptr<Instantiatable> Object::getPrototype() {
        return this->prototype;
    }

    vector<shared_ptr<Variable>> &Object::getVars() {
        return variables;
    }

    void Object::setPrototype(weak_ptr<Instantiatable> ptr) {
        this->prototype = dynamic_pointer_cast<Class>(ptr.lock());
    }

    void Module::add(shared_ptr<DeclarationSymbol> child) {
        this->members.insert(make_pair(child->getName(),child));
    }

    void Class::add(shared_ptr<DeclarationSymbol> child) {
        this->members.insert(make_pair(child->getName(),child));
        this->memberPosition.insert(make_pair(child->getName(),layout.size()));
        this->layout.push_back(child->getName());
    }

    shared_ptr<Instance> Class::newInstance() {
        auto ret = new Object();
        ret->prototype = shared_from_this();
        for(const auto& name:layout){
            auto proto = dynamic_pointer_cast<Type::Instantiatable>(members.find(name)->second);
            auto tmp = dynamic_pointer_cast<Type::Variable>(proto->newInstance());
            ret->variables.push_back(tmp);
        }
        return shared_ptr<Object>(ret);
    }

    bool Class::equal(shared_ptr<Comparable> ptr) {
        return ptr.get()==this;
    }

    EnumVariable::EnumVariable(int value)
        :Variable(InstanceEnum::Enum_),value(value){}

    EnumVariable::EnumVariable()
        :Variable(InstanceEnum::Enum_){

    }

    void EnumVariable::setValue(int value) {

    }

    void EnumVariable::setPrototype(weak_ptr<Instantiatable> ptr) {
        this->prototype = dynamic_pointer_cast<Enumeration>(ptr.lock());
        this->value = this->prototype.lock()->getDefalutNumber();
    }

    weak_ptr<Instantiatable> EnumVariable::getPrototype() {
        return prototype;
    }

    int EnumVariable::getValue() {
        return value;
    }

    bool Enumeration::equal(shared_ptr<Comparable> ptr) {
        return ptr.get()==this;
    }

    shared_ptr<Instance> Enumeration::newInstance() {
        auto ret = new EnumVariable();
        ret->prototype = shared_from_this();
        ret->value = defaultValue;
        return shared_ptr<EnumVariable>(ret);
    }

    Enumeration::Enumeration() : DeclarationSymbol(DeclarationEnum::Enum_) {

    }

    bool Function::equal(shared_ptr<Comparable> ptr) {
        auto func = dynamic_pointer_cast<Function>(ptr);
        if(!func)return false;
        for(int i=0;i<this->argsSignature.size();i++){
            auto lhs = reinterpret_pointer_cast<Comparable>(this->argsSignature[i]);
            auto rhs = reinterpret_pointer_cast<Comparable>(func->argsSignature[i]);
            if(!lhs->equal(rhs))return false;
        }
        auto lhs = reinterpret_pointer_cast<Comparable>(this->retSignature);
        auto rhs = reinterpret_pointer_cast<Comparable>(func->retSignature);
        if(!lhs->equal(rhs))return false;
        return true;
    }

    Function::Function(FunctionEnum funcKind)
        :DeclarationSymbol(DeclarationEnum::Function), funcKind(funcKind) {}


    Field::Field(shared_ptr<Instantiatable> target)
        :DeclarationSymbol(DeclarationEnum::Field),target(std::move(target)){}

    shared_ptr<Instance> Field::newInstance() {
        return target->newInstance();
    }


    UserFunction::UserFunction(shared_ptr<AST> implCodeTree)
        :Function(FunctionEnum::User),implCodeTree(std::move(implCodeTree)){}

    ExternalFunction::ExternalFunction(std::string library, std::string name)
        : Function(FunctionEnum::External),library(std::move(library)),name(std::move(name)){}

    Variable::Variable(InstanceEnum kind) : kind(kind){}

    void Variable::setConstant(bool value) {
        constant = value;
    }

    bool Variable::isConstant() {
        return constant;
    }


}
