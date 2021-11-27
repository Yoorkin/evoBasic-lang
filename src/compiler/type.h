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
 //   enum class DeclarationEnum{DECLARATION_ENUM_LIST};

/*
 *  谁用宏在项目中搞花样谁就是没事找抽 8/10/2021
    STRING_ENUM_DECLARE(InstanceEnum,INSTANCE_ENUM_LIST);
    STRING_ENUM_DECLARE(FunctionEnum,FUNCTION_ENUM_LIST);
    STRING_ENUM_DECLARE(DeclarationEnum,DECLARATION_ENUM_LIST);
*/

    class Domain;
    class Class;
    class Symbol;
    class Variant;
    class Prototype;
    class Function;
    class Interface;
    class Variable;

    void strToLowerByRef(std::string& str);

    enum class SymbolKind{
        Error,Class,Enum,EnumMember,Record,Function,Module,
        Primitive,TmpDomain,Variable,Argument,Array,Interface
    };

    class Symbol {
    private:
        friend Domain;
        std::string name;
        std::string mangling_name;
        SymbolKind kind;
        Location *location_ = nullptr;
        AccessFlag access = AccessFlag::Public;
    protected:
        Domain *parent = nullptr;
    public:
        const std::string indent_unit = "\t";

        Symbol(const Symbol&)=delete;
        explicit Symbol(SymbolKind kind): kind(kind){}

        virtual std::string getName();
        virtual void setName(std::string str);

        SymbolKind getKind(){return kind;}

        virtual Domain *getParent();
        virtual void setParent(Domain *parent);

        void setAccessFlag(AccessFlag flag);
        AccessFlag getAccessFlag();

        void setLocation(Location *location);
        Location *getLocation();

        template<typename T>
        T as(){
            return dynamic_cast<T>(this);
        }

        std::string mangling(char separator = '$');

        virtual std::string debug(int indent)=0;
    };


    class Prototype : public Symbol{
        data::ptr byte_length = 0;
    public:
        Prototype(const Prototype&)=delete;
        explicit Prototype(SymbolKind kind): Symbol(kind){};
        virtual bool equal(Prototype *ptr)=0;

        virtual data::ptr getByteLength();
        virtual void setByteLength(data::ptr value);
    };

    class Error : public Prototype{
    public:
        Error(const Error&)=delete;
        explicit Error();
        std::string debug(int indent)override;
        bool equal(Prototype *ptr)override{
            return this == ptr;
        }
    };

    class Variable : public Symbol{
        friend class Record;
        Prototype *prototype{nullptr};
        bool is_const = false;
        bool is_global = false;
        std::size_t offset = -1;
        bool is_static = false;
    public:
        Variable();
        explicit Variable(SymbolKind kind);
        bool isConstant();
        void setConstant(bool value);
        Prototype *getPrototype();
        void setPrototype(Prototype *ptr);
        std::string debug(int indent)override;
        std::size_t getOffset();
        void setOffset(std::size_t value);
        virtual data::ptr getRealByteLength();
        bool isStatic();
        void setStatic(bool value);
    };

    //domain interface
    class Domain : public Prototype{
        using MemberMap = std::map<std::string,Symbol*>;
        MemberMap childs;
        std::vector<Variable*> memory_layout;
    public:
        class iterator : public std::iterator<std::input_iterator_tag,
                                                    Symbol*,
                                                    std::ptrdiff_t,
                                                    Symbol*,
                                                    Symbol*>{
            MemberMap::iterator iterator_;
        public:
            explicit iterator(MemberMap::iterator iter) : iterator_(iter){}
            iterator& operator++() {iterator_++; return *this;}
            iterator operator++(int) {auto tmp = *this; iterator_++; return tmp;}
            bool operator==(const iterator& other) const {return other.iterator_==this->iterator_;}
            bool operator!=(const iterator& other) const {return !(other==*this);}
            Symbol* operator->() const {return iterator_->second;}
            Symbol* operator*() const {return iterator_->second;}
        };
        Domain(const Domain&)=delete;
        explicit Domain(SymbolKind kind) : Prototype(kind){}
        virtual void add(Symbol *symbol);
        virtual Symbol *find(const std::string& name); //search object in members
        virtual Symbol *lookUp(const std::string& name); //search object in members and importedModule
        iterator begin();
        iterator end();
        void updateMemoryLayout();
        void addMemoryLayout(Variable *variable);
    };

    class Module : public Domain{
        std::list<Module*> importedModule;
    public:
        Module(const Module&)=delete;
        explicit Module(): Domain(SymbolKind::Module){}

        void addImport(Symbol *child);

        bool equal(Prototype *ptr)override {return false;}
        std::string debug(int indent)override;
    };

    class Record : public Domain {
        std::vector<Variable*> fields;
    public:
        explicit Record() : Domain(SymbolKind::Record){}
        explicit Record(SymbolKind kind) : Domain(kind){}
        bool equal(Prototype *ptr)override;

        const std::vector<Variable*>& getFields();
        void add(Symbol *symbol)override;
        std::string debug(int indent)override;
    };

    class Class : public Record {
    protected:
        //std::list<ast::Variable*> initialize_rules;

        Class *base_class;
        std::list<Interface*> impl_interface;
        Function *constructor;
    public:
        Class(const Class&)=delete;
        explicit Class();
        explicit Class(SymbolKind kind): Record(kind){}

        bool equal(Prototype *ptr)override;

        void add(Symbol *symbol) override;

        void setExtend(Class *base);
        void setConstructor(Function *constructor);
        void addImplementation(Interface *interface);
        void addInitializeRule(ast::Variable* variable_node);

        std::string debug(int indent)override;
    };

    class Interface : public Domain{
    public:
        explicit Interface():Domain(SymbolKind::Interface){}
        void add(Symbol *symbol)final;

        bool equal(Prototype *ptr)final{PANIC;}

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
            bool equal(Prototype *ptr)override;
            std::string debug(int indent)override;
            vm::Data getDataKind();
        };

    }
    using namespace primitive;

    class Enumeration : public Class{
        int defaultValue;
    public:
        Enumeration(const Enumeration&)=delete;
        explicit Enumeration();
        int getDefaultNumber() const{return defaultValue;}
        void setDefaultNumber(int num){ defaultValue=num;}
        void add(Symbol *symbol) override;

        bool equal(Prototype *ptr)override;
        std::string debug(int indent)override;
    };

    class EnumMember : public Symbol{
        int index;
    public:
        EnumMember(const Enumeration&)=delete;
        explicit EnumMember(int index): Symbol(SymbolKind::EnumMember),index(index){}
        int getIndex()const{return index;}
        std::string debug(int indent)override;
    };

    class Parameter : public Variable{
        bool is_byval,is_optional;
    public:
        Parameter(std::string name, Prototype *prototype, bool isByval, bool isOptional);
        std::string debug(int indent)override;
        bool isByval();
        bool isOptional();
        data::ptr getRealByteLength()override;
    };

    class Array : public Class{
        Prototype *element_type;
        data::ptr size_;
    public:
        explicit Array(Prototype *element,data::u32 size);
        Prototype *getElementPrototype();
        bool equal(Prototype *ptr)override;
        std::string debug(int indent)override;
        data::ptr getByteLength()override;
        data::ptr getSize(){return size_;}
    };


    class Function: public Domain{
    public:
        enum Flag{Method,Static,Virtual};
    private:
        std::vector<Parameter*> argsSignature;
        Prototype *retSignature = nullptr;
        std::size_t tmp_domain_count = 0;
    public:
        Function(const Function&)=delete;
        explicit Function();

        void add(Symbol *symbol)override;

        std::vector<Parameter*>& getArgsSignature();
        Prototype *getRetSignature();
        void setRetSignature(Prototype *ptr);
        virtual Flag getFunctionFlag()=0;
        std::string debug(int indent)override;

        bool equal(Prototype *ptr)override;
    };

    class UserFunction: public Function{
        ast::Function *function_node;
        Function::Flag flag;
        bool is_static = false;
    public:
        UserFunction(const UserFunction&)=delete;
        explicit UserFunction(Function::Flag flag,ast::Function *function_node);
        ast::Function *getFunctionNode();
        Function::Flag getFunctionFlag()override{return flag;}
        bool isStatic();
        void setStatic(bool value);
    };


    class ExternalFunction: public Function{
        std::string library,name;
    public:
        ExternalFunction(const ExternalFunction&)=delete;
        explicit ExternalFunction(std::string library,std::string name);
        Function::Flag getFunctionFlag()override{return Function::Flag::Static;}
    };

    class TemporaryDomain : public Domain {
        UserFunction *parent_function;
    public:
        explicit TemporaryDomain(type::Domain *parent,UserFunction *function);
        void add(type::Symbol *symbol)override;
        bool equal(Prototype *ptr)override{PANIC;};
        std::string debug(int indent)override;
    };

}


#endif //EVOBASIC_TYPE_H
