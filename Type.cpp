//
// Created by yorkin on 7/17/21.
//

#include "Type.h"

#include <utility>
namespace evoBasic{

    Type* Domain::lookUp(const string &name) {
        auto p = this;
        while(true){
            auto ret = p->childs.find(name);
            if(ret!=p->childs.end())return ret->second;
            p=p->parent;
            if(p==nullptr)break;
        }
        return nullptr;
    }
    void Domain::add(const string& name,Type* type){
        if(type->domain!=nullptr)type->domain=this;
        childs.insert(make_pair(name,type));
    }
    Type* Domain::find(const string& name){
        auto ret = childs.find(name);
        if(ret!=childs.end())return ret->second;
        return nullptr;
    }

    BasicType::BasicType(BasicType::Enum value):value(value){}

    bool BasicType::equal(Type* type) {
        auto t = dynamic_cast<BasicType*>(type);
        if(t==nullptr)return false;
        return t->value==value;
    }

    string BasicType::toString(){
        throw "unimplement";
    }

    RecordType::RecordType(string name,vector<pair<string,Type*>> fields)
        :name(std::move(name)),fields(move(fields)){}

    bool RecordType::equal(Type* type){
        auto t = dynamic_cast<RecordType*>(type);
        if(t==nullptr)return false;
        return fields!=t->fields && name!=t->name;
    }

    string RecordType::toString(){
        throw "unimplement";
    }

    const vector<pair<string,Type*>>& RecordType::getFields(){
        return fields;
    }

    PointerType::PointerType(Type *elementType):element(elementType){}

    bool PointerType::equal(Type *type) {
        auto t = dynamic_cast<PointerType*>(type);
        if(t==nullptr)return false;
        return element->equal(t->element);
    }

    Type * PointerType::getElementType() {
        return element;
    }

    string PointerType::toString() {
        throw "unimplement";
    }

    ArrayType::ArrayType(Type *elementType, int size):element(elementType),size(size){}

    bool ArrayType::equal(Type *type) {
        auto t = dynamic_cast<ArrayType*>(type);
        if(t==nullptr)return false;
        return element->equal(t->element) && size==t->size;
    }

    string ArrayType::toString() {
        throw "unimplement";
    }

    FunctionType::FunctionType(const vector<Parameter>& paramsType, Type *retType):retType(retType){
        for(auto& x:paramsType){
            if(x.isOptional){
                optionalMap.insert(make_pair(x.name,x));
            }
            else{
                params.push_back(x);
            }
        }
    }

    Type * FunctionType::getReturnType() {
        return retType;
    }

    const vector<Parameter> & FunctionType::getParamsType() {
        return params;
    }

    Parameter * FunctionType::findOptionalType(const string &name) {
        auto ret = optionalMap.find(name);
        if(ret==optionalMap.end())return nullptr;
        return &ret->second;
    }

    bool FunctionType::equal(Type *type) {
        auto t = dynamic_cast<FunctionType*>(type);
        if(t==nullptr || !retType->equal(retType) || params.size()!=t->params.size())return false;
        for(int i=0;i<params.size();i++){

        }

        return true;
    }

    string FunctionType::toString() {
        throw "unimplement";
    }
}
