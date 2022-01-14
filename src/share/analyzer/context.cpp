//
// Created by yorkin on 11/1/21.
//

#include "context.h"
#include "semantic.h"

#include <parser/parser.h>
#include <utils/nullSafe.h>
#include <loader/bytecode.h>

using namespace std;
using namespace evoBasic::type;
using namespace evoBasic::type::primitive;
namespace evoBasic{


    BuiltIn::BuiltIn() {
        error_symbol = new Error;
        error_symbol->setName("< Error >");

        //void,boolean,i8,i16,i32,i64,f32,f64,u8,u16,u32,u64
        primitive_vector = {
            nullptr,
            new Primitive("boolean", vm::Data(vm::Data::boolean)),
            new Primitive("byte", vm::Data(vm::Data::i8)),
            new Primitive("short", vm::Data(vm::Data::i16)),
            new Primitive("integer", vm::Data(vm::Data::i32)),
            new Primitive("long", vm::Data(vm::Data::i64)),
            new Primitive("single", vm::Data(vm::Data::f32)),
            new Primitive("double", vm::Data(vm::Data::f64)),
            new Primitive("u8", vm::Data(vm::Data::u8)),
            new Primitive("u16", vm::Data(vm::Data::u16)),
            new Primitive("u32", vm::Data(vm::Data::u32)),
            new Primitive("u64", vm::Data(vm::Data::u64)),
        };

        variant_class = new VariantClass();
        variant_class->setName("variant");

        object_class = new type::Class;
        object_class->setName("Object");

        string_class = new type::Class;
        string_class->setName("String");
        string_class->setExtend(object_class);

        operators_name = {
            "Get","Compare","Times","Div","Plus","Minus","Neg","Pos","Invoke"
        };
    }

    VariantClass *BuiltIn::getVariantClass() const {
        return variant_class;
    }

    Error *BuiltIn::getErrorPrototype() const {
        return error_symbol;
    }

    BuiltIn::Primitive *BuiltIn::getPrimitive(vm::Data data) const {
        return primitive_vector[(int)data.getValue()];
    }

    type::Class *BuiltIn::getObjectClass() const {
        return object_class;
    }

    type::Class *BuiltIn::getStringClass() const {
        return string_class;
    }

    std::string BuiltIn::getOperatorName() const {
        return std::string();
    }


    ConversionRules::ConversionRules(BuiltIn* builtIn) {
        enum Types {bin=0,i08,i16,i32,i64,f32,f64,_n_};
        auto& in = *builtIn;
        vector<Prototype*> enumToPrototype = {
                in.getPrimitive(vm::Data::boolean),
                in.getPrimitive(vm::Data::i8),
                in.getPrimitive(vm::Data::i16),
                in.getPrimitive(vm::Data::i32),
                in.getPrimitive(vm::Data::i64),
                in.getPrimitive(vm::Data::f32),
                in.getPrimitive(vm::Data::f64)
        };

        //explicit conversion
        for(int l=0;l<enumToPrototype.size();l++){
            for(int r=0;r<enumToPrototype.size();r++){
                const auto& l_type = enumToPrototype[l];
                const auto& r_type = enumToPrototype[r];
                explicit_cast_rules.insert({l_type,r_type});
            }
        }

        //promotion in binary operation
        vector<vector<Types>> promotion_table = {
                /*        bin i08 i16 132 i64 f32 f64 */
                /* bin */{bin,i32,i32,i32,i64,f64,f64},
                /* i08 */{i32,i32,i32,i32,i64,f64,f64},
                /* i16 */{i32,i32,i32,i32,i64,f64,f64},
                /* i32 */{i32,i32,i32,i32,i64,f64,f64},
                /* i64 */{i64,i64,i64,i64,i64,f64,f64},
                /* f32 */{f64,f64,f64,f64,f64,f64,f64},
                /* f64 */{f64,f64,f64,f64,f64,f64,f64}
        };
        
        for(int l=0;l<enumToPrototype.size();l++){
            for(int r=0;r<enumToPrototype.size();r++){
                const auto& l_type = enumToPrototype[l];
                const auto& r_type = enumToPrototype[r];
                Types idx = promotion_table[l][r];
                auto result_prototype = enumToPrototype[(int)idx];
                promotion_rules.insert({{l_type,r_type},result_prototype});
            }
        }

        //implicit conversion for narrowing or widening
        enum AcceptState {Yes,No_};
        vector<vector<AcceptState>> implicit_cast_table = {
                /* dst -> bin i08 i16 132 i64 f32 f64 */
                /* src */
                /* bin */{No_,No_,No_,No_,No_,No_,No_},
                /* i08 */{No_,No_,Yes,Yes,Yes,Yes,Yes},
                /* i16 */{No_,Yes,No_,Yes,Yes,Yes,Yes},
                /* i32 */{No_,Yes,Yes,No_,Yes,Yes,Yes},
                /* i64 */{No_,Yes,Yes,Yes,No_,Yes,Yes},
                /* f32 */{No_,Yes,Yes,Yes,Yes,No_,Yes},
                /* f64 */{No_,Yes,Yes,Yes,Yes,Yes,No_}
        };

        for(int l=0;l<enumToPrototype.size();l++){
            for(int r=0;r<enumToPrototype.size();r++){
                const auto& l_type = enumToPrototype[l];
                const auto& r_type = enumToPrototype[r];
                AcceptState state = implicit_cast_table[l][r];
                if(state == No_)continue;
                implicit_cast_rules.insert({l_type,r_type});
            }
        }
    }

    bool
    ConversionRules::isExplicitCastRuleExist(ConversionRules::Ptr src, ConversionRules::Ptr dst) const {
        return explicit_cast_rules.contains({src,dst});
    }

    bool
    ConversionRules::isImplicitCastRuleExist(ConversionRules::Ptr src, ConversionRules::Ptr dst) const {
        return implicit_cast_rules.contains({src,dst});
    }

    optional<ConversionRules::Rule>
    ConversionRules::getImplicitPromotionRule(ConversionRules::Ptr lhs, ConversionRules::Ptr rhs) const {
        auto target = promotion_rules.find({lhs, rhs});
        if(target == promotion_rules.end())return {};
        else return *target;
    }

    void ConversionRules::insertCastAST(ConversionRules::Value dst, ast::Expression **expression) {
        switch (dst->getKind()) {
            case SymbolKind::Class:
                break;
            case SymbolKind::Enum:
                break;
            case SymbolKind::EnumMember:
                break;
            case SymbolKind::Record:
                break;
            case SymbolKind::Function:
                break;
            case SymbolKind::Module:
                break;
            case SymbolKind::Primitive:
                *expression = new ast::Cast(*expression,dst);
                break;
            case SymbolKind::TmpDomain:
                break;
            case SymbolKind::Variable:
                break;
            case SymbolKind::Error:
                break;
            case SymbolKind::Interface:
                break;
            case SymbolKind::Parameter:
                break;
            case SymbolKind::Array:
                break;
        }
        (*expression)->type = new ExpressionType(dst,ExpressionType::rvalue);
    }


    Function *CompileTimeContext::getEntrance() {
        NotNull(entrance);
        return entrance;
    }

    void CompileTimeContext::setEntrance(Function *function) {
        NotNull(function);
        entrance = function;
    }

    Module *CompileTimeContext::getGlobal() {
        NotNull(global);
        return global;
    }

    CompileTimeContext::CompileTimeContext() : conversion_rules(&builtin) {
        global = new type::Module;
        global->setName("global");
        auto& in = getBuiltIn();
        global->add(in.getObjectClass());
        global->add(in.getStringClass());
        global->add(in.getPrimitive(vm::Data::boolean));
        global->add(in.getPrimitive(vm::Data::i8));
        global->add(in.getPrimitive(vm::Data::i16));
        global->add(in.getPrimitive(vm::Data::i32));
        global->add(in.getPrimitive(vm::Data::i64));
        global->add(in.getPrimitive(vm::Data::f32));
        global->add(in.getPrimitive(vm::Data::f64));
    }


    const BuiltIn &CompileTimeContext::getBuiltIn() {
        return builtin;
    }

    ConversionRules &CompileTimeContext::getConversionRules() {
        return conversion_rules;
    }

    type::Symbol *CompileTimeContext::findSymbol(list<string> full_name) {
        type::Symbol *current = getGlobal();
        if(full_name.front() != current->getName())
            PANICMSG(full_name.front());
        full_name.pop_front();

        for(auto &path : full_name){
            auto domain = current->as<Domain*>();
            current = domain->find(path);
        }

        return current;
    }

    Loader *CompileTimeContext::getLoader() {
        return loader;
    }

}
