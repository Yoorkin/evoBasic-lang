//
// Created by yorkin on 11/1/21.
//

#ifndef EVOBASIC2_CONTEXT_H
#define EVOBASIC2_CONTEXT_H
#include <memory>
#include <set>
#include <functional>
#include <optional>

#include <analyzer/type.h>
#include <utils/data.h>
#include <utils/dependencies.h>
#include <loader/loader.h>
#include <parser/parseTree.h>

namespace evoBasic{

    namespace ast{
        struct Expression;
    }

    using SymbolPtr = type::Symbol*;
    using PromotionRuleFunction = std::function<std::shared_ptr<type::Prototype>(parseTree::Node *node)>;
    using BinaryOpSignature = std::pair<SymbolPtr,SymbolPtr>;
    class CompileTimeContext;

    class BuiltIn{
        friend CompileTimeContext;
        explicit BuiltIn();

        using Primitive = type::primitive::Primitive;
        std::vector<Primitive*> primitive_vector;
        std::vector<unicode::Utf8String> operators_name;
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
        unicode::Utf8String getOperatorName()const;
    };

    class ConversionRules{
        friend CompileTimeContext;
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
        std::optional<Rule> getImplicitPromotionRule(Ptr lhs, Ptr rhs)const;
        void insertCastAST(Value dst, ast::Expression **expression);
    };

    class CompileTimeContext{
        type::Module *global = nullptr;
        type::Function *entrance = nullptr;
        BuiltIn builtin;
        ConversionRules conversion_rules;
        Loader *loader = nullptr;
    public:
        Dependencies<type::Domain*> byteLengthDependencies;
        Dependencies<type::Class*> inheritDependencies;

        CompileTimeContext(const CompileTimeContext&)=delete;
        CompileTimeContext();
        type::Symbol *findSymbol(std::list<unicode::Utf8String> full_name);
        type::Module *getGlobal();
        type::Function *getEntrance();
        void setEntrance(type::Function *function);

        Loader *getLoader();
        const BuiltIn& getBuiltIn();
        ConversionRules& getConversionRules();
    };

}


#endif //EVOBASIC2_CONTEXT_H
