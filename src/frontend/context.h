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

        using Primivite = type::primitive::Primitive;
        std::vector<std::shared_ptr<Primivite>> primitive_vector;
        std::shared_ptr<type::primitive::VariantClass> variant_class;
        std::shared_ptr<type::Error> error_symbol;
        static BuiltIn builtin;
    public:
        std::shared_ptr<type::primitive::VariantClass> getVariantClass()const;
        std::shared_ptr<Primivite> getPrimitive(vm::Data kind)const;
        std::shared_ptr<type::Error> getErrorPrototype()const;
    };

    class ConversionRules{
        friend Context;
    public:
        using sharedPtr = std::shared_ptr<type::Prototype>;
        using Key = std::pair<sharedPtr,sharedPtr>;
        using Value = sharedPtr;
        using Rule = std::pair<Key,Value>;
    private:
        explicit ConversionRules(BuiltIn* builtIn);
        std::map<Key,Value> promotion_rules;
        std::set<Key> explicit_cast_rules;
        std::set<Key> implicit_cast_rules;//implicit narrowing or widening
    public:
        bool isExplicitCastRuleExist(sharedPtr src, sharedPtr dst)const;
        bool isImplicitCastRuleExist(sharedPtr src,sharedPtr dst)const;
        std::optional<Rule> getImplicitPromotionRule(sharedPtr src,sharedPtr dst)const;
        void insertCastAST(Value dst,ast::expr::Expression **expression);
    };

    class Context{
        std::shared_ptr<type::Module> global;
        std::shared_ptr<type::UserFunction> entrance;
        BuiltIn builtin;
        ConversionRules conversion_rules;
    public:
        Dependencies<std::shared_ptr<type::Symbol>> byteLengthDependencies;
        //Dependencies<std::pair<std::shared_ptr<type::Variable>,ast::expr::Expression*>> typeInferenceDependencies;
        Context(const Context&)=delete;
        Context();
        std::shared_ptr<type::Module> getGlobal();
        std::shared_ptr<type::UserFunction> getEntrance();
        void setEntrance(std::shared_ptr<type::UserFunction> function);

        const BuiltIn& getBuiltIn();
        ConversionRules& getConversionRules();
    };

}


#endif //EVOBASIC2_CONTEXT_H
