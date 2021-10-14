//
// Created by yorkin on 7/17/21.
//

#ifndef EVOBASIC_TYPE_H
#define EVOBASIC_TYPE_H
#include<map>
#include<string>
#include<list>
#include<utility>
#include<vector>
#include<tuple>
#include<utility>
#include<exception>
#include<memory>
#include"AST.h"
#include"../utils/stringEnum.h"

namespace evoBasic::Type{

#define PRIMITIVE               Variant,Integer,Long,Byte,Boolean

#define INSTANCE_ENUM_LIST      PRIMITIVE,Object,Enum_
#define DECLARATION_ENUM_LIST   PRIMITIVE,Class,Enum_,Type,Function,Module,Field,Variable
#define FUNCTION_ENUM_LIST      User,External,Intrinsic

    enum class InstanceEnum{INSTANCE_ENUM_LIST};
    enum class FunctionEnum{FUNCTION_ENUM_LIST};
    enum class DeclarationEnum{DECLARATION_ENUM_LIST};

/*
 *  谁用宏在项目中搞花样谁就是没事找抽 8/10/2021
    STRING_ENUM_DECLARE(InstanceEnum,INSTANCE_ENUM_LIST);
    STRING_ENUM_DECLARE(FunctionEnum,FUNCTION_ENUM_LIST);
    STRING_ENUM_DECLARE(DeclarationEnum,DECLARATION_ENUM_LIST);
*/

    class Domain;
    class Class;
    class Object;
    //class Instantiatable;
    class DeclarationSymbol;
    class Variant;
    class RValue;
    class LValue;
    class Prototype;
    class Member;
    class Function;

    void strToLowerByRef(string& str);

    enum class ValueKind{lvalue,rvalue};


    class Member{
        Member()=default;
    public:
        static Member FromSymbol(AccessFlag flag,std::shared_ptr<DeclarationSymbol> ptr);
        static Member FromFunction(AccessFlag flag, std::shared_ptr<Function> function);
        static Member FromField(AccessFlag flag,std::string field_name,std::shared_ptr<Prototype> field_prototype);

        std::shared_ptr<DeclarationSymbol> symbol;
        std::string name;
        AccessFlag access;
        enum {SymbolMember,FunctionMember,FieldMember}kind;
        MethodFlag method;
        static Member Empty;
        bool operator==(const Member& rhs) const;
    };

    class Value{
        ValueKind kind;
        std::weak_ptr<Prototype> prototype;
    public:
        Value(const Value&)=delete;
        explicit Value(ValueKind kind):kind(kind){}

        ValueKind getKind();
        void setKind(ValueKind kind);

        virtual void setPrototype(std::weak_ptr<Prototype> ptr){this->prototype = ptr;}
        virtual std::weak_ptr<Prototype> getPrototype(){return this->prototype;}
    };


    class LValue : public Value{
    public:
        explicit LValue():Value(ValueKind::lvalue){}
    };

    class RValue : public Value{
    public:
        explicit RValue():Value(ValueKind::rvalue){}
    };


    template<typename T>
    class PrimitiveValue : public LValue {
        T t;
    public:
        void set(T value){this->t = value;}
        T get(){return this->t;}
    };


    class Instance : public RValue{
        InstanceEnum kind;
    public:
        Instance(const Instance&)=delete;
        explicit Instance(InstanceEnum kind) : kind(kind){}
    };

    class Ref : public LValue{
        std::shared_ptr<Instance> instance;
    public:
        void set(std::shared_ptr<Instance> value){
            this->instance = std::move(value);
        }

        std::shared_ptr<Instance> get(){
            return this->instance;
        }
    };


//    class Instantiatable{
//    public:
//        Instantiatable(const Instantiatable&)=delete;
//        Instantiatable()=default;
//        virtual shared_ptr<Instance> create()=0;
//    };
//
//    class Comparable{
//    public:
//        Comparable(const Comparable&)=delete;
//        Comparable()=default;
//        virtual bool equal(shared_ptr<Comparable> ptr)=0;
//    };



    class DeclarationSymbol : public std::enable_shared_from_this<DeclarationSymbol> {
        friend Domain;
        std::string name;
        DeclarationEnum kind;
    protected:
        weak_ptr<Domain> parent;
    public:
        DeclarationSymbol(const DeclarationSymbol&)=delete;
        explicit DeclarationSymbol(DeclarationEnum kind):kind(kind){}

        virtual std::string getName();
        virtual void setName(std::string str);

        DeclarationEnum getKind(){return kind;}

        virtual std::weak_ptr<Domain> getParent();
        virtual void setParent(std::weak_ptr<Domain> parent);
    };



    class Prototype : public DeclarationSymbol{
    public:
        Prototype(const Prototype&)=delete;
        explicit Prototype(DeclarationEnum kind): DeclarationSymbol(kind){};
        virtual std::shared_ptr<Value> create()=0;
        virtual bool equal(std::shared_ptr<Prototype> ptr)=0;
    };



    template<typename T>
    class Primitive : public Prototype {
    public:
        bool equal(std::shared_ptr<Prototype> ptr)override{
            auto p = dynamic_pointer_cast<Primitive<T>>(ptr);
            return p.operator bool();
        }

        std::shared_ptr<Value> create()override{
            auto ret = make_shared<PrimitiveValue<T>>();
            ret->setPrototype(shared_from_this());
        }
    };


    //domain interface
    class Domain : public Prototype{
    public:
        Domain(const Domain&)=delete;
        explicit Domain(DeclarationEnum kind) : Prototype(kind){}
        virtual void add(Member member)=0;
        virtual Member find(const string& name)=0; //search object in members
        virtual Member lookUp(const string& name); //search object in members and importedModule
        virtual void addImport(std::shared_ptr<DeclarationSymbol> child)=0;
    };

    class Module : public Domain{
        std::map<std::string,Member> members;
        std::list<std::shared_ptr<Module>> importedModule;
    public:
        Module(const Module&)=delete;
        explicit Module(): Domain(DeclarationEnum::Module){}
        void add(Member member)override;
        Member find(const string& name)override;

        void addImport(shared_ptr<DeclarationSymbol> child)override;

        std::shared_ptr<Value> create()override {return {nullptr};}
        bool equal(std::shared_ptr<Prototype> ptr)override {return false;}

    };

    class Record : public Domain {
        std::map<std::string,Member> fields;
    public:
        explicit Record() : Domain(DeclarationEnum::Type){}
        Member find(const string& name)override;
        std::shared_ptr<Value> create()override;
        bool equal(std::shared_ptr<Prototype> ptr)override;

        void add(Member member)override;
        void addImport(std::shared_ptr<DeclarationSymbol>)override{throw "error";}
    };

    class Class : public Domain {
        std::map<std::string,Member> members;
        std::map<std::string,int> memberPosition;
        std::vector<std::string> layout;
        std::list<std::pair<int,Node>> initialize_rules;
        std::map<std::string,Member> virtual_table;
        std::list<std::shared_ptr<DeclarationSymbol>> inherit_list;
    public:
        Class(const Class&)=delete;
        explicit Class(): Domain(DeclarationEnum::Class){}
        explicit Class(DeclarationEnum kind): Domain(kind){}

        std::shared_ptr<Value> create()override;
        bool equal(std::shared_ptr<Prototype> ptr)override;

        void add(Member member)override;
        Member find(const string& name)override;
        void addImport(std::shared_ptr<DeclarationSymbol> child)override;

        //void addInitializeRule(int layout_index,)

        void addInherit(std::shared_ptr<Class> base);
    };

    namespace primitive{
        class VariantClass : public Class{
        public:
            explicit VariantClass();
            std::shared_ptr<Value> create()override;
        };

        class Integer : public Class{
        public:
            explicit Integer();
            std::shared_ptr<Value> create()override;
        };


//        class Boolean : public Class{
//
//            shared_ptr<Instance> create()override;
//        };
//
//        class Long : public Class{
//            shared_ptr<Instance> create()override;
//        };

    }


    class Function: public DeclarationSymbol{
    public:
        struct Argument{
            std::string name;
            bool isByval,isOptional;
            std::shared_ptr<DeclarationSymbol> symbol;
            Argument(std::string name,std::shared_ptr<DeclarationSymbol> symbol,bool isByval,bool isOptional)
                    :name(std::move(name)),symbol(std::move(symbol)),isByval(isByval),isOptional(isOptional){}
        };
    private:
        vector<Argument> argsSignature;
        shared_ptr<DeclarationSymbol> retSignature;
    public:
        Function(const Function&)=delete;
        explicit Function(): DeclarationSymbol(DeclarationEnum::Function){};
        void addArgument(Argument arg);

        shared_ptr<DeclarationSymbol> getRetSignature();
        void setRetSignature(shared_ptr<DeclarationSymbol> ptr);
        virtual MethodFlag getMethodFlag()=0;
    };




//    class Field : public DeclarationSymbol{
//        shared_ptr<DeclarationSymbol> target;
//    public:
//        Field(const Field&)=delete;
//        explicit Field(shared_ptr<DeclarationSymbol> target);
//    };

    class Enumeration : public Class{
        std::map<std::string,int> stringToInt;
        int defaultValue;
    public:
        Enumeration(const Enumeration&)=delete;
        explicit Enumeration();
        int getDefalutNumber() const{return defaultValue;}
        void setDefalutNumber(int num){defaultValue=num;}
        const std::map<std::string,int>& getMapping(){return stringToInt;}
        void addEnumMember(int value,std::string name);

        shared_ptr<Value> create()override;
        bool equal(shared_ptr<Prototype> ptr)override;
    };



    class UserFunction: public Function{
        std::shared_ptr<Node> implCodeTree;
        MethodFlag flag;
    public:
        UserFunction(const UserFunction&)=delete;
        explicit UserFunction(MethodFlag flag,std::shared_ptr<Node> implCodeTree);
        std::shared_ptr<Node> getImplCodeTree();
        MethodFlag getMethodFlag()override{return flag;}
    };

    class ExternalFunction: public Function{
        std::string library,name;
    public:
        ExternalFunction(const ExternalFunction&)=delete;
        explicit ExternalFunction(std::string library,std::string name);
        MethodFlag getMethodFlag()override{return MethodFlag::Normal;}
    };



    class VoidValue : public RValue{};



    class VariantInstance:public Instance{
        union Data{
            int integer_;
            long long_;
            char byte_;
            bool boolean_;
            Object* object_;
        }data{};
        weak_ptr<DeclarationSymbol> prototype;
    public:
        VariantInstance(const Variant&)=delete;
        explicit VariantInstance();
    };

    class EnumInstance:public Instance{
        friend Enumeration;
        weak_ptr<Enumeration> prototype;
        int value;
    public:
        EnumInstance(const Enumeration&)=delete;
        explicit EnumInstance(int value);
        explicit EnumInstance();

        int getValue();
        void setValue(int value);

        void setPrototype(std::weak_ptr<Prototype> ptr)override;
    };

    class RecordInstance : public Instance {
        std::vector<std::shared_ptr<Value>> variables;
    public:
        std::vector<std::shared_ptr<Value>>& getVars(){return variables;}
    };




}


#endif //EVOBASIC_TYPE_H
