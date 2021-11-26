//
// Created by yorkin on 11/1/21.
//

#ifndef EVOBASIC2_CONTEXT_H
#define EVOBASIC2_CONTEXT_H
#include <memory>
#include <set>
#include <functional>
#include <optional>
#include "type.h"
#include "data.h"
#include "dependencies.h"

namespace evoBasic{

    using SymbolPtr = type::Symbol*;
    using PromotionRuleFunction = std::function<std::shared_ptr<type::Prototype>(ast::Node *node)>;
    using BinaryOpSignature = std::pair<SymbolPtr,SymbolPtr>;
    class Context;

    class BuiltIn{
        friend Context;
        explicit BuiltIn();

        using Primitive = type::primitive::Primitive;
        std::vector<Primitive*> primitive_vector;
        type::primitive::VariantClass *variant_class = nullptr;
        type::Error *error_symbol = nullptr;
        type::Class *object_class = nullptr;
        type::Class *string_class = nullptr;
        static BuiltIn builtin;
    public:
        type::VariantClass *getVariantClass()const;
        Primitive *getPrimitive(vm::Data kind)const;
        type::Class *getObjectClass()const;
        type::Class *getStringClass()const;
        type::Error *getErrorPrototype()const;
    };

    class ConversionRules{
        friend Context;
    public:
        using Ptr = type::Prototype*;
        using Key = std::pair<Ptr,Ptr>;
        using Value = Ptr;
        using Rule = std::pair<Key,Value>;
    private:
        explicit ConversionRules(BuiltIn* builtIn);
        std::map<Key,Value> promotion_rules;
        std::set<Key> explicit_cast_rules;
        std::set<Key> implicit_cast_rules;//implicit narrowing or widening
    public:
        bool isExplicitCastRuleExist(Ptr src, Ptr dst)const;
        bool isImplicitCastRuleExist(Ptr src, Ptr dst)const;
        std::optional<Rule> getImplicitPromotionRule(Ptr src,Ptr dst)const;
        void insertCastAST(Value dst,ast::expr::Expression **expression);
    };

    class Context{
        type::Module *global = nullptr;
        type::UserFunction *entrance = nullptr;
        BuiltIn builtin;
        ConversionRules conversion_rules;
    public:
        Dependencies<type::Domain*> byteLengthDependencies;
        Context(const Context&)=delete;
        Context();
        type::Module *getGlobal();
        type::UserFunction *getEntrance();
        void setEntrance(type::UserFunction *function);

        const BuiltIn& getBuiltIn();
        ConversionRules& getConversionRules();
    };

}


#endif //EVOBASIC2_CONTEXT_H
