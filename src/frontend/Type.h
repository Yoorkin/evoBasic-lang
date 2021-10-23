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
#define DECLARATION_ENUM_LIST   Class,Enum_,EnumMember,Type,Function,Module,Variant,Primitive,FunctionScope,Field,Error
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
    class Symbol;
    class Variant;
    class RValue;
    class LValue;
    class Prototype;
    class Function;

    void strToLowerByRef(string& str);

    enum class ValueKind{lvalue,rvalue};


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



    class Symbol : public std::enable_shared_from_this<Symbol> {
        friend Domain;
        std::string name;
        DeclarationEnum kind;
        AccessFlag access = AccessFlag::Public;
    protected:
        weak_ptr<Domain> parent;
    public:
        const std::string indent_unit = "\t";

        Symbol(const Symbol&)=delete;
        explicit Symbol(DeclarationEnum kind): kind(kind){}

        virtual std::string getName();
        virtual void setName(std::string str);

        DeclarationEnum getKind(){return kind;}

        virtual std::weak_ptr<Domain> getParent();
        virtual void setParent(std::weak_ptr<Domain> parent);

        void setAccessFlag(AccessFlag flag);
        AccessFlag getAccessFlag();

        template<typename T>
        std::shared_ptr<T> as_shared(){
            if(this==nullptr)return shared_ptr<T>(nullptr);
            return dynamic_pointer_cast<T>(shared_from_this());
        }

        virtual std::string debug(int indent)=0;
    };


    class Prototype : public Symbol{
    public:
        Prototype(const Prototype&)=delete;
        explicit Prototype(DeclarationEnum kind): Symbol(kind){};
        virtual std::shared_ptr<Value> create()=0;
        virtual bool equal(std::shared_ptr<Prototype> ptr)=0;
    };

    class Error : public Prototype{
    public:
        Error(const Error&)=delete;
        explicit Error();
        std::string debug(int indent)override;
        bool equal(std::shared_ptr<Prototype> ptr)override{
            return this == ptr.get();
        }
        std::shared_ptr<Value> create()override{
            throw "error";
        }
    };

    class Field : public Symbol{
        std::shared_ptr<Prototype> prototype;
        bool is_const;
        bool need_inference;
    public:
        explicit Field(std::shared_ptr<Prototype> prototype,bool isConstant)
                : Symbol(DeclarationEnum::Field),prototype(prototype),need_inference(false){
            if(!prototype) throw "error";
        }

        explicit  Field(bool isConstant)
            : Symbol(DeclarationEnum::Field),prototype(nullptr),need_inference(true){}

        bool isConstant(){
            return is_const;
        }

        std::shared_ptr<Prototype> getPrototype(){
            return prototype;
        }

        void setPrototype(std::shared_ptr<Prototype> ptr){
            need_inference = true;
            this->prototype = ptr;
        }

        bool isNeedInference(){
            return need_inference;
        }

        std::string debug(int indent)override;

    };


    //domain interface
    class Domain : public Prototype{
    public:
        Domain(const Domain&)=delete;
        explicit Domain(DeclarationEnum kind) : Prototype(kind){}
        virtual void add(std::shared_ptr<Symbol> symbol)=0;
        virtual std::shared_ptr<Symbol> find(const string& name)=0; //search object in members
        virtual std::shared_ptr<Symbol> lookUp(const string& name); //search object in members and importedModule
        virtual void addImport(std::shared_ptr<Symbol> child)=0;
    };

    class Module : public Domain{
        std::map<std::string,std::shared_ptr<Symbol>> members;
        std::list<std::shared_ptr<Module>> importedModule;
    public:
        Module(const Module&)=delete;
        explicit Module(): Domain(DeclarationEnum::Module){}
        void add(std::shared_ptr<Symbol> symbol)override;
        std::shared_ptr<Symbol> find(const string& name)override;

        void addImport(shared_ptr<Symbol> child)override;

        std::shared_ptr<Value> create()override {return {nullptr};}
        bool equal(std::shared_ptr<Prototype> ptr)override {return false;}
        std::string debug(int indent)override;
    };



    class Record : public Domain {
        std::map<std::string,std::shared_ptr<Symbol>> fields;
    public:
        explicit Record() : Domain(DeclarationEnum::Type){}
        std::shared_ptr<Symbol> find(const string& name)override;
        std::shared_ptr<Value> create()override;
        bool equal(std::shared_ptr<Prototype> ptr)override;

        void add(std::shared_ptr<Symbol> symbol)override;
        void addImport(std::shared_ptr<Symbol>)override{throw "error";}
        std::string debug(int indent)override;
    };



    class Class : public Domain {
    protected:
        std::map<std::string,std::shared_ptr<Symbol>> members;
        std::map<std::string,int> memberPosition;
        std::vector<std::string> layout;
        std::list<std::pair<int,Node>> initialize_rules;
        std::map<std::string,std::shared_ptr<Symbol>> virtual_table;
        std::list<std::shared_ptr<Symbol>> inherit_list;
    public:
        Class(const Class&)=delete;
        explicit Class(): Domain(DeclarationEnum::Class){}
        explicit Class(DeclarationEnum kind): Domain(kind){}

        std::shared_ptr<Value> create()override;
        bool equal(std::shared_ptr<Prototype> ptr)override;

        void add(std::shared_ptr<Symbol> symbol)override;
        std::shared_ptr<Symbol> find(const string& name)override;
        void addImport(std::shared_ptr<Symbol> child)override;

        //void addInitializeRule(int layout_index,)

        void addInherit(std::shared_ptr<Class> base);
        std::string debug(int indent)override;
    };

    namespace primitive{

        class VariantClass : public Class{
        public:
            explicit VariantClass();
            std::shared_ptr<Value> create()override;
        };

        template<typename T>
        class PrimitiveValue : public LValue {
            T t;
        public:
            void set(T value){this->t = value;}
            T get(){return this->t;}
        };


        template<typename T>
        class Primitive : public Prototype {
        public:
            explicit Primitive(std::string name) : Prototype(DeclarationEnum::Primitive){
                setName(std::move(name));
                setAccessFlag(AccessFlag::Public);
            };

            bool equal(std::shared_ptr<Prototype> ptr)override{
                auto p = dynamic_pointer_cast<Primitive<T>>(ptr);
                return p.operator bool();
            }

            std::shared_ptr<Value> create()override{
                auto ret = make_shared<PrimitiveValue<T>>();
                ret->setPrototype(static_pointer_cast<Prototype>(shared_from_this()));
                return ret;
            }

            std::string debug(int indent)override{
                stringstream str;
                for(int i=0;i<indent;i++)str<<indent_unit;
                str<<getName()<<" : Primitive\n";
                return str.str();
            }
        };

    }


    class Enumeration : public Class{
        int defaultValue;
    public:
        Enumeration(const Enumeration&)=delete;
        explicit Enumeration();
        int getDefalutNumber() const{return defaultValue;}
        void setDefalutNumber(int num){defaultValue=num;}

        shared_ptr<Value> create()override;
        bool equal(shared_ptr<Prototype> ptr)override;
        std::string debug(int indent)override;
    };

    class EnumMember : public Symbol{
        int index;
    public:
        EnumMember(const Enumeration&)=delete;
        explicit EnumMember(int index): Symbol(DeclarationEnum::EnumMember),index(index){}
        int getIndex()const{return index;}
        std::string debug(int indent)override;
    };

    class Function: public Symbol{
    public:
        struct Argument{
            std::string name;
            bool isByval,isOptional;
            std::shared_ptr<Prototype> symbol;
            Argument(std::string name,std::shared_ptr<Prototype> prototype,bool isByval,bool isOptional)
                    :name(std::move(name)),symbol(std::move(prototype)),isByval(isByval),isOptional(isOptional){}
        };
    private:
        vector<Argument> argsSignature;
        shared_ptr<Prototype> retSignature;
    public:
        Function(const Function&)=delete;
        explicit Function(): Symbol(DeclarationEnum::Function){};
        void addArgument(Argument arg);

        shared_ptr<Prototype> getRetSignature();
        void setRetSignature(shared_ptr<Prototype> ptr);
        virtual MethodFlag getMethodFlag()=0;
        std::string debug(int indent)override;
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

//    class InitFunction: public UserFunction{
//    public:
//        InitFunction(const InitFunction&)=delete;
//        InitFunction(std::shared_ptr<Node> implCodeTree): UserFunction(MethodFlag::Normal,implCodeTree){}
//    };
//

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
        weak_ptr<Symbol> prototype;
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
