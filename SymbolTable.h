////
//// Created by yorkin on 7/14/21.
////
//
//#ifndef EVOBASIC_SYMBOLTABLE_H
//#define EVOBASIC_SYMBOLTABLE_H
//#include<string>
//#include<map>
//#include<unordered_map>
//#include<utility>
//#include<vector>
//#include<any>
//namespace evoBasic{
//    using namespace std;
//
//    class Scope;
//    class Prototype;
//
//    enum class BuiltInTypeEnum:int{i1=0,i8,i16,i32,i64,f32,f64,u8,u16,u32,u64};
//    vector<string> BuiltInTypeEnumToString = {"i1","i8","i16","i32","i64","f32","f64","u8","u16","u32","u64"};
//
//    class Symbol{
//        Prototype* type;
//        Scope* parent;
//        string name;
//    public:
//        Symbol(Prototype* type, const string& name, Scope* scope);
//        virtual Prototype* getPrototype();
//        virtual Scope* getParent();
//        virtual void setParent(Scope* parent);
//        virtual const string& getName();
//        virtual string mangling();
//    };
//
//    class Prototype{
//    public:
//        virtual string toString()=0;
//        virtual bool equal(Prototype* x)=0;
//    };
//
//
//    class BuiltInType: public Prototype{
//        BuiltInTypeEnum value;
//        static map<BuiltInTypeEnum,BuiltInType> builtInMap;
//    public:
//        BuiltInType(BuiltInTypeEnum value);
//        string toString()override;
//        bool equal(Prototype* x)override;
//        static BuiltInType* get(BuiltInTypeEnum value);
//    };
//
//    class EnumerationType:public Prototype{
//    public:
//        EnumerationType();
//        string toString()override;
//        bool equal(Prototype* x)override;
//    };
//
//
//    class Parameter{
//    public:
//        Symbol* type;
//        bool byval;
//        bool optional;
//        string name;
//        Parameter(Symbol* type, bool byval, bool optional,const string& name)
//                :type(type),byval(byval),optional(optional),name(name){}
//    };
//
//
//    class FunctionType:public Prototype{
//    public:
//        FunctionType(Prototype* ret, vector<Parameter*> parameters);
//        vector<Parameter*> parameters;
//        Prototype* returnType;
//        bool equal(Prototype* x)override;
//        string toString()override;
//    };
//
//    class LambdaType:public Prototype{};
//
//    class StructType:public Prototype{
//    public:
//        explicit StructType(vector<Prototype*> fields);
//        vector<Prototype*> fields;
//        bool equal(Prototype* x)override;
//        string toString()override;
//    };
//
//    class ArrayType:public Prototype{
//        Prototype* elementType;
//        vector<int> ranks;
//    public:
//        ArrayType(Prototype* elementType, vector<int> bounds);
//        Prototype* getElementType();
//        int getSize(int rank);
//        bool equal(Prototype* x)override;
//        string toString()override;
//    };
//
//
//    class Constant:public Symbol{
//        any value;
//    public:
//        Constant(Prototype* type, const string& name, any value, Scope* scope);
//        template<typename T>
//        T getValue(){
//            return any_cast<T>(value);
//        }
//    };
//
//
//    class Variable:public Symbol{
//        bool isLocalVariable;
//    public:
//        Variable(Prototype* type, const string& name, Scope* scope);
//        string mangling()override;
//    };
//
//
//    class Scope:public Symbol{
//        map<string,Symbol*> childs;
//    public:
//        Scope(Prototype* type, const string& name, Scope* scope);
//        Symbol* find(const string& name);
//        void add(Symbol* symbol);
//        Symbol* lookUp(const string& name);
//    };
//
//
//    class Function:public Scope{
//
//    public:
//        Function(Symbol* ret, const vector<Parameter*>& parameters, const string& name, Scope* scope);
//        FunctionType* getPrototype()override{return (FunctionType*)(Symbol::getPrototype());}
//        vector<Parameter*> param;
//        Symbol* ret;
//    };
//
//
//    class Struct:public Scope{
//    public:
//        Struct(StructType* type,vector<string> fields,const string& name,Scope* scope);
//        StructType* getPrototype()override{return (StructType*)(Symbol::getPrototype());}
//    };
//
//    class BuiltIn:public Symbol{
//    public:
//        BuiltIn(BuiltInType* type,const string& name,Scope* scope);
//        BuiltInType* getPrototype()override{return (BuiltInType*) Symbol::getPrototype();}
//    };
//
//    class Enumeration:public Scope{
//    public:
//        Enumeration(EnumerationType* type,const string& name,Scope* scope);
//        void addMember(const string& name,int value);
//    };
//
//
//    class SymbolTable{
//        Scope* global;
//    public:
//        SymbolTable();
//        Scope* getGlobal();
//    };
//}
//
//
//#endif //EVOBASIC_SYMBOLTABLE_H
