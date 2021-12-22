//
// Created by yorkin on 12/13/21.
//

#include "ilGen.h"
#include<variant>

namespace evoBasic{
    using namespace il;
    using namespace std;

    DataType mapILType(type::Prototype *type){
        switch(type->getKind()){
            case type::SymbolKind::Class:       return il::ref;
            case type::SymbolKind::Enum:        return il::u32;
            case type::SymbolKind::Record:      return il::record;
            case type::SymbolKind::Function:    return il::delegate;
            case type::SymbolKind::Array:       return il::array;
            case type::SymbolKind::Primitive:{
                auto primitive = type->as<type::Primitive*>();
                switch(primitive->getDataKind().getValue()){
                    case vm::Data::i8:      return il::i8;
                    case vm::Data::i16:     return il::i16;
                    case vm::Data::i32:     return il::i32;
                    case vm::Data::i64:     return il::i64;
                    case vm::Data::u8:      return il::u8;
                    case vm::Data::u16:     return il::u16;
                    case vm::Data::u32:     return il::u32;
                    case vm::Data::u64:     return il::u64;
                    case vm::Data::f32:     return il::f32;
                    case vm::Data::f64:     return il::f64;
                    case vm::Data::boolean: return il::boolean;
                    default: PANIC;
                }
            }
            default: PANIC;
        }
    }

#define Visit(RETURN,AST,VAR,...) RETURN ILGen::visit##AST(ast::AST *VAR##_node,##__VA_ARGS__)
    Visit(il::IL*,Global,global){

    }

    Visit(il::Class*,Class,class){

    }

    Visit(il::Module*,Module,module){

    }

    Visit(il::Interface*,Interface,interface){

    }

    Visit(il::Record*,Type,type){

    }

    Visit(il::Enum*,Enum,enum){

    }

    Visit(il::Ftn*,Function,function){

    }

    Visit(il::Ext*,External,external){

    }

    Visit(il::Ctor*,Constructor,ctor){

    }



    Visit(il::Block*,Let,let,               il::Block *current,il::Block *next){

    }
    Visit(il::Block*,Select,select,         il::Block *current,il::Block *next){

    }
    Visit(il::Block*,Loop,loop,             il::Block *current,il::Block *next){

    }
    Visit(il::Block*,If,if,                 il::Block *current,il::Block *next){

    }
    Visit(il::Block*,Case,case,             il::Block *current,il::Block *next){

    }
    Visit(il::Block*,For,for,               il::Block *current,il::Block *next){

    }
    Visit(il::Block*,ExprStmt,expr_stmt,    il::Block *current,il::Block *next){

    }
    Visit(il::Block*,Return,return,         il::Block *current,il::Block *next){

    }
    Visit(il::Block*,Exit,exit,             il::Block *current,il::Block *next){

    }

    Visit(il::Block*,Continue,continue,     il::Block *current,il::Block *next){

    }



    Visit(void,Expression,expression,   il::Block *current){

    }

    Visit(void,Unary,unary,             il::Block *current){

    }

    Visit(void,Binary,binary,           il::Block *current){

    }

    Visit(void,Assign,assign,           il::Block *current){

    }

    Visit(void,Cast,cast,               il::Block *current){

    }

    Visit(void,Parentheses,parentheses, il::Block *current){

    }

    Visit(void,ArrayElement,element,    il::Block *current){

    }

    Visit(void,Delegate,delegate,       il::Block *current){

    }

    Visit(void,Argument,argument,       il::Block *current){

    }

    Visit(void,New,new,                 il::Block *current){

    }

    void ILGen::visitFtnCall(ast::FtnCall *ftn_node, il::Block *current) {

    }

    Visit(void,SFtnCall,sftn,           il::Block *current){

    }


    void ILGen::loadCalleeArguments(ast::Call *call,il::Block *current){
        map<string,ast::Argument*> used_options;

//        // load regular argument by declaration order
//        FOR_EACH(iter,call->argument){
//            if(iter->is_option)used_options.insert({iter->parameter->getName(),iter});
//            else{
//                visitArgument(iter,current);
//            }
//        }

        // load optional argument by declaration order
        for(auto parameter : call->function->getArgsOptions()){
            auto target = used_options.find(parameter->getName());
            // load value declared in invoke,otherwise load default value
            if(target != used_options.end()){
                visitArgument(target->second,current);
            }
            else{
                auto default_exp = parameter->getInitial();
                visitExpression(default_exp,current);
                if(parameter->isByval()){
                    // pass byval
                    switch (default_exp->expression_kind) {
                        case ast::Expression::Assign:{
                            //todo
                            break;
                        }
                        case ast::Expression::Ftn:
                            visitFtnCall((ast::FtnCall*)default_exp,current);
                            break;
                        case ast::Expression::VFtn:
                            visitVFtnCall((ast::VFtnCall*)default_exp,current);
                            break;
                        case ast::Expression::SFtn:
                            visitSFtnCall((ast::SFtnCall*)default_exp,current);
                            break;
                        case ast::Expression::Fld:{
                            auto fld = (ast::Fld*)default_exp;
                            visitFld(fld,current);
                            current->Ldfld(mapILType(fld->variable->getPrototype()),fld->variable->getToken(factory));
                            break;
                        }
                        case ast::Expression::SFld:{
                            auto sfld = (ast::SFld*)default_exp;
                            visitSFld(sfld,current);
                            current->Ldsfld(mapILType(sfld->variable->getPrototype()),sfld->variable->getToken(factory));
                            break;
                        }
                        case ast::Expression::Element:
                        case ast::Expression::Unary:
                        case ast::Expression::Binary:
                        case ast::Expression::New:
                        case ast::Expression::Digit:
                        case ast::Expression::Decimal:
                        case ast::Expression::String:
                        case ast::Expression::Boolean:
                        case ast::Expression::Char:
                        case ast::Expression::Delegate:
                        case ast::Expression::Parentheses:
                        case ast::Expression::Cast:
                            // do nothing
                            break;
                        default: PANIC;
                    }
                }
                else{
                    // pass byref
                    switch (default_exp->expression_kind) {
                        case ast::Expression::Assign:{
                            //todo
                            break;
                        }
                        case ast::Expression::Ftn:
                            visitFtnCall((ast::FtnCall*)default_exp,current);

                            break;
                        case ast::Expression::VFtn:
                            visitVFtnCall((ast::VFtnCall*)default_exp,current);
                            break;
                        case ast::Expression::SFtn:
                            visitSFtnCall((ast::SFtnCall*)default_exp,current);
                            break;
                        case ast::Expression::Fld:{
                            auto fld = (ast::Fld*)default_exp;
                            visitFld(fld,current);
                            current->Ldfld(mapILType(fld->variable->getPrototype()),fld->variable->getToken(factory));
                            break;
                        }
                        case ast::Expression::SFld:{
                            auto sfld = (ast::SFld*)default_exp;
                            visitSFld(sfld,current);
                            current->Ldsfld(mapILType(sfld->variable->getPrototype()),sfld->variable->getToken(factory));
                            break;
                        }
                        case ast::Expression::Element:
                        case ast::Expression::Unary:
                        case ast::Expression::Binary:
                        case ast::Expression::New:
                        case ast::Expression::Digit:
                        case ast::Expression::Decimal:
                        case ast::Expression::String:
                        case ast::Expression::Boolean:
                        case ast::Expression::Char:
                        case ast::Expression::Delegate:
                        case ast::Expression::Parentheses:
                        case ast::Expression::Cast:
                            // do nothing
                            break;
                        default: PANIC;
                    }
                }
            }
            auto target_exp = (target != used_options.end() ? target->second->expr : parameter->getInitial());
            visitExpression(target_exp,current);
        }
    }

    void ILGen::visitVFtnCall(ast::VFtnCall *vftn_node, il::Block *current) {
        visitExpression(vftn_node->ref,current);
        auto token = vftn_node->function->getToken(factory);
        loadCalleeArguments(vftn_node,current);
    }

    Visit(void,ExtCall,ext, il::Block *current){
        auto token = ext_node->function->getToken(factory);
        //current->Ldfld(,token);
    }

    Visit(void,SFld,sfld, il::Block *current){
        auto token = sfld_node->variable->getToken(factory);
        //current->Ldsfld(,token);
    }

    Visit(void,Fld,fld, il::Block *current){
        auto token = fld_node->variable->getToken(factory);
        visitExpression(fld_node->ref,current);
        current->Ldfld(mapILType(fld_node->variable->getPrototype()),token);
    }



    Visit(void,Local,local, il::Block *current){
        auto index = local_node->variable->getLayoutIndex();
        //current->Push(DataType::u16,(data::u16),index);
        current->Ldloc(mapILType(local_node->variable->getPrototype()));
    }

    Visit(void,Arg,arg, il::Block *current){
        auto index = arg_node->variable->getLayoutIndex();
        current->Push(DataType::u16,(data::u16)index);
        current->Ldarg(mapILType(arg_node->variable->getPrototype()));
    }

    Visit(void,Digit,digit, il::Block *current){
        current->Push(DataType::i32,digit_node->value);
    }

    Visit(void,Decimal,decimal, il::Block *current){
        current->Push(DataType::f64,decimal_node->value);
    }

    Visit(void,String,string, il::Block *current){
        current->Ldc(factory->createToken(string_node->value));
        //todo: call ctor for String
    }

    Visit(void,Char,char, il::Block *current){
        current->Push(DataType::character,char_node->value);
    }

    Visit(void,Boolean,boolean, il::Block *current){
        current->Push(DataType::boolean,boolean_node->value);
    }

}
