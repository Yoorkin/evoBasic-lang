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

    class Symbol;
    class Class;
    class Object;
    class Variable;
    class Function;
    class Enum;
    class Instantiatable;
    class DeclarationSymbol;

    class Instance{
    public:
        virtual void setPrototype(weak_ptr<Instantiatable> ptr)=0;
        virtual weak_ptr<Instantiatable> getPrototype()=0;
    };

    class Instantiatable{
    public:
        virtual shared_ptr<Instance> newInstance()=0;
    };

    class Comparable{
    public:
        virtual bool equal(shared_ptr<Comparable> ptr)=0;
    };

    //domain interface
    class Domain{
    public:
        virtual void add(shared_ptr<DeclarationSymbol> child)=0;
    };




    class DeclarationSymbol{
        friend Domain;
        std::string name;
        weak_ptr<Domain> parent;
        DeclarationEnum kind;
    public:
        explicit DeclarationSymbol(DeclarationEnum kind):kind(kind){}

        virtual std::string getName(){return name;};
        virtual void setName(std::string str){name=std::move(str);};
        DeclarationEnum getKind(){return kind;}
    };



    class Module : public DeclarationSymbol,
                   public Domain{
        map<string,shared_ptr<DeclarationSymbol>> members;
    public:
        explicit Module(): DeclarationSymbol(DeclarationEnum::Module){}
        void add(shared_ptr<DeclarationSymbol> child)override;
    };

    class Class : public DeclarationSymbol,
                  public Domain,
                  public Instantiatable,
                  public Comparable,
                  public std::enable_shared_from_this<Class>{
        std::map<string,shared_ptr<DeclarationSymbol>> members;
        std::map<string,int> memberPosition;
        std::vector<std::string> layout;
    public:
        explicit Class(): DeclarationSymbol(DeclarationEnum::Class){}
        void add(shared_ptr<DeclarationSymbol> child)override;
        shared_ptr<Instance> newInstance()override;
        bool equal(shared_ptr<Comparable> ptr)override;
    };

    class Function: public DeclarationSymbol,
                    public Comparable{
        FunctionEnum funcKind;
        vector<shared_ptr<DeclarationSymbol>> argsSignature;
        shared_ptr<DeclarationSymbol> retSignature;
    public:
        explicit Function(FunctionEnum funcKind);
        bool equal(shared_ptr<Comparable> ptr)override;
    };

    class Field : public DeclarationSymbol, public Instantiatable{
        shared_ptr<Instantiatable> target;
    public:
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
        explicit Enumeration();
        int getDefalutNumber() const{return defaultValue;}
        std::map<std::string,int>& getMapping(){return stringToInt;}
        shared_ptr<Instance> newInstance()override;
        bool equal(shared_ptr<Comparable> ptr)override;
    };



    class UserFunction: public Function{
        shared_ptr<AST> implCodeTree;
    public:
        explicit UserFunction(shared_ptr<AST> implCodeTree);
    };

    class ExternalFunction: public Function{
        std::string library,name;
    public:
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
        explicit Variable(InstanceEnum kind);
        void setConstant(bool value);
        bool isConstant();
    };

    class Variant:public Variable{
        union Data{
            int integer_;
            long long_;
            char byte_;
            bool boolean_;
            Object* object_;
        }data{};
    public:
        explicit Variant(bool isConstant);
        weak_ptr<Instantiatable> getPrototype()override;
    };

    class Object:public Variable{
        friend Class;
        shared_ptr<Class> prototype;
        vector<shared_ptr<Variable>> variables;
    public:
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
        explicit EnumVariable(int value);
        explicit EnumVariable();
        int getValue();
        void setValue(int value);
        void setPrototype(weak_ptr<Instantiatable> ptr)override;
        weak_ptr<Instantiatable> getPrototype()override;
    };




}


#endif //EVOBASIC_TYPE_H
