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

    class Domain{
        Domain* parent;
        map<string,Type*> childs;
    public:
        Domain()=default;
        Type* lookUp(const string& name);
        void add(const string& name,Type* type);
        Type* find(const string& name);
    };

    class Type{
    public:
        virtual bool equal(Type* type)=0;
        virtual string toString()=0;
        Domain* domain{};
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
    public:
        RecordType(const string& name,vector<pair<string,Type*>> fields);
        bool equal(Type* type)override;
        string toString()override;
    };

    class PointerType:public Type{
    public:
        PointerType(Type* elementType);
        bool equal(Type* type)override;
        string toString()override;
    };

    class ArrayType:public Type{
    public:
        ArrayType(Type* elementType,int size);
        bool equal(Type* type)override;
        string toString()override;
    };

    struct Parameter{
        bool isByval,isOptional;
        Type* type;
        Parameter(bool isByval,bool isOptional,Type* type)
            :isByval(isByval),isOptional(isOptional),type(type){}
    };

    class FunctionType:public Type{
    public:
        FunctionType(vector<Parameter> paramsType,Type* retType);
        bool equal(Type* type)override;
        string toString()override;
        Type* getReturnType();
        const vector<Parameter>& getParamsType();
        const vector<Parameter>& getOptionalType();
    };



    // type analysis
    // let i = ModuleA.ModuleB.FunctionC(1,2) <=>
    // lookup(ModuleA)
    //   . ModuleA ModuleB -> ModuleB , . ModuleB FunctionC -> Function(i32,i32)As i32 , FunctionC i32 i32 -> i32
    //  symbol symbol symbol -> symbol  symbol symbol symbol -> function                 function i32 i32 -> i32

}




#endif //EVOBASIC_TYPE_H
