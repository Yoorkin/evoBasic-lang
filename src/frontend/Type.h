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
#define DECLARATION_ENUM_LIST   PRIMITIVE,Class,Enum_,Type,Function,Module,Field
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
    class Variable;
    class Function;
    class Instantiatable;
    class DeclarationSymbol;
    class Variant;

    void strToLowerByRef(string& str);

    class Instance{
    public:
        Instance(const Instance&)=delete;
        Instance()=default;
        virtual void setPrototype(weak_ptr<Instantiatable> ptr)=0;
        virtual weak_ptr<Instantiatable> getPrototype()=0;
    };

    class Instantiatable{
    public:
        Instantiatable(const Instantiatable&)=delete;
        Instantiatable()=default;
        virtual shared_ptr<Instance> newInstance()=0;
    };

    class Comparable{
    public:
        Comparable(const Comparable&)=delete;
        Comparable()=default;
        virtual bool equal(shared_ptr<Comparable> ptr)=0;
    };

    class Member{
    public:
        Member(AccessFlag flag,shared_ptr<DeclarationSymbol> symbol)
                :access(flag),symbol(std::move(symbol)),method(MethodFlag::Normal),isMethod(false){}

        Member(AccessFlag flag,MethodFlag method,shared_ptr<DeclarationSymbol> symbol)
                :access(flag),symbol(std::move(symbol)),method(method),isMethod(true){}

        shared_ptr<DeclarationSymbol> symbol;
        AccessFlag access;
        bool isMethod;
        MethodFlag method;
        static Member Empty;
        bool operator==(const Member& rhs) const;
    };


    class DeclarationSymbol{
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

    //domain interface
    class Domain : public DeclarationSymbol,public std::enable_shared_from_this<Domain>{
    public:
        Domain(const Domain&)=delete;
        explicit Domain(DeclarationEnum kind): DeclarationSymbol(kind){};
        virtual void add(Member member)=0;
        virtual Member find(const string& name)=0; //search object in members
        virtual Member lookUp(const string& name); //search object in members and importedModule
        virtual void addImport(const shared_ptr<DeclarationSymbol>& child)=0;
    };

    class Module : public Domain{
        std::map<string,Member> members;
        std::list<shared_ptr<Module>> importedModule;
    public:
        Module(const Module&)=delete;
        explicit Module(): Domain(DeclarationEnum::Module){}
        void add(Member member)override;
        Member find(const string& name)override;
        void addImport(const shared_ptr<DeclarationSymbol>& child)override;
        weak_ptr<Domain> getParent()override;
    };

    class Class : public Domain,
                  public Instantiatable,
                  public Comparable{
        std::map<std::string,Member> members;
        std::map<std::string,int> memberPosition;
        std::vector<std::string> layout;
        std::list<std::pair<int,Node>> initialize_rules;
        std::map<std::string,Member> virtual_table;
        std::list<std::shared_ptr<DeclarationSymbol>> inherit_list;
    public:
        Class(const Class&)=delete;
        explicit Class(): Domain(DeclarationEnum::Class){}
        void add(Member member)override;
        shared_ptr<Instance> newInstance()override;
        bool equal(shared_ptr<Comparable> ptr)override;
        Member find(const string& name)override;
        //void addInitializeRule(int layout_index,)
        void addImport(const shared_ptr<DeclarationSymbol>& child)override;
        weak_ptr<Domain> getParent()override;
        void addInherit(std::shared_ptr<Class> base);
    };

    namespace primitive{
        class VariantClass : public Class{
        public:
            explicit VariantClass();
            shared_ptr<Instance> newInstance()override;
        };

        class Integer : public Class{
        public:
            explicit Integer();
            shared_ptr<Instance> newInstance()override;
        };

//        class Boolean : public Class{
//
//            shared_ptr<Instance> newInstance()override;
//        };
//
//        class Long : public Class{
//            shared_ptr<Instance> newInstance()override;
//        };

    }


    class Function: public DeclarationSymbol,
                    public Comparable{
    public:
        struct Argument{
            std::string name;
            bool isByval,isOptional;
            std::shared_ptr<DeclarationSymbol> symbol;
            Argument(std::string name,std::shared_ptr<DeclarationSymbol> symbol,bool isByval,bool isOptional)
                    :name(std::move(name)),symbol(std::move(symbol)),isByval(isByval),isOptional(isOptional){}
        };
    private:
        FunctionEnum funcKind;
        vector<Argument> argsSignature;
        shared_ptr<DeclarationSymbol> retSignature;
        MethodFlag flag;
    public:
        Function(const Function&)=delete;
        explicit Function(FunctionEnum funcKind);
        void addArgument(Argument arg);
        MethodFlag getMethodFlag();
        void setMethodFlag(MethodFlag flag);
        shared_ptr<DeclarationSymbol> getRetSignature();
        void setRetSignature(shared_ptr<DeclarationSymbol> ptr);
        bool equal(shared_ptr<Comparable> ptr)override;
    };

    class Field : public DeclarationSymbol, public Instantiatable{
        shared_ptr<Instantiatable> target;
    public:
        Field(const Field&)=delete;
        explicit Field(shared_ptr<Instantiatable> target);
        shared_ptr<Instance> newInstance()override;
    };

    class Enumeration : public DeclarationSymbol,
                        public Instantiatable,
                        public Comparable,
                        public enable_shared_from_this<Enumeration>{
        std::map<std::string,int> stringToInt;
        int defaultValue;
    public:
        Enumeration(const Enumeration&)=delete;
        explicit Enumeration();
        int getDefalutNumber() const{return defaultValue;}
        void setDefalutNumber(int num){defaultValue=num;}
        const std::map<std::string,int>& getMapping(){return stringToInt;}
        void addEnumMember(int value,std::string name);
        shared_ptr<Instance> newInstance()override;
        bool equal(shared_ptr<Comparable> ptr)override;
    };



    class UserFunction: public Function{
        shared_ptr<Node> implCodeTree;
    public:
        UserFunction(const UserFunction&)=delete;
        explicit UserFunction(shared_ptr<Node> implCodeTree);
    };

    class ExternalFunction: public Function{
        std::string library,name;
    public:
        ExternalFunction(const ExternalFunction&)=delete;
        explicit ExternalFunction(std::string library,std::string name);
    };

//    use ExternalFunction instead
//    class Intrinsic: public Function{
//        virtual void onCall(vector<shared_ptr<Variable>> args,shared_ptr<Variable> ret)=0;
//    };



    class Variable: public Instance{
        InstanceEnum kind;
        bool constant = false;
    public:
        Variable(const Variable&)=delete;
        explicit Variable(InstanceEnum kind);
        void setConstant(bool value);
        bool isConstant() const;
    };

    class Variant:public Variable{
        union Data{
            int integer_;
            long long_;
            char byte_;
            bool boolean_;
            Object* object_;
        }data{};
        weak_ptr<Instantiatable> prototype;
    public:
        Variant(const Variant&)=delete;
        explicit Variant();
        weak_ptr<Instantiatable> getPrototype()override;
        void setPrototype(weak_ptr<Instantiatable> ptr)override;
    };

    class Object:public Variable{
        friend Class;
        shared_ptr<Class> prototype;
        vector<shared_ptr<Variable>> variables;
    public:
        Object(const Object&)=delete;
        explicit Object();
        vector<shared_ptr<Variable>>& getVars();
        weak_ptr<Instantiatable> getPrototype()override;
        void setPrototype(weak_ptr<Instantiatable> ptr)override;
    };

    class EnumVariable:public Variable{
        friend Enumeration;
        weak_ptr<Enumeration> prototype;
        int value;
    public:
        EnumVariable(const Enumeration&)=delete;
        explicit EnumVariable(int value);
        explicit EnumVariable();
        int getValue();
        void setValue(int value);
        void setPrototype(weak_ptr<Instantiatable> ptr)override;
        weak_ptr<Instantiatable> getPrototype()override;
    };




}


#endif //EVOBASIC_TYPE_H
