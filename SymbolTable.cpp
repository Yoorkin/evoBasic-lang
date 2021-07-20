////
//// Created by yorkin on 7/14/21.
////
//
//#include "SymbolTable.h"
//
//#include<utility>
//#include<stack>
//#include<list>
//namespace evoBasic{
//    using namespace std;
//
//    Symbol::Symbol(Prototype* type, const string& name, Scope* scope):type(type),name(name){
//        setParent(scope);
//    }
//
//    Scope* Symbol::getParent(){
//        return parent;
//    }
//
//    void Symbol::setParent(Scope* parent){
//        this->parent=parent;
//        parent->add(this);
//    }
//
//    const string& Symbol::getName(){
//        return name;
//    }
//
//    string Symbol::mangling(){
//        list<string> path;
//        auto p=this;
//        while(true){
//            path.push_front(this->getName());
//            p=p->getParent();
//            if(p->getParent()!=nullptr){
//                path.push_front(".");
//            }
//            else break;
//        }
//        return string(path.begin(),path.end());
//    }
//
//
//    Prototype* Symbol::getPrototype(){
//        return type;
//    }
//
//
//    string BuiltInType::toString(){
//        return BuiltInTypeEnumToString[(int)value];
//    }
//
//    bool BuiltInType::equal(Prototype* x){
//        auto t = dynamic_cast<BuiltInType*>(x);
//        return !(t==nullptr||t->value!=value);
//    }
//
//#define BUILTIN(x) {BuiltInTypeEnum::x,BuiltInType(BuiltInTypeEnum::x)}
//    map<BuiltInTypeEnum,BuiltInType> builtInMap={
//           BUILTIN(i1),BUILTIN(i8),BUILTIN(i16),BUILTIN(i32),BUILTIN(i64),
//           BUILTIN(u8),BUILTIN(u16),BUILTIN(u32),BUILTIN(u64),
//           BUILTIN(f32),BUILTIN(f64)
//    };
//#undef BUILTIN
//
//    BuiltInType* BuiltInType::get(BuiltInTypeEnum value){
//        auto ret = builtInMap.find(value);
//        if(ret==builtInMap.end())return nullptr;
//        return &ret->second;
//    }
//
//    BuiltInType::BuiltInType(BuiltInTypeEnum value): value(value){}
//
//
//    FunctionType::FunctionType(Prototype* ret, vector<Parameter*> param)
//        :returnType(ret),parameters(move(param)){}
//
//
//    bool FunctionType::equal(Prototype* x){
//        auto t = dynamic_cast<FunctionType*>(x);
//        if(t == nullptr || !t->returnType->equal(returnType)) return false;
//
//        for(int i=0;i<parameters.size();i++){
//                if(!parameters[i]->type->equal(t->parameters[i]->type)
//                    || parameters[i]->optional != t->parameters[i]->optional
//                    || parameters[i]->byval!=t->parameters[i]->byval)return false;
//        }
//
//        return true;
//    }
//
//
//    string FunctionType::toString() {
//        list<string> str = {"Function("};
//        for(auto x:parameters){
//            if(x->optional)str.emplace_back("optional ");
//            if(x->byval)str.emplace_back("byval ");
//            else str.emplace_back("byref ");
//            str.push_back(x->type->toString());
//            str.emplace_back(",");
//        }
//        str.pop_back();
//        str.emplace_back(")As ");
//        str.push_back(returnType->toString());
//        return string(str.begin(),str.end());
//    }
//
//
//    StructType::StructType(vector<Prototype*> fields)
//        :fields(move(fields)){}
//
//    bool StructType::equal(Prototype* x){
//        auto t = dynamic_cast<StructType*>(x);
//        if(t==nullptr || t->fields.size()!=fields.size())return false;
//        for(int i=0;i<fields.size();i++){
//            if(fields[i]->equal(t->fields[i]))return false;
//        }
//        return true;
//    }
//
//    string StructType::toString() {
//        list<string> str = {"Prototype("};
//        for(auto x:fields){
//            str.push_back(x->toString());
//            str.emplace_back(",");
//        }
//        str.pop_back();
//        str.emplace_back(")");
//        return string(str.begin(),str.end());
//    }
//
//    ArrayType::ArrayType(Prototype* elementType, vector<int> bounds): elementType(elementType), ranks(move(bounds)){}
//
//    Prototype* ArrayType::getElementType(){
//        return elementType;
//    }
//
//    int ArrayType::getSize(int rank){
//        return ranks[rank];
//    }
//
//    bool ArrayType::equal(Prototype* x){
//        auto t = dynamic_cast<ArrayType*>(x);
//        if(t == nullptr || !elementType->equal(t->elementType) || t->ranks.size() != ranks.size())return false;
//        for(int i=0;i<ranks.size();i++){
//            if(t->ranks[i]!=ranks[i])return false;
//        }
//        return true;
//    }
//
//    string ArrayType::toString() {
//        list<string> str = {"Array("};
//        for(auto x:ranks){
//            str.push_back(to_string(x));
//            str.emplace_back(",");
//        }
//        str.pop_back();
//        str.emplace_back(")As ");
//        str.push_back(elementType->toString());
//        return string(str.begin(),str.end());
//    }
//
//    Constant::Constant(Prototype* type, const string& name, any value, Scope* scope)
//        : Symbol(type, name, scope), value(move(value)){}
//
//
//    Variable::Variable(Prototype* type, const string& name, Scope* scope)
//        : Symbol(type, name, scope){
//        isLocalVariable=(dynamic_cast<Function*>(scope)!=nullptr);
//    }
//
//    string Variable::mangling(){
//        return getName();
//    }
//
//    Scope::Scope(Prototype* type, const string& name, Scope* scope): Symbol(type, name, scope){}
//
//    Symbol* Scope::find(const string& name){
//        auto ret = childs.find(name);
//        if(ret!=childs.end())return nullptr;
//        return ret->second;
//    }
//
//    void Scope::add(Symbol* symbol){
//        childs.insert(make_pair(symbol->getName(),symbol));
//    }
//
//    Symbol* Scope::lookUp(const string& name){
//        auto p = this;
//        while(true){
//            auto ret = p->find(name);
//            if(ret!=nullptr)return ret;
//            p=p->getParent();
//            if(p==nullptr)break;
//        }
//        return nullptr;
//    }
//
//    Function::Function(Symbol* ret, const vector<Parameter*>& parameters, const string& name, Scope* scope)
//        :Scope(new FunctionType(ret->getPrototype(),parameters),name,scope),param(parameters),ret(ret){}
//
//    Struct::Struct(StructType* type,vector<string> fields,const string& name,Scope* scope):Scope(type,name,scope){
//        for(int i=0;i<type->fields.size();i++){
//            new Variable(type->fields[i],fields[i],scope);
//        }
//    }
//
//    BuiltIn::BuiltIn(BuiltInType *type, const string &name,Scope* scope)
//        : Symbol(type, name, scope){}
//
//    Enumeration::Enumeration(EnumerationType* type,const string& name,Scope* scope):Scope(type,name,scope){}
//
//    void Enumeration::addMember(const string& name,int value){
//        new Constant(BuiltInType::get(BuiltInTypeEnum::i32),name,value,this);
//    }
//
//#define BUILTIN_SYMBOL(x) new BuiltIn(BuiltInType::get(BuiltInTypeEnum::x),#x,global)
//    SymbolTable::SymbolTable() {
//        global=new Scope(nullptr,"global",nullptr);
//        BUILTIN_SYMBOL(i1);
//        BUILTIN_SYMBOL(i8);
//        BUILTIN_SYMBOL(i16);
//        BUILTIN_SYMBOL(i32);
//        BUILTIN_SYMBOL(i64);
//        BUILTIN_SYMBOL(u8);
//        BUILTIN_SYMBOL(u16);
//        BUILTIN_SYMBOL(u32);
//        BUILTIN_SYMBOL(u64);
//        BUILTIN_SYMBOL(f32);
//        BUILTIN_SYMBOL(f64);
//    }
//#undef BUILTIN_SYMBOL
//
//    Scope* SymbolTable::getGlobal() {
//        return global;
//    }
//
//    EnumerationType::EnumerationType()= default;
//
//    string EnumerationType::toString(){
//        return "enum";
//    }
//    bool EnumerationType::equal(Prototype* x){
//        return dynamic_cast<EnumerationType*>(x)!=nullptr;
//    }
//}
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
