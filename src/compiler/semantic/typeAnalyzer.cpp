//
// Created by yorkin on 11/24/21.
//

#include "typeAnalyzer.h"
#include "semantic.h"
#include "logger.h"

using namespace std;
using namespace evoBasic::type;
using namespace evoBasic::ast;
using namespace evoBasic::ast::expr;
namespace evoBasic{


    std::any TypeAnalyzer::visitGlobal(ast::Global **global_node, TypeAnalyzerArgs args) {
        args.domain = args.context->getGlobal();

        auto iter = (**global_node).member;
        while(iter){
            visitMember(&iter,args);
            iter = iter->next_sibling;
        }
        return {};
    }

    std::any TypeAnalyzer::visitModule(ast::Module **module_node, TypeAnalyzerArgs args) {
        args.domain = (**module_node).module_symbol;

        auto iter = (**module_node).member;
        while(iter){
            visitMember(&iter,args);
            iter = iter->next_sibling;
        }
        return {};
    }

    std::any TypeAnalyzer::visitClass(ast::Class **class_node, TypeAnalyzerArgs args) {
        args.domain = (**class_node).class_symbol;

        auto iter = (**class_node).member;
        while(iter){
            visitMember(&iter,args);
            iter = iter->next_sibling;
        }
        return {};
    }

    std::any TypeAnalyzer::visitFunction(ast::Function **function_node, TypeAnalyzerArgs args) {
        auto function = (**function_node).function_symbol;
        args.domain = args.user_function = function;

        auto iter = (**function_node).statement;
        while(iter){
            visitStatement(&iter,args);
            iter = iter->next_sibling;
        }
        return {};
    }

    std::any TypeAnalyzer::visitLet(ast::stmt::Let **let_node, TypeAnalyzerArgs args) {
        auto iter = (**let_node).variable;
        while(iter){
            auto name = getID(iter->name);
            if(is_name_valid(name,iter->location,args.domain)){
                type::Prototype *result_prototype = nullptr;
                if(iter->initial != nullptr && iter->annotation != nullptr){
                    auto init_type = any_cast<ExpressionType*>(visitExpression(&iter->initial,args));
                    if(init_type->value_kind == ExpressionType::error)continue;
                    if(iter->annotation){
                        auto anno_prototype = any_cast<Prototype*>(visitAnnotation(&iter->annotation,args));
                        if(!init_type->prototype->equal(anno_prototype)){
                            if(args.context->getConversionRules().isImplicitCastRuleExist(init_type->prototype,anno_prototype)){
                                Logger::warning(iter->initial->location,format()<<"implicit conversion from '"<<init_type->prototype->getName()
                                                                               <<"' to '"<<anno_prototype->getName()<<"'");
                                args.context->getConversionRules().insertCastAST(anno_prototype,&(iter->initial));
                            }
                            else {
                                Logger::error(iter->initial->location, format() << "initialize expression type '"
                                                                               << init_type->prototype->getName()
                                                                               << "' is not equivalent to variable type '"
                                                                               << anno_prototype->getName() << "'");
                                continue;
                            }
                        }
                        result_prototype = anno_prototype;
                    }
                    else{
                        result_prototype = init_type->prototype;
                    }
                }
                else if(iter->initial != nullptr){
                    auto init_type = any_cast<ExpressionType*>(visitExpression(&iter->initial,args));
                    result_prototype = init_type->prototype;
                }
                else if(iter->annotation != nullptr){
                    result_prototype = any_cast<Prototype*>(visitAnnotation(&iter->annotation,args));
                }
                else{
                    Logger::error(iter->location,"need initial expression or type mark");
                    continue;
                }

                switch (result_prototype->getKind()) {
                    case type::SymbolKind::Record:
                    case type::SymbolKind::Array:
                        args.context->byteLengthDependencies.addDependent(args.user_function,result_prototype->as<Domain*>());
                }

                auto field = new type::Variable;
                field->setName(name);
                field->setPrototype(result_prototype);
                args.domain->add(field);
            }

            iter = iter->next_sibling;
        }
        return {};
    }

    std::any TypeAnalyzer::visitVariable(ast::Variable **var_node, TypeAnalyzerArgs args) {

    }

    std::any TypeAnalyzer::visitSelect(ast::stmt::Select **select_node, TypeAnalyzerArgs args) {
        auto condition_type = any_cast<ExpressionType*>(visitExpression(&(**select_node).condition,args));
        auto iter = (**select_node).case_;
        while(iter){
            if(iter->condition){
                auto case_type = any_cast<ExpressionType*>(visitExpression(&iter->condition,args));
                if(!condition_type->prototype->equal(case_type->prototype)){
                    Logger::error(iter->location,format()<<"case condition type '"
                                                      <<case_type->prototype->getName()<<"' is not equivalent to select expression type '"
                                                      <<condition_type->prototype->getName()<<"'");
                }
            }
            visitStatementList(&iter->statement,args);
        }

        return {};
    }

    std::any TypeAnalyzer::visitLoop(ast::stmt::Loop **loop_node, TypeAnalyzerArgs args) {
        auto condition_type = any_cast<ExpressionType*>(visitExpression(&(**loop_node).condition,args));
        if(!condition_type->prototype->equal(args.context->getBuiltIn().getPrimitive(vm::Data::boolean))){
            Logger::error((**loop_node).condition->location,"expression type of loop condition must be Boolean");
        }
        visitStatementList(&(**loop_node).statement,args);
        return {};
    }

    std::any TypeAnalyzer::visitIf(ast::stmt::If **ifstmt_node, TypeAnalyzerArgs args) {
        auto iter = (**ifstmt_node).case_;
        while(iter){
            if(iter->condition){
                auto case_type = any_cast<ExpressionType*>(visitExpression(&iter->condition,args));
                if(!case_type->prototype->equal(args.context->getBuiltIn().getPrimitive(vm::Data::boolean))){
                    Logger::error(iter->location,format()<<"expression type of if condition must be boolean but here is '"
                                                      <<case_type->prototype->getName()<<"'");
                }
            }
            visitStatementList(&iter->statement,args);
        }
        return {};
    }

    std::any TypeAnalyzer::visitFor(ast::stmt::For **forstmt_node, TypeAnalyzerArgs args) {
        auto iterator_type = any_cast<ExpressionType*>(visitExpression(&(**forstmt_node).iterator,args));
        if(iterator_type->value_kind == ExpressionType::error){
            Logger::error((**forstmt_node).iterator->location,"iterator not found");
        }
        else if(iterator_type->value_kind != ExpressionType::lvalue){
            Logger::error((**forstmt_node).iterator->location,"iterator must be a lvalue");
        }
        else{
            auto begin_type = any_cast<ExpressionType*>(visitExpression(&(**forstmt_node).begin,args)),
                    end_type = any_cast<ExpressionType*>(visitExpression(&(**forstmt_node).end,args));

            if(!begin_type->prototype->equal(iterator_type->prototype)){
                Logger::error((**forstmt_node).begin->location,format()<<"Begin expression type '"
                                                                    <<begin_type->prototype->getName()<<"' is not equivalent to iterator type '"
                                                                    <<iterator_type->prototype->getName()<<"'");
            }
            if(!end_type->prototype->equal(iterator_type->prototype)){
                Logger::error((**forstmt_node).begin->location,format()<<"End expression type '"
                                                                    <<end_type->prototype->getName()<< "' is not equivalent to iterator type '"
                                                                    <<iterator_type->prototype->getName()<<"'");
            }
            if((**forstmt_node).step){
                auto step_type = any_cast<ExpressionType*>(visitExpression(&(**forstmt_node).step,args));
                if(!step_type->prototype->equal(iterator_type->prototype)){
                    Logger::error((**forstmt_node).begin->location,format()<<"Step expression type '"
                                                                        <<step_type->prototype->getName()<<"' is not equivalent to iterator type '"
                                                                        <<iterator_type->prototype->getName()<<"'");
                }
            }
        }

        visitStatementList(&(**forstmt_node).statement,args);
        return {};
    }

    std::any TypeAnalyzer::visitReturn(ast::stmt::Return **ret_node, TypeAnalyzerArgs args) {
        auto type = any_cast<ExpressionType*>(visitExpression(&(**ret_node).expr,args));
        if(!type->prototype->equal(args.user_function->getRetSignature())){
            Logger::error((**ret_node).location,format()<<"Return type '"
                                                     <<type->prototype->getName()<<"' is not equivalent to Function signature '"
                                                     <<args.user_function->getRetSignature()->getName()<<"'");
        }
        return {};
    }

    std::any TypeAnalyzer::visitExprStmt(ast::stmt::ExprStmt **expr_stmt_node, TypeAnalyzerArgs args) {
        return visitExpression(&(**expr_stmt_node).expr,args);
    }

    void TypeAnalyzer::visitStatementList(ast::stmt::Statement **stmt_list, TypeAnalyzerArgs args) {
        args.domain = new type::TemporaryDomain(args.domain,args.user_function);
        auto iter = (*stmt_list);
        while(iter){
            visitStatement(&iter,args);
            iter = iter->next_sibling;
        }
    }

    std::any TypeAnalyzer::visitBinary(ast::expr::Binary **binary_node, TypeAnalyzerArgs args) {
        auto logic_node = *binary_node;
        auto lhs_type = any_cast<ExpressionType*>(visitExpression(&logic_node->lhs,args));
        if(logic_node->op == ast::expr::Binary::Dot){
            args.dot_expression_context = lhs_type->prototype;
        }
        auto rhs_type = any_cast<ExpressionType*>(visitExpression(&logic_node->rhs,args));


        if(lhs_type->value_kind == ExpressionType::error || rhs_type->value_kind == ExpressionType::error)
            return logic_node->type = ExpressionType::Error;

        auto is_binary_op_vaild = [&](ExpressionType *lhs_type, ExpressionType *rhs_type,
                                      ast::expr::Expression **lhs, ast::expr::Expression **rhs)->bool{
            if(!rhs_type->prototype->equal(lhs_type->prototype)){
                auto result = args.context->getConversionRules().getImplicitPromotionRule(lhs_type->prototype,rhs_type->prototype);
                if(result.has_value()){
                    auto rule = result.value();
                    auto result_type = rule.second;
                    if(!lhs_type->prototype->equal(result_type)) {
                        Logger::warning((*lhs)->location,format()<<"implicit conversion from '"<<lhs_type->prototype->getName()
                                                                 <<"' to '"<<result_type->getName()<<"'");
                        args.context->getConversionRules().insertCastAST(result_type, lhs);

                    }
                    if(!rhs_type->prototype->equal(result_type)) {
                        Logger::warning((*rhs)->location,format()<<"implicit conversion from '"<<rhs_type->prototype->getName()
                                                                 <<"' to '"<<result_type->getName()<<"'");
                        args.context->getConversionRules().insertCastAST(result_type, lhs);
                    }
                    return true;
                }
                else{
                    Logger::error(logic_node->location,format()<< "invalid operands to binary expression.lhs type is '"
                                                               << lhs_type->prototype->getName()
                                                               << "' and rhs type is'"<<rhs_type->prototype->getName());
                    return false;
                }
            }
            return true;
        };

        auto boolean_prototype = args.context->getBuiltIn().getPrimitive(vm::Data::i8);

        auto is_boolean = [&](ExpressionType *type,Location *location)->bool{
            if(!type->prototype->equal(boolean_prototype)){
                Logger::error(location,"expression type must be Boolean");
                return false;
            }
            return true;
        };


        switch (logic_node->op) {
            case ast::expr::Binary::And:
            case ast::expr::Binary::Or:
            case ast::expr::Binary::Xor:
                if(!is_boolean(lhs_type,logic_node->lhs->location) || !is_boolean(rhs_type,logic_node->rhs->location)){
                    return ExpressionType::Error;
                }
                return logic_node->type = new ExpressionType(boolean_prototype,ExpressionType::rvalue);
            case ast::expr::Binary::Not:
                if(!is_boolean(rhs_type,logic_node->rhs->location)){
                    return ExpressionType::Error;
                }
                return logic_node->type = new ExpressionType(boolean_prototype,ExpressionType::rvalue);

            case ast::expr::Binary::EQ:
            case ast::expr::Binary::NE:
            case ast::expr::Binary::GE:
            case ast::expr::Binary::LE:
            case ast::expr::Binary::GT:
            case ast::expr::Binary::LT:
                if(!is_binary_op_vaild(lhs_type, rhs_type, &(logic_node->lhs), &(logic_node->rhs)))return ExpressionType::Error;
                return logic_node->type = new ExpressionType(boolean_prototype,ExpressionType::rvalue);

            case ast::expr::Binary::ADD:
            case ast::expr::Binary::MINUS:
            case ast::expr::Binary::MUL:
            case ast::expr::Binary::DIV:
            case ast::expr::Binary::FDIV:
                if(!is_binary_op_vaild(lhs_type, rhs_type, &(logic_node->lhs), &(logic_node->rhs)))return ExpressionType::Error;
                return logic_node->type = new ExpressionType(lhs_type->prototype,ExpressionType::rvalue);

            case ast::expr::Binary::ASSIGN:
                if(!is_binary_op_vaild(lhs_type, rhs_type, &(logic_node->lhs), &(logic_node->rhs)))return ExpressionType::Error;
                if(lhs_type->value_kind != ExpressionType::lvalue){
                    Logger::error(logic_node->lhs->location,"lvalue required as left operand of assignment");
                    return ExpressionType::Error;
                }
                return logic_node->type = new ExpressionType(lhs_type->prototype,ExpressionType::lvalue);

            case ast::expr::Binary::Index:
                switch(lhs_type->prototype->getKind()) {
                    case SymbolKind::Class:
                        //TODO operator[] override
                        break;
                    case SymbolKind::Array:
                        return new ExpressionType(lhs_type->prototype->as<Array*>()->getElementPrototype(),ExpressionType::lvalue);
                        break;
                    default:
                        Logger::error(logic_node->location,"invalid expression");
                        return ExpressionType::Error;
                }

            case ast::expr::Binary::Dot:
                switch (lhs_type->prototype->getKind()) {
                    case SymbolKind::Record:
                    case SymbolKind::Array:
                        if(logic_node->lhs->expression_kind == Expression::callee_){
                            auto tmp = new type::Variable;
                            tmp->setPrototype(lhs_type->prototype);
                            args.user_function->addMemoryLayout(tmp);
                            switch (lhs_type->prototype->getKind()) {
                                case SymbolKind::Record:
                                case SymbolKind::Array:
                                    args.context->byteLengthDependencies.addDependent(args.user_function,lhs_type->prototype->as<Domain*>());
                                    break;
                            }
                            logic_node->temp_address = tmp;
                        }
                        return new ExpressionType(rhs_type->prototype,lhs_type->value_kind);
                    case SymbolKind::Class:
                        return new ExpressionType(rhs_type->prototype,rhs_type->value_kind);
                    default:
                        return rhs_type;
                }
        }
    }

    std::any TypeAnalyzer::visitUnary(ast::expr::Unary **unit_node, TypeAnalyzerArgs args) {
        return (**unit_node).type = any_cast<ExpressionType*>(visitExpression(&(**unit_node).terminal,args));
    }

    std::any TypeAnalyzer::visitCallee(ast::expr::Callee **callee_node_ptr, TypeAnalyzerArgs args) {
        auto callee_node = *callee_node_ptr;
        auto target_type = any_cast<ExpressionType*>(visitID(&callee_node->name,args));
        args.dot_expression_context = target_type->prototype;

        if(target_type->value_kind == ExpressionType::error){
            return target_type;
        }

        auto func = target_type->prototype->as<type::Function*>();

        if(!func){
            Logger::error(callee_node->name->location,format()<<"'"<<target_type->prototype->getName()<<"' is not a callable target");
            return ExpressionType::Error;
        }

        auto current_arg = 0;
        auto args_count = 0;
        auto params_count = func->getArgsSignature().size();

        if(func->getFunctionFlag() != type::Function::Flag::Static){
            params_count--;
            current_arg++;
        }

        auto arg = callee_node->argument;
        while(arg){
            args.checking_args_index = current_arg;
            arg = arg->next_sibling;
            current_arg++;
            args_count++;
        }


        if(args_count > params_count){
            Logger::error(callee_node->location,format()<<"too many arguments to function call, expected "
                                                        <<params_count<<", have "<<args_count);
        }
        else if(args_count < params_count){
            Logger::error(callee_node->location,format()<<"too few arguments to function call, expected "
                                                        <<params_count<<", have "<<args_count);
        }

        auto ret = func->getRetSignature();
        return callee_node->type = new ExpressionType(ret->as<type::Prototype*>(),ExpressionType::rvalue);
    }

    std::any TypeAnalyzer::visitArg(ast::expr::Callee::Argument **argument_node, TypeAnalyzerArgs args) {
        auto function = args.dot_expression_context->as<type::Function*>();
        if(args.checking_args_index >= function->getArgsSignature().size())return {};
        auto param =  function->getArgsSignature()[args.checking_args_index];

        args.dot_expression_context = nullptr;
        auto arg_type = any_cast<ExpressionType*>(visitExpression(&(**argument_node).expr, args));
        if(arg_type->value_kind == ExpressionType::error)return {};

        auto arg_prototype = arg_type->prototype;
        auto param_prototype = param->getPrototype();

        auto report_type_error = [&](){
            Logger::error((**argument_node).location, format() << "parameter type is '" << param_prototype->getName()
                                                            << "' but the argument type is '" << arg_prototype->getName() << "'");
        };

        auto try_implicit_conversion = [&]()->bool{
            if(args.context->getConversionRules().isImplicitCastRuleExist(arg_prototype,param_prototype)){
                Logger::warning((**argument_node).location, format() << "implicit conversion from '" << arg_prototype->getName()
                                                                  << "' to '" << param_prototype->getName() << "'");
                args.context->getConversionRules().insertCastAST(param_prototype,&(**argument_node).expr);
                return true;
            }
            return false;
        };

        /*
         *   Param\Arg          ByVal                  ByRef            Undefined
         *   ByVal      Yes,allow implicit conversion  Error      Yes,allow implicit conversion
         *   ByRef      store value to tmp address,                   Error when arg
         *              allow implicit conversion      Yes             is not lvalue
         */

        if(param->isByval()){
            switch ((**argument_node).pass_kind) {
                case ast::expr::Callee::Argument::undefined:
                case ast::expr::Callee::Argument::byval:
                    if(!param->getPrototype()->equal(arg_type->prototype)){
                        if(!try_implicit_conversion())report_type_error();
                    }
                    break;
                case ast::expr::Callee::Argument::byref:
                    Logger::error((**argument_node).location, "require ByVal but declared ByRef");
                    break;
            }
        }
        else{
            switch ((**argument_node).pass_kind) {
                case ast::expr::Callee::Argument::byval:
                    if(!param->getPrototype()->equal(arg_type->prototype)){
                        if(!try_implicit_conversion())report_type_error();
                    }
                    (**argument_node).temp_address = new type::Variable;
                    (**argument_node).temp_address->setPrototype(param->getPrototype());
                    switch (param->getPrototype()->getKind()) {
                        case SymbolKind::Record:
                        case SymbolKind::Array:
                            args.context->byteLengthDependencies.addDependent(args.user_function,param->getPrototype()->as<Domain*>());
                            break;
                    }
                    args.user_function->addMemoryLayout((**argument_node).temp_address);
                    break;
                case ast::expr::Callee::Argument::byref:
                    if(!param->getPrototype()->equal(arg_type->prototype)){
                        report_type_error();
                    }
                    break;
                case ast::expr::Callee::Argument::undefined:
                    if(arg_type->value_kind != ExpressionType::lvalue){
                        Logger::error((**argument_node).location, format() << "can not pass a temporary value ByRef."
                                                                        << "Change parameter to Byval or explicit declare 'ByVal' here.\n"
                                                                        << "Syntax: exampleFunction(Byval <Expression>) ");
                    }
                    else if(!param->getPrototype()->equal(arg_type->prototype)){
                        report_type_error();
                    }
                    break;
            }
        }
        (**argument_node).expr->type = arg_type;
        return {};
    }

    std::any TypeAnalyzer::visitID(ast::expr::ID **id_node, TypeAnalyzerArgs args) {
        auto name = getID(*id_node);
        Symbol *target;
        if(!args.dot_expression_context){
            target = args.domain->lookUp(name);
            if(!target){
                Logger::error((**id_node).location,"object not find");
                return ExpressionType::Error;
            }
        }
        else{
            auto domain = args.dot_expression_context->as<Domain*>();
            if(!(domain && (target = domain->find(name)))){
                Logger::error((**id_node).location,"object not find");
                return ExpressionType::Error;
            }
        }

        switch (target->getKind()) {
            case type::SymbolKind::Variable:
            case type::SymbolKind::Argument:
                return new ExpressionType(target->as<type::Variable*>()->getPrototype(),ExpressionType::lvalue);
            default:
                return new ExpressionType(target->as<Prototype*>(),ExpressionType::path);
        }
    }




    std::any TypeAnalyzer::visitDigit(ast::expr::Digit **digit_node, TypeAnalyzerArgs args) {
        auto prototype = args.context->getBuiltIn().getPrimitive(vm::Data::i32)->as<type::Class*>();
        return (**digit_node).type = new ExpressionType(prototype,ExpressionType::rvalue);
    }

    std::any TypeAnalyzer::visitDecimal(ast::expr::Decimal **decimal_node, TypeAnalyzerArgs args) {
        auto prototype = args.context->getBuiltIn().getPrimitive(vm::Data::i32)->as<type::Class*>();
        return (**decimal_node).type = new ExpressionType(prototype,ExpressionType::rvalue);
    }

    std::any TypeAnalyzer::visitBoolean(ast::expr::Boolean **bl_node, TypeAnalyzerArgs args) {
        auto prototype = args.context->getBuiltIn().getPrimitive(vm::Data::i32)->as<type::Class*>();
        return (**bl_node).type = new ExpressionType(prototype,ExpressionType::rvalue);
    }

    std::any TypeAnalyzer::visitChar(ast::expr::Char **ch_node, TypeAnalyzerArgs args) {
        auto prototype = args.context->getBuiltIn().getPrimitive(vm::Data::i32)->as<type::Class*>();
        return (**ch_node).type = new ExpressionType(prototype,ExpressionType::rvalue);
    }

    std::any TypeAnalyzer::visitString(ast::expr::String **str_node, TypeAnalyzerArgs args) {
        auto cls = args.context->getBuiltIn().getStringClass();
        return (**str_node).type = new ExpressionType(cls,ExpressionType::rvalue);
    }

    std::any TypeAnalyzer::visitParentheses(ast::expr::Parentheses **parentheses_node, TypeAnalyzerArgs args) {
        args.dot_expression_context = args.domain;
        return visitExpression(&(**parentheses_node).expr,args);
    }

}
