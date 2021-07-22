//
// Created by yorkin on 7/17/21.
//

#ifndef EVOBASIC_TYPE_H
#define EVOBASIC_TYPE_H
#include<map>
#include<string>
#include<list>
#include<vector>
#include<tuple>
namespace evoBasic{
    using namespace std;
    class Type;
    class FunctionType;

    class Domain{
        Domain* parent=nullptr;
        map<string,Type*> childs;
    public:
        Domain()=default;
        Type* lookUp(const string& name);
        virtual void add(const string& name,Type* type);
        virtual Type* find(const string& name);
    };

    class Type{
    public:
        virtual bool equal(Type* type)=0;
        virtual string toString()=0;
        Domain* domain{};
    };

    enum class AccessFlag{Public,Private,Friend,Static};
    enum class MethodFlag{Virtual,Override,Normal};
    struct Method{

    };

    class ClassType:public Type,public Domain{
    public:
        ClassType(string name);
        const map<string,tuple<AccessFlag,MethodFlag,FunctionType*>>& getMethodMap();
    };

    class Symbol:public Type{
    public:
        Type* attched;
        explicit Symbol(Type* attched):attched(attched){}
        bool equal(Type* type)override{return false;}
        string toString()override{return "Symbol";}
    };

    class BasicType:public Type{
    public:
        enum Enum{i32,i64,f32,f64};
        explicit BasicType(Enum value);
        bool equal(Type* type)override;
        string toString()override;
    private:
        Enum value;
    };

    class RecordType:public Type{
        vector<pair<string,Type*>> fields;
        string name;
    public:
        RecordType(string  name,vector<pair<string,Type*>> fields);
        bool equal(Type* type)override;
        string toString()override;
        const vector<pair<string,Type*>>& getFields();
    };

    class PointerType:public Type{
        Type *element;
    public:
        explicit PointerType(Type* elementType);
        bool equal(Type* type)override;
        string toString()override;
        Type *getElementType();
    };

    class ArrayType:public Type{
        Type *element;
        int size;
    public:
        ArrayType(Type* elementType,int size);
        bool equal(Type* type)override;
        string toString()override;
    };

    struct Parameter{
        bool isByval,isOptional;
        Type* type;
        string name;
        Parameter(bool isByval,bool isOptional,Type* type,string name)
            :isByval(isByval),isOptional(isOptional),type(type),name(move(name)){}
    };

    class FunctionType:public Type{
        vector<Parameter> params;
        map<string,Parameter> optionalMap;
        Type *retType;
    public:
        FunctionType(const vector<Parameter>& paramsType,Type* retType);
        bool equal(Type* type)override;
        string toString()override;
        Type* getReturnType();
        const vector<Parameter>& getParamsType();
        Parameter *findOptionalType(const string& name);
    };



    // type analysis
    // let i = ModuleA.ModuleB.FunctionC(1,2) <=>
    // lookup(ModuleA)
    //   . ModuleA ModuleB -> ModuleB , . ModuleB FunctionC -> Function(i32,i32)As i32 , FunctionC i32 i32 -> i32
    //  symbol symbol symbol -> symbol  symbol symbol symbol -> function                 function i32 i32 -> i32

}




#endif //EVOBASIC_TYPE_H
