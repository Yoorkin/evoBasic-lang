//
// Created by yorkin on 7/17/21.
//

#ifndef EVOBASIC_TYPE_H
#define EVOBASIC_TYPE_H
#include <map>
#include <string>
#include <list>
#include <utility>
#include <vector>
#include <tuple>
#include <utility>
#include <exception>
#include <sstream>
#include <memory>
#include "ast.h"
#include "token.h"
#include "bytecode.h"

namespace evoBasic::type{

#define PRIMITIVE               Variant,Integer,Long,Byte,Boolean

#define INSTANCE_ENUM_LIST      PRIMITIVE,Object,Enum_
#define DECLARATION_ENUM_LIST   Class,Enum_,EnumMember,Type,Function,Module,Variant,Primitive,TmpDomain, \
                                Variable,Error,Interface,Argument,Array
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
    class Symbol;
    class Variant;
    class Prototype;
    class Function;
    class Interface;

    void strToLowerByRef(std::string& str);

    class Symbol : public std::enable_shared_from_this<Symbol> {
        friend Domain;
        std::string name;
        std::string mangling_name;
        DeclarationEnum kind;
        Location *location_ = nullptr;
        AccessFlag access = AccessFlag::Public;
    protected:
        std::weak_ptr<Domain> parent;
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

        void setLocation(Location *location);
        Location *getLocation();

        template<typename T>
        std::shared_ptr<T> as_shared(){
            if(this==nullptr)return {nullptr};
            return dynamic_pointer_cast<T>(shared_from_this());
        }

        std::string mangling();

        virtual std::string debug(int indent)=0;
    };


    class Prototype : public Symbol{
        data::u32 byte_length = 0;
    public:
        Prototype(const Prototype&)=delete;
        explicit Prototype(DeclarationEnum kind): Symbol(kind){};
        virtual bool equal(std::shared_ptr<Prototype> ptr)=0;

        virtual data::u32 getByteLength();
        virtual void setByteLength(data::u32 value);
    };

    class Error : public Prototype{
    public:
        Error(const Error&)=delete;
        explicit Error();
        std::string debug(int indent)override;
        bool equal(std::shared_ptr<Prototype> ptr)override{
            return this == ptr.get();
        }
    };

    class Variable : public Symbol{
        friend class Record;
        std::shared_ptr<Prototype> prototype{nullptr};
        bool is_const = false;
        bool is_global = false;
        std::size_t offset = -1;
    public:
        Variable();
        explicit Variable(DeclarationEnum kind);
        bool isConstant();
        bool isGlobal();
        void setGlobal();
        void setConstant(bool value);
        std::shared_ptr<Prototype> getPrototype();
        void setPrototype(std::shared_ptr<Prototype> ptr);
        std::string debug(int indent)override;
        std::size_t getOffset();
        void setOffset(std::size_t value);
        virtual data::u32 getRealByteLength();
    };

    //domain interface
    class Domain : public Prototype{
        using MemberMap = std::map<std::string,std::shared_ptr<Symbol>>;
        MemberMap childs;
        std::vector<std::shared_ptr<Variable>> memory_layout;
    public:
        class iterator : public std::iterator<std::input_iterator_tag,
                                                    std::shared_ptr<Symbol>,
                                                    std::ptrdiff_t,
                                                    std::shared_ptr<Symbol>,
                                                    std::shared_ptr<Symbol>&>{
            MemberMap::iterator iterator_;
        public:
            explicit iterator(MemberMap::iterator iter) : iterator_(iter){}
            iterator& operator++() {iterator_++; return *this;}
            iterator operator++(int) {auto tmp = *this; iterator_++; return tmp;}
            bool operator==(const iterator& other) const {return other.iterator_==this->iterator_;}
            bool operator!=(const iterator& other) const {return !(other==*this);}
            Symbol* operator->() const {return iterator_->second.get();}
            Symbol* operator*() const {return iterator_->second.get();}
        };
        Domain(const Domain&)=delete;
        explicit Domain(DeclarationEnum kind) : Prototype(kind){}
        virtual void add(std::shared_ptr<Symbol> symbol);
        virtual std::shared_ptr<Symbol> find(const std::string& name); //search object in members
        virtual std::shared_ptr<Symbol> lookUp(const std::string& name); //search object in members and importedModule
        iterator begin();
        iterator end();
        void updateMemoryLayout();
        void addMemoryLayout(std::shared_ptr<Variable> variable);
    };

    class Module : public Domain{
        std::list<std::shared_ptr<Module>> importedModule;
    public:
        Module(const Module&)=delete;
        explicit Module(): Domain(DeclarationEnum::Module){}

        void addImport(std::shared_ptr<Symbol> child);

        bool equal(std::shared_ptr<Prototype> ptr)override {return false;}
        std::string debug(int indent)override;
    };

    class Record : public Domain {
        std::vector<std::shared_ptr<Variable>> fields;
    public:
        explicit Record() : Domain(DeclarationEnum::Type){}
        explicit Record(DeclarationEnum kind) : Domain(kind){}
        bool equal(std::shared_ptr<Prototype> ptr)override;

        const std::vector<std::shared_ptr<Variable>>& getFields();
        void add(std::shared_ptr<Symbol> symbol)override;
        std::string debug(int indent)override;
    };

    class Class : public Record {
    protected:
        //std::list<ast::Variable*> initialize_rules;

        std::shared_ptr<Class> base_class;
        std::list<std::shared_ptr<Interface>> impl_interface;
        std::shared_ptr<Function> constructor;
    public:
        Class(const Class&)=delete;
        explicit Class();
        explicit Class(DeclarationEnum kind): Record(kind){}

        bool equal(std::shared_ptr<Prototype> ptr)override;

        void setExtend(std::shared_ptr<Class> base);
        void setConstructor(std::shared_ptr<Function> constructor);
        void addImplementation(std::shared_ptr<Interface> interface);
        void addInitializeRule(ast::Variable* variable_node);

        std::string debug(int indent)override;
    };

    class Interface : public Domain{
    public:
        explicit Interface():Domain(DeclarationEnum::Interface){}
        void add(std::shared_ptr<Symbol> symbol)final;

        bool equal(std::shared_ptr<Prototype> ptr)final{throw "error";}

        std::string debug(int indent)final;
    };

    namespace primitive{

        class VariantClass : public Class{
        public:
            explicit VariantClass();
        };


        class Primitive : public Class {
            vm::Data kind_;
        public:
            explicit Primitive(std::string name,vm::Data data_kind);
            bool equal(std::shared_ptr<Prototype> ptr)override;
            std::string debug(int indent)override;
            vm::Data getDataKind();
        };

    }

    class Enumeration : public Class{
        int defaultValue;
    public:
        Enumeration(const Enumeration&)=delete;
        explicit Enumeration();
        int getDefaultNumber() const{return defaultValue;}
        void setDefaultNumber(int num){ defaultValue=num;}
        void add(std::shared_ptr<Symbol> symbol) override;

        bool equal(std::shared_ptr<Prototype> ptr)override;
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

    class Argument : public Variable{
        bool is_byval,is_optional;
    public:
        Argument(std::string name,std::shared_ptr<Prototype> prototype,bool isByval,bool isOptional);
        std::string debug(int indent)override;
        bool isByval();
        bool isOptional();
        data::u32 getRealByteLength()override;
    };

    class Array : public Class{
        std::shared_ptr<Prototype> element_type;
        data::u32 size_;
    public:
        explicit Array(std::shared_ptr<Prototype> element,data::u32 size);
        std::shared_ptr<Prototype> getElementPrototype();
        bool equal(std::shared_ptr<Prototype> ptr)override;
        std::string debug(int indent)override;
        data::u32 getByteLength()override;
        data::u32 getSize(){return size_;}
    };


    class Function: public Domain{
    private:
        std::vector<std::shared_ptr<Argument>> argsSignature;
        std::shared_ptr<Prototype> retSignature;
        std::size_t tmp_domain_count = 0;
    public:
        Function(const Function&)=delete;
        explicit Function();

        void add(std::shared_ptr<Symbol> symbol)override;

        const std::vector<std::shared_ptr<Argument>>& getArgsSignature();
        std::shared_ptr<Prototype> getRetSignature();
        void setRetSignature(std::shared_ptr<Prototype> ptr);
        virtual MethodFlag getMethodFlag()=0;
        std::string debug(int indent)override;

        bool equal(std::shared_ptr<Prototype> ptr)override;
    };


    class UserFunction: public Function{
        ast::Function *function_node;
        MethodFlag flag;
    public:
        UserFunction(const UserFunction&)=delete;
        explicit UserFunction(MethodFlag flag,ast::Function *function_node);
        ast::Function *getFunctionNode();
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
        MethodFlag getMethodFlag()override{return MethodFlag::NonMethod;}
    };

    class TemporaryDomain : public Domain {
        std::shared_ptr<UserFunction> parent_function;
    public:
        explicit TemporaryDomain(std::weak_ptr<type::Domain> parent,std::shared_ptr<UserFunction> function);
        void add(std::shared_ptr<type::Symbol> symbol)override;
        bool equal(std::shared_ptr<Prototype> ptr)override{throw "error";};
        std::string debug(int indent)override;
    };

}


#endif //EVOBASIC_TYPE_H
