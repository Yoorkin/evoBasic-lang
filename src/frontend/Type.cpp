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

    Member Member::Empty{AccessFlag::Public,std::shared_ptr<DeclarationSymbol>()};

    bool Member::operator==(const Member &rhs) const {
        return symbol==rhs.symbol;
    }


    void strToLowerByRef(string& str){
        transform(str.begin(),str.end(),str.begin(),[](unsigned char c){ return std::tolower(c); });
    }

    weak_ptr<Instantiatable> Variant::getPrototype() {
        return prototype;
    }

    void Variant::setPrototype(weak_ptr<Instantiatable> ptr) {
        prototype = ptr;
    }

    Variant::Variant(): Variable(InstanceEnum::Variant){}



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

    void Module::add(Member member) {
        this->members.insert(make_pair(member.symbol->getName(),member));
        member.symbol->setParent(shared_from_this());
    }

    void Module::addImport(const shared_ptr<DeclarationSymbol>& child) {
        if(auto mod = dynamic_pointer_cast<Module>(child)){
            importedModule.push_back(mod);
        }
        else{
            members.insert(make_pair(child->getName(),Member(AccessFlag::Public,child)));
        }
    }

    void Class::add(Member member) {
        if(member.isMethod){
            if(member.method==MethodFlag::Virtual||member.method==MethodFlag::Override){
                virtual_table.insert({member.symbol->getName(),member});
            }
        }
        this->members.insert(make_pair(member.symbol->getName(),member));
        this->memberPosition.insert(make_pair(member.symbol->getName(),layout.size()));
        this->layout.push_back(member.symbol->getName());
        member.symbol->setParent(shared_from_this());
    }

    shared_ptr<Instance> Class::newInstance() {
        auto ret = new Object();
        ret->prototype = dynamic_pointer_cast<Class>(shared_from_this());
        for(const auto& name:layout){
            auto proto = dynamic_pointer_cast<Type::Instantiatable>(members.find(name)->second.symbol);
            auto tmp = dynamic_pointer_cast<Type::Variable>(proto->newInstance());
            ret->variables.push_back(tmp);
        }
        return shared_ptr<Object>(ret);
    }

    bool Class::equal(shared_ptr<Comparable> ptr) {
        return ptr.get()==this;
    }

    void Class::addImport(const shared_ptr<DeclarationSymbol> &child) {

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

    void Enumeration::addEnumMember(int value,std::string name) {
        this->stringToInt.insert(std::make_pair(name,value));
    }

    bool Function::equal(shared_ptr<Comparable> ptr) {
        auto func = dynamic_pointer_cast<Function>(ptr);
        if(!func)return false;
        for(int i=0;i<this->argsSignature.size();i++){
            auto lhs = reinterpret_pointer_cast<Comparable>(this->argsSignature[i].symbol);
            auto rhs = reinterpret_pointer_cast<Comparable>(func->argsSignature[i].symbol);
            if(!lhs->equal(rhs))return false;
        }
        auto lhs = reinterpret_pointer_cast<Comparable>(this->retSignature);
        auto rhs = reinterpret_pointer_cast<Comparable>(func->retSignature);
        if(!lhs->equal(rhs))return false;
        return true;
    }

    Function::Function(FunctionEnum funcKind)
        :DeclarationSymbol(DeclarationEnum::Function), funcKind(funcKind) {}


    shared_ptr<DeclarationSymbol> Function::getRetSignature() {
        return this->retSignature;
    }

    void Function::setRetSignature(std::shared_ptr<DeclarationSymbol> ptr){
        retSignature = std::move(ptr);
    }

    MethodFlag Function::getMethodFlag() {
        return this->flag;
    }

    void Function::setMethodFlag(MethodFlag flag) {
        this->flag = flag;
    }

    void Function::addArgument(Function::Argument arg) {
        this->argsSignature.push_back(arg);
    }


    Field::Field(shared_ptr<Instantiatable> target)
        :DeclarationSymbol(DeclarationEnum::Field),target(std::move(target)){}

    shared_ptr<Instance> Field::newInstance() {
        return target->newInstance();
    }


    UserFunction::UserFunction(shared_ptr<Node> implCodeTree)
        :Function(FunctionEnum::User),implCodeTree(std::move(implCodeTree)){}

    std::shared_ptr<Node> UserFunction::getImplCodeTree() {
        return this->implCodeTree;
    }

    ExternalFunction::ExternalFunction(std::string library, std::string name)
        : Function(FunctionEnum::External),library(std::move(library)),name(std::move(name)){}

    Variable::Variable(InstanceEnum kind) : kind(kind){}

    void Variable::setConstant(bool value) {
        constant = value;
    }

    bool Variable::isConstant() const {
        return constant;
    }

    std::string DeclarationSymbol::getName() {
        return this->name;
    }

    void DeclarationSymbol::setName(std::string str) {
        strToLowerByRef(str);
        this->name=std::move(str);
    }

    std::weak_ptr<Domain> DeclarationSymbol::getParent() {
        return parent;
    }

    void DeclarationSymbol::setParent(std::weak_ptr<Domain> parent) {
        this->parent = parent;
    }

    Member Domain::lookUp(const string &name) {
        std::shared_ptr<Domain> ptr = shared_from_this();
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

    weak_ptr<Domain> Module::getParent() {
        return parent;
    }

    weak_ptr<Domain> Class::getParent() {
        return parent;
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

    namespace primitive{
        shared_ptr<Instance> VariantClass::newInstance() {
            return make_shared<Variant>();
        }

        VariantClass::VariantClass() {
            setName("variant");
        }

        Integer::Integer() {
            setName("integer");
        }

        shared_ptr<Instance> Integer::newInstance() {
            throw "unimplemented";//TODO
        }
    }

    ValueKind Value::getKind() {
        return kind;
    }

    void Value::setKind(ValueKind kind) {
        this->kind = kind;
    }
}
