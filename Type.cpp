//
// Created by yorkin on 7/17/21.
//

#include "Type.h"
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
        return !(t==nullptr||t->value!=value);
    }

    string BasicType::toString(){

    }

    RecordType::RecordType(const string& name,vector<pair<string,Type*>> fields){

    }
    bool RecordType::equal(Type* type){

    }
    string RecordType::toString(){

    }
}
