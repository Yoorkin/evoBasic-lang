//
// Created by yorkin on 11/1/21.
//

#include "context.h"
#include "parser.h"
#include "nullSafe.h"
#include "bytecode.h"
#include "semantic/semantic.h"
using namespace std;
using namespace evoBasic::type;
using namespace evoBasic::type::primitive;
namespace evoBasic{

    BuiltIn::BuiltIn() {
        error_symbol = make_shared<Error>();
        error_symbol->setName("< Error >");

        //void,boolean,i8,i16,i32,i64,f32,f64,u8,u16,u32,u64
        primitive_vector = {
            nullptr,
            make_shared<Primivite>("boolean",vm::Data(vm::Data::i8)),
            make_shared<Primivite>("byte",vm::Data(vm::Data::i8)),
            make_shared<Primivite>("short",vm::Data(vm::Data::i16)),
            make_shared<Primivite>("integer",vm::Data(vm::Data::i32)),
            make_shared<Primivite>("long",vm::Data(vm::Data::i64)),
            make_shared<Primivite>("single",vm::Data(vm::Data::f32)),
            make_shared<Primivite>("double",vm::Data(vm::Data::f64)),
            make_shared<Primivite>("u8",vm::Data(vm::Data::u8)),
            make_shared<Primivite>("u16",vm::Data(vm::Data::u16)),
            make_shared<Primivite>("u32",vm::Data(vm::Data::u32)),
            make_shared<Primivite>("u64",vm::Data(vm::Data::u64)),
        };

//        variant_class = make_shared<VariantClass>();
//        variant_class->setName("variant");
    }

    std::shared_ptr<VariantClass> BuiltIn::getVariantClass() const {
        return variant_class;
    }

    std::shared_ptr<Error> BuiltIn::getErrorPrototype() const {
        return error_symbol;
    }

    shared_ptr<BuiltIn::Primivite> BuiltIn::getPrimitive(vm::Data data) const {
        return primitive_vector[(int)data.getValue()];
    }


    ConversionRules::ConversionRules(BuiltIn* builtIn) {
        enum Types {bin=0,i08,i16,i32,i64,f32,f64,_n_};
        auto& in = *builtIn;
        vector<shared_ptr<Prototype>> enumToPrototype = {
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
    ConversionRules::isExplicitCastRuleExist(ConversionRules::sharedPtr src, ConversionRules::sharedPtr dst) const {
        return explicit_cast_rules.contains({src,dst});
    }

    bool
    ConversionRules::isImplicitCastRuleExist(ConversionRules::sharedPtr src, ConversionRules::sharedPtr dst) const {
        return implicit_cast_rules.contains({src,dst});
    }

    optional<ConversionRules::Rule>
    ConversionRules::getImplicitPromotionRule(ConversionRules::sharedPtr src, ConversionRules::sharedPtr dst) const {
        auto target = promotion_rules.find({src,dst});
        if(target == promotion_rules.end())return {};
        else return *target;
    }

    void ConversionRules::insertCastAST(ConversionRules::Value dst, ast::expr::Expression **expression) {
        switch (dst->getKind()) {
            case DeclarationEnum::Class:
                break;
            case DeclarationEnum::Enum_:
                break;
            case DeclarationEnum::EnumMember:
                break;
            case DeclarationEnum::Type:
                break;
            case DeclarationEnum::Function:
                break;
            case DeclarationEnum::Module:
                break;
            case DeclarationEnum::Variant:
                break;
            case DeclarationEnum::Primitive:
//                *expression = new ast::expr::Binary(
//                        *expression,
//                        constructAnnotationAST(format()<<"global."<<dst->getName())
//                        );
                break;
            case DeclarationEnum::TmpDomain:
                break;
            case DeclarationEnum::Variable:
                break;
            case DeclarationEnum::Error:
                break;
            case DeclarationEnum::Interface:
                break;
            case DeclarationEnum::Argument:
                break;
            case DeclarationEnum::Array:
                break;
        }
        (*expression)->type = new ExpressionType(dst,ExpressionType::rvalue);
    }


    std::shared_ptr<UserFunction> Context::getEntrance() {
        NotNull(entrance.get());
        return entrance;
    }

    void Context::setEntrance(std::shared_ptr<UserFunction> function) {
        NotNull(function.get());
        entrance = move(function);
    }

    std::shared_ptr<Module> Context::getGlobal() {
        NotNull(global.get());
        return global;
    }

    Context::Context() : conversion_rules(&builtin) {
        global = make_shared<type::Module>();
        global->setName("global");
        auto& in = getBuiltIn();
       // global->add(in.getVariantClass());
        global->add(in.getPrimitive(vm::Data::boolean));
        global->add(in.getPrimitive(vm::Data::i8));
        global->add(in.getPrimitive(vm::Data::i16));
        global->add(in.getPrimitive(vm::Data::i32));
        global->add(in.getPrimitive(vm::Data::i64));
        global->add(in.getPrimitive(vm::Data::f32));
        global->add(in.getPrimitive(vm::Data::f64));
    }


    const BuiltIn &Context::getBuiltIn() {
        return builtin;
    }

    ConversionRules &Context::getConversionRules() {
        return conversion_rules;
    }
}
