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



    void TypeAnalyzer::visitStatementList(ast::stmt::Statement *stmt_list, TypeAnalyzerArgs args) {
        args.domain = new type::TemporaryDomain(args.domain,args.user_function);
        auto iter = stmt_list;
        while(iter){
            visitStatement(iter,args);
            iter = iter->next_sibling;
        }
    }

    std::any TypeAnalyzer::visitArg(ast::expr::Callee::Argument *argument_node, TypeAnalyzerArgs args) {
        auto function = args.dot_expression_context->as<type::Function*>();
        if(args.checking_arg_index >= function->getArgsSignature().size())return {};
        auto param =  function->getArgsSignature()[args.checking_arg_index];

        args.dot_expression_context = nullptr;
        auto arg_type = any_cast<ExpressionType*>(visitExpression((*argument_node).expr, args));
        if(arg_type->value_kind == ExpressionType::error)return {};

        auto arg_prototype = arg_type->prototype;
        auto param_prototype = param->getPrototype();

        auto report_type_error = [&](){
            Logger::error((*argument_node).location, format() << "parameter type is '" << param_prototype->getName()
                                                            << "' but the argument type is '" << arg_prototype->getName() << "'");
        };

        auto try_implicit_conversion = [&]()->bool{
            if(args.context->getConversionRules().isImplicitCastRuleExist(arg_prototype,param_prototype)){
                Logger::warning((*argument_node).location, format() << "implicit conversion from '" << arg_prototype->getName()
                                                                  << "' to '" << param_prototype->getName() << "'");
                args.context->getConversionRules().insertCastAST(param_prototype,&(*argument_node).expr);
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
            switch ((*argument_node).pass_kind) {
                case ast::expr::Callee::Argument::undefined:
                case ast::expr::Callee::Argument::byval:
                    if(!param->getPrototype()->equal(arg_type->prototype)){
                        if(!try_implicit_conversion())report_type_error();
                    }
                    break;
                case ast::expr::Callee::Argument::byref:
                    Logger::error((*argument_node).location, "require ByVal but declared ByRef");
                    break;
            }
        }
        else{
            switch ((*argument_node).pass_kind) {
                case ast::expr::Callee::Argument::byval:
                    if(!param->getPrototype()->equal(arg_type->prototype)){
                        if(!try_implicit_conversion())report_type_error();
                    }
                    (*argument_node).temp_address = new type::Variable;
                    (*argument_node).temp_address->setPrototype(param->getPrototype());
                    switch (param->getPrototype()->getKind()) {
                        case SymbolKind::Record:
                        case SymbolKind::Array:
                            args.context->byteLengthDependencies.addDependent(args.user_function,param->getPrototype()->as<Domain*>());
                            break;
                    }
                    args.user_function->addMemoryLayout((*argument_node).temp_address);
                    break;
                case ast::expr::Callee::Argument::byref:
                    if(!param->getPrototype()->equal(arg_type->prototype)){
                        report_type_error();
                    }
                    break;
                case ast::expr::Callee::Argument::undefined:
                    if(arg_type->value_kind != ExpressionType::lvalue){
                        Logger::error((*argument_node).location, format() << "can not pass a temporary value ByRef."
                                                                        << "Change parameter to Byval or explicit declare 'ByVal' here.\n"
                                                                        << "Syntax: exampleFunction(Byval <Expression>) ");
                    }
                    else if(!param->getPrototype()->equal(arg_type->prototype)){
                        report_type_error();
                    }
                    break;
            }
        }
        (*argument_node).expr->type = arg_type;
        return {};
    }

    std::any TypeAnalyzer::visitMember(ast::Member *member_node, TypeAnalyzerArgs args) {
        switch ((*member_node).member_kind) {
            case ast::Member::function_: return visitFunction((ast::Function*)member_node,args);
            case ast::Member::class_:    return visitClass((ast::Class*)member_node,args);
            case ast::Member::module_:   return visitModule((ast::Module*)member_node,args);
            case ast::Member::type_:     return{}; //return visitType((ast::Type*)member_node,args);
            case ast::Member::enum_:     return{}; //visitEnum((ast::Enum*)member_node,args);
            case ast::Member::dim_:      return{}; //return visitDim((ast::Dim*)member_node,args);
            case ast::Member::external_: return visitExternal((ast::External*)member_node,args);
        }
        PANIC;
    }

    std::any TypeAnalyzer::visitStatement(ast::stmt::Statement *stmt_node, TypeAnalyzerArgs args) {
        switch ((*stmt_node).stmt_flag) {
            case ast::stmt::Statement::let_: return visitLet((ast::stmt::Let*)stmt_node,args);
            case ast::stmt::Statement::loop_:return visitLoop((ast::stmt::Loop*)stmt_node,args);
            case ast::stmt::Statement::if_:  return visitIf((ast::stmt::If*)stmt_node,args);
            case ast::stmt::Statement::for_: return visitFor((ast::stmt::For*)stmt_node,args);
            case ast::stmt::Statement::select_:return visitSelect((ast::stmt::Select*)stmt_node,args);
            case ast::stmt::Statement::return_:return visitReturn((ast::stmt::Return*)stmt_node,args);
            case ast::stmt::Statement::continue_:return visitContinue((ast::stmt::Continue*)stmt_node,args);
            case ast::stmt::Statement::exit_:return visitExit((ast::stmt::Exit*)stmt_node,args);
            case ast::stmt::Statement::expr_:return visitExprStmt((ast::stmt::ExprStmt*)stmt_node,args);
        }
        PANIC;
    }
    
    std::any TypeAnalyzer::visitGlobal(ast::Global *global_node, TypeAnalyzerArgs args) {
        args.domain = args.current_class_or_module = args.context->getGlobal();

        auto iter = (*global_node).member;
        while(iter){
            visitMember(iter,args);
            iter = iter->next_sibling;
        }
        return {};
    }

    std::any TypeAnalyzer::visitModule(ast::Module *module_node, TypeAnalyzerArgs args) {
        args.domain = args.current_class_or_module = (*module_node).module_symbol;

        auto iter = (*module_node).member;
        while(iter){
            visitMember(iter,args);
            iter = iter->next_sibling;
        }
        return {};
    }

    std::any TypeAnalyzer::visitClass(ast::Class *class_node, TypeAnalyzerArgs args) {
        args.domain = args.current_class_or_module = (*class_node).class_symbol;

        auto iter = (*class_node).member;
        while(iter){
            visitMember(iter,args);
            iter = iter->next_sibling;
        }
        return {};
    }

    std::any TypeAnalyzer::visitFunction(ast::Function *function_node, TypeAnalyzerArgs args) {
        auto function = (*function_node).function_symbol;
        args.domain = args.user_function = function;

        auto iter = (*function_node).statement;
        while(iter){
            visitStatement(iter,args);
            iter = iter->next_sibling;
        }
        return {};
    }

    std::any TypeAnalyzer::visitLet(ast::stmt::Let *let_node, TypeAnalyzerArgs args) {
        auto iter = (*let_node).variable;
        while(iter){
            auto name = getID(iter->name);
            if(is_name_valid(name,iter->location,args.domain)){
                type::Prototype *result_prototype = nullptr;
                if(iter->initial != nullptr && iter->annotation != nullptr){
                    auto init_type = any_cast<ExpressionType*>(visitExpression(iter->initial,args));
                    if(init_type->value_kind == ExpressionType::error)continue;
                    if(iter->annotation){
                        auto anno_prototype = any_cast<Prototype*>(visitAnnotation(iter->annotation,args));
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
                    auto init_type = any_cast<ExpressionType*>(visitExpression(iter->initial,args));
                    result_prototype = init_type->prototype;
                }
                else if(iter->annotation != nullptr){
                    result_prototype = any_cast<Prototype*>(visitAnnotation(iter->annotation,args));
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


    std::any TypeAnalyzer::visitSelect(ast::stmt::Select *select_node, TypeAnalyzerArgs args) {
        auto condition_type = any_cast<ExpressionType*>(visitExpression((*select_node).condition,args));
        auto iter = (*select_node).case_;
        while(iter){
            if(iter->condition){
                auto case_type = any_cast<ExpressionType*>(visitExpression(iter->condition,args));
                if(!condition_type->prototype->equal(case_type->prototype)){
                    Logger::error(iter->location,format()<<"case condition type '"
                                                         <<case_type->prototype->getName()<<"' is not equivalent to select expression type '"
                                                         <<condition_type->prototype->getName()<<"'");
                }
            }
            visitStatementList(iter->statement,args);
        }

        return {};
    }

    std::any TypeAnalyzer::visitLoop(ast::stmt::Loop *loop_node, TypeAnalyzerArgs args) {
        auto condition_type = any_cast<ExpressionType*>(visitExpression((*loop_node).condition,args));
        if(!condition_type->prototype->equal(args.context->getBuiltIn().getPrimitive(vm::Data::boolean))){
            Logger::error((*loop_node).condition->location,"expression type of loop condition must be Boolean");
        }
        visitStatementList((*loop_node).statement,args);
        return {};
    }

    std::any TypeAnalyzer::visitIf(ast::stmt::If *ifstmt_node, TypeAnalyzerArgs args) {
        auto iter = (*ifstmt_node).case_;
        while(iter){
            if(iter->condition){
                auto case_type = any_cast<ExpressionType*>(visitExpression(iter->condition,args));
                if(!case_type->prototype->equal(args.context->getBuiltIn().getPrimitive(vm::Data::boolean))){
                    Logger::error(iter->location,format()<<"expression type of if condition must be boolean but here is '"
                                                         <<case_type->prototype->getName()<<"'");
                }
            }
            visitStatementList(iter->statement,args);
        }
        return {};
    }

    std::any TypeAnalyzer::visitFor(ast::stmt::For *forstmt_node, TypeAnalyzerArgs args) {
        auto iterator_type = any_cast<ExpressionType*>(visitExpression((*forstmt_node).iterator,args));
        if(iterator_type->value_kind == ExpressionType::error){
            Logger::error((*forstmt_node).iterator->location,"iterator not found");
        }
        else if(iterator_type->value_kind != ExpressionType::lvalue){
            Logger::error((*forstmt_node).iterator->location,"iterator must be a lvalue");
        }
        else{
            auto begin_type = any_cast<ExpressionType*>(visitExpression((*forstmt_node).begin,args)),
                    end_type = any_cast<ExpressionType*>(visitExpression((*forstmt_node).end,args));

            if(!begin_type->prototype->equal(iterator_type->prototype)){
                Logger::error((*forstmt_node).begin->location,format()<<"Begin expression type '"
                                                                       <<begin_type->prototype->getName()<<"' is not equivalent to iterator type '"
                                                                       <<iterator_type->prototype->getName()<<"'");
            }
            if(!end_type->prototype->equal(iterator_type->prototype)){
                Logger::error((*forstmt_node).begin->location,format()<<"End expression type '"
                                                                       <<end_type->prototype->getName()<< "' is not equivalent to iterator type '"
                                                                       <<iterator_type->prototype->getName()<<"'");
            }
            if((*forstmt_node).step){
                auto step_type = any_cast<ExpressionType*>(visitExpression((*forstmt_node).step,args));
                if(!step_type->prototype->equal(iterator_type->prototype)){
                    Logger::error((*forstmt_node).begin->location,format()<<"Step expression type '"
                                                                           <<step_type->prototype->getName()<<"' is not equivalent to iterator type '"
                                                                           <<iterator_type->prototype->getName()<<"'");
                }
            }
        }

        visitStatementList((*forstmt_node).statement,args);
        return {};
    }

    std::any TypeAnalyzer::visitReturn(ast::stmt::Return *ret_node, TypeAnalyzerArgs args) {
        auto type = any_cast<ExpressionType*>(visitExpression((*ret_node).expr,args));
        if(type->value_kind == ExpressionType::error)return {};

        auto dst_prototype = args.user_function->getRetSignature();
        if(!dst_prototype->equal(type->prototype)){
            if(args.context->getConversionRules().isImplicitCastRuleExist(type->prototype,dst_prototype)){
                Logger::warning(ret_node->expr->location, format() << "implicit conversion from '" << type->prototype->getName()
                                                                      << "' to '" << dst_prototype->getName() << "'");
                args.context->getConversionRules().insertCastAST(dst_prototype,&ret_node->expr);
            }
            else{
                Logger::error(ret_node->location,format()<<"cannot implicit convert '"
                                                           <<type->prototype->mangling('.')
                                                           <<"' to Integer");
            }
        }

//        if(!type->prototype->equal(args.user_function->getRetSignature())){
//            Logger::error((*ret_node).location,format()<<"Return type '"
//                                                        <<type->prototype->getName()<<"' is not equivalent to Function signature '"
//                                                        <<args.user_function->getRetSignature()->getName()<<"'");
//        }
        return {};
    }
    
    std::any TypeAnalyzer::visitExpression(ast::expr::Expression *expr_node, TypeAnalyzerArgs args) {
        using exp = ast::expr::Expression;
        using namespace ast::expr;
        switch ((*expr_node).expression_kind) {
            case exp::binary_:      return visitBinary((Binary*)expr_node,args);
            case exp::unary_:       return visitUnary((Unary*)expr_node, args);
            case exp::digit_:       return visitDigit((Digit*)expr_node,args);
            case exp::decimal_:     return visitDecimal((Decimal *)expr_node,args);
            case exp::string_:      return visitString((String*)expr_node,args);
            case exp::char_:        return visitChar((Char*)expr_node,args);
            case exp::parentheses_: return visitParentheses((Parentheses*)expr_node,args);
            case exp::callee_:      return visitCallee((Callee*)expr_node,args);
            case exp::boolean_:     return visitBoolean((Boolean*)expr_node,args);
            case exp::ID_:          return visitID((ID*)expr_node,args);
            case exp::new_:         return visitNew((New*)expr_node,args);
            case exp::index_:       return visitIndex((Index*)expr_node,args);
            case exp::dot_:         return visitDot((Dot*)expr_node,args);
            case exp::assign_:      return visitAssign((Assign*)expr_node,args);
        }
        PANIC;
    }

    std::any TypeAnalyzer::visitUnary(ast::expr::Unary *unary_node, TypeAnalyzerArgs args) {
        return visitExpression(unary_node->terminal,args);
    }

    std::any TypeAnalyzer::visitDigit(ast::expr::Digit *digit_node, TypeAnalyzerArgs args) {
        auto primitive = args.context->getBuiltIn().getPrimitive(vm::Data::i32);
        return (*digit_node).type = new ExpressionType(primitive,ExpressionType::rvalue);
    }

    std::any TypeAnalyzer::visitDecimal(ast::expr::Decimal *decimal_node, TypeAnalyzerArgs args) {
        auto primitive = args.context->getBuiltIn().getPrimitive(vm::Data::i32);
        return (*decimal_node).type = new ExpressionType(primitive,ExpressionType::rvalue);
    }

    std::any TypeAnalyzer::visitBoolean(ast::expr::Boolean *bl_node, TypeAnalyzerArgs args) {
        auto primitive = args.context->getBuiltIn().getPrimitive(vm::Data::i32);
        return (*bl_node).type = new ExpressionType(primitive,ExpressionType::rvalue);
    }

    std::any TypeAnalyzer::visitChar(ast::expr::Char *ch_node, TypeAnalyzerArgs args) {
        auto primitive = args.context->getBuiltIn().getPrimitive(vm::Data::i32);
        return (*ch_node).type = new ExpressionType(primitive,ExpressionType::rvalue);
    }

    std::any TypeAnalyzer::visitString(ast::expr::String *str_node, TypeAnalyzerArgs args) {
        auto cls = args.context->getBuiltIn().getStringClass();
        return (*str_node).type = new ExpressionType(cls,ExpressionType::rvalue);
    }
    
    std::any TypeAnalyzer::visitBinary(ast::expr::Binary *binary_node, TypeAnalyzerArgs args) {
        auto lhs_type = any_cast<ExpressionType*>(visitExpression(binary_node->lhs,args));
        auto rhs_type = any_cast<ExpressionType*>(visitExpression(binary_node->rhs,args));
        if(lhs_type->value_kind == ExpressionType::error || rhs_type->value_kind == ExpressionType::error)
            return ExpressionType::Error;

        auto boolean_prototype = args.context->getBuiltIn().getPrimitive(vm::Data::i8);
        auto is_boolean = [&](ExpressionType *type,Location *location)->bool{
            if(!type->prototype->equal(boolean_prototype)){
                Logger::error(location,"expression type must be Boolean");
                return false;
            }
            return true;
        };
        
        using Op = ast::expr::Binary;
        switch (binary_node->op) {
            case Op::And:
            case Op::Or:
            case Op::Xor:{
                if(!is_boolean(lhs_type,binary_node->lhs->location) ||
                   !is_boolean(rhs_type,binary_node->rhs->location)){
                    return ExpressionType::Error;
                }
                return binary_node->type = new ExpressionType(boolean_prototype,ExpressionType::rvalue);

            }
            case Op::Not:{
                if(!is_boolean(rhs_type,binary_node->rhs->location)){
                    return ExpressionType::Error;
                }
                return binary_node->type = new ExpressionType(boolean_prototype,ExpressionType::rvalue);
            }
            case Op::EQ:
            case Op::NE:
            case Op::GE:
            case Op::LE:
            case Op::GT:
            case Op::LT:{
                if(!check_binary_op_valid(binary_node->location,args.context->getConversionRules(),
                                          lhs_type->prototype,rhs_type->prototype,&binary_node->lhs,&binary_node->rhs))
                    return ExpressionType::Error;
                return binary_node->type = new ExpressionType(boolean_prototype,ExpressionType::rvalue);
            }
            case Op::ADD:
            case Op::MINUS:
            case Op::MUL:
            case Op::DIV:
            case Op::FDIV:{
                if(!check_binary_op_valid(binary_node->location,args.context->getConversionRules(),
                                          lhs_type->prototype,rhs_type->prototype,&binary_node->lhs,&binary_node->rhs))
                    return ExpressionType::Error;
                return binary_node->type = new ExpressionType(lhs_type->prototype,ExpressionType::rvalue);
            }
            default:
                PANIC;
        }
    }
    
    std::any TypeAnalyzer::visitDot(ast::expr::Dot *dot_node, TypeAnalyzerArgs args) {
        auto lhs_type = any_cast<ExpressionType*>(visitExpression(dot_node->lhs,args));
        args.dot_expression_context = lhs_type->prototype;
        auto rhs_type = any_cast<ExpressionType*>(visitExpression(dot_node->rhs,args));

        if(lhs_type->is_static xor rhs_type->is_static){
            Logger::error(dot_node->rhs->location,"cannot find object");
        }

        return dot_node->type = rhs_type;
    }

    std::any TypeAnalyzer::visitID(ast::expr::ID *id_node, TypeAnalyzerArgs args) {
        try{
            auto name = getID(id_node);
            Symbol *target = nullptr;
            if(!args.dot_expression_context){
                target = args.domain->lookUp(name);
                if(!target) throw SymbolNotFound(id_node->location,nullptr,name);
            }
            else{
                auto domain = args.dot_expression_context->as<Domain*>();
                if(!(domain && (target = domain->find(name)))){
                    throw SymbolNotFound(id_node->location,args.dot_expression_context,name);
                }
            }

            check_access(id_node->location,target,args.domain,args.current_class_or_module);

            switch (target->getKind()) {
                case type::SymbolKind::Variable:
                case type::SymbolKind::Argument:
                    return new ExpressionType(target->as<type::Variable*>()->getPrototype(),ExpressionType::lvalue,target->isStatic());
                default:
                    return new ExpressionType(target->as<Prototype*>(),ExpressionType::path,true);
            }
        }
        catch (SymbolNotFound& e){
            if(e.search_domain){
                Logger::error(e.location,format()<<"cannot find object '"
                                                 <<e.search_domain->mangling('.')
                                                 <<'.'<<e.search_name<<"'");
            }
            else{
                Logger::error(e.location,format()<<"cannot find object '"<<e.search_name<<"'");
            }
            return ExpressionType::Error;
        }
    }

    std::any TypeAnalyzer::visitAssign(ast::expr::Assign *assign_node, TypeAnalyzerArgs args) {
        auto lhs_type = any_cast<ExpressionType*>(visitExpression(assign_node->lhs,args));
        auto rhs_type = any_cast<ExpressionType*>(visitExpression(assign_node->rhs,args));
        if(lhs_type->value_kind == ExpressionType::error || rhs_type->value_kind == ExpressionType::error)
            return ExpressionType::Error;

        if(lhs_type->value_kind != ExpressionType::lvalue){
            Logger::error(assign_node->location,"lvalue required as left operand of assignment");
        }

        check_binary_op_valid(assign_node->location,args.context->getConversionRules(),
                              lhs_type->prototype,rhs_type->prototype,&assign_node->lhs,&assign_node->rhs);

        return assign_node->type = lhs_type;
    }

    std::any TypeAnalyzer::visitIndex(ast::expr::Index *index_node, TypeAnalyzerArgs args) {
        auto target_type = any_cast<ExpressionType*>(visitExpression(index_node->target,args));
        auto value_type = any_cast<ExpressionType*>(visitExpression(index_node->value,args));

        Prototype *dst_prototype = nullptr,*ret_prototype = nullptr;
        switch(target_type->prototype->getKind()){
            case SymbolKind::Array:
                dst_prototype = args.context->getBuiltIn().getPrimitive(vm::Data::i32);
                ret_prototype = target_type->prototype->as<Array*>()->getElementPrototype();
                break;
            case SymbolKind::Class:{
                //TODO index operator overload
                break;
            }
        }

        if(!dst_prototype->equal(value_type->prototype)){
            if(args.context->getConversionRules().isImplicitCastRuleExist(value_type->prototype,dst_prototype)){
                Logger::warning(index_node->value->location, format() << "implicit conversion from '" << value_type->prototype->getName()
                                                                     << "' to '" << dst_prototype->getName() << "'");
                args.context->getConversionRules().insertCastAST(dst_prototype,&index_node->value);
            }
            else{
                Logger::error(index_node->location,format()<<"cannot implicit convert '"
                                                           <<value_type->prototype->mangling('.')
                                                           <<"' to Integer");
            }
        }
        
        return index_node->type = new ExpressionType(ret_prototype,ExpressionType::lvalue);
    }

    std::any TypeAnalyzer::visitCallee(ast::expr::Callee *callee_node, TypeAnalyzerArgs args) {
        auto target_type = any_cast<ExpressionType*>(visitExpression(callee_node->name,args));
        
        if(target_type->value_kind == ExpressionType::error){
            return target_type;
        }

        auto func = target_type->prototype->as<type::Function*>();

        if(!func){
            Logger::error(callee_node->name->location,format()<<"'"<<target_type->prototype->getName()<<"' is not a callable target");
            return ExpressionType::Error;
        }

        check_callee(callee_node->location,callee_node->argument,func,args);

        auto ret = func->getRetSignature();
        return callee_node->type = new ExpressionType(ret->as<type::Prototype*>(),ExpressionType::rvalue);
    }


    std::any TypeAnalyzer::visitNew(ast::expr::New *new_node, TypeAnalyzerArgs args) {
        auto prototype = any_cast<Prototype*>(visitAnnotation(new_node->annotation,args));

        auto cls = prototype->as<type::Class*>();
        if(!cls){
            Logger::error(new_node->location,"type is not a Class");
            return ExpressionType::Error;
        }

        auto init = cls->find("init")->as<UserFunction*>();
        if(!init){
            Logger::error(new_node->location,format()<<"cannot find initializer in '"<<cls->mangling('.')<<"'");
            return ExpressionType::Error;
        }

        check_callee(new_node->location,new_node->argument,init,args);

        return new ExpressionType(cls,ExpressionType::rvalue);
    }

    std::any TypeAnalyzer::visitExprStmt(ast::stmt::ExprStmt *expr_stmt_node, TypeAnalyzerArgs args) {
        args.dot_expression_context = nullptr;
        return visitExpression(expr_stmt_node->expr,args);
    }

    std::any TypeAnalyzer::visitParentheses(ast::expr::Parentheses *parentheses_node, TypeAnalyzerArgs args) {
        args.dot_expression_context = nullptr;
        return visitExpression(parentheses_node->expr,args);
    }

    void TypeAnalyzer::check_access(Location *code_location, Symbol *target, Domain *current, Domain *current_class_or_module){
        switch(target->getAccessFlag()){
            case AccessFlag::Public:
                break;
            case AccessFlag::Private:{
                auto domain = current;
                while(domain){
                    if(target->getParent() == domain)return;
                    domain = domain->getParent();
                }
                Logger::error(code_location,format()<<"'"<<target->mangling('.')<<"' is private");
                break;
            }
            case AccessFlag::Protected:{
                if(current_class_or_module->getKind()==SymbolKind::Class){
                    auto domain = current_class_or_module->as<type::Class*>();
                    while(domain){
                        if(target->getParent() == domain)return;
                        domain = domain->getExtend();
                    }
                }
                Logger::error(code_location,format()<<"'"<<target->mangling('.')<<"' is protected");
                break;
            }
        }
    }
    
    bool TypeAnalyzer::check_binary_op_valid(Location *code,ConversionRules &rules,
                                             Prototype *lhs,Prototype *rhs,Expression **lhs_node,Expression **rhs_node) {
        if(!rhs->equal(lhs)){
            auto result = rules.getImplicitPromotionRule(lhs,rhs);
            if(result.has_value()){
                auto rule = result.value();
                auto result_type = rule.second;
                if(!lhs->equal(result_type)) {
                    Logger::warning((*lhs_node)->location,format()<<"implicit conversion from '"<<lhs->getName()
                                                             <<"' to '"<<result_type->getName()<<"'");
                    rules.insertCastAST(result_type, lhs_node);
                }
                if(!rhs->equal(result_type)) {
                    Logger::warning((*rhs_node)->location,format()<<"implicit conversion from '"<<rhs->getName()
                                                             <<"' to '"<<result_type->getName()<<"'");
                    rules.insertCastAST(result_type, rhs_node);
                }
                return true;
            }
            else{
                Logger::error(code,format()<< "invalid operands to binary expression.lhs type is '"
                                                           << lhs->getName()
                                                           << "' and rhs type is'"<<rhs->getName());
                return false;
            }
        }
        else{
            return true;
        }
    }

    void TypeAnalyzer::check_callee(Location *location, Argument *argument, type::Function *target, TypeAnalyzerArgs args){
        auto args_count = 0;
        auto params_count = target->getArgsSignature().size();

        auto arg = argument;
        while(arg){
            arg = arg->next_sibling;
            args_count++;
        }

        if(args_count > params_count){
            Logger::error(location,format()<<"too many arguments to function call, expected "
                                           <<params_count<<", have "<<args_count);
        }
        else if(args_count < params_count){
            Logger::error(location,format()<<"too few arguments to function call, expected "
                                           <<params_count<<", have "<<args_count);
        }
        else{
            args.checking_function = target;
            args.checking_arg_index = 0;
            while(argument){
                visitArg(argument,args);
                argument = argument->next_sibling;
                args.checking_arg_index++;
            }
        }

    }

    std::any TypeAnalyzer::visitAnnotation(ast::Annotation *annotation_node, TypeAnalyzerArgs args) {
        auto iter = (*annotation_node).unit;
        args.need_lookup = true;
        args.dot_expression_context = args.domain;
        auto symbol = visitAnnotationUnit(iter,args);
        iter = iter->next_sibling;
        args.dot_expression_context = any_cast<Symbol*>(symbol);

        args.need_lookup = false;
        while(iter!=nullptr){
            symbol = visitAnnotationUnit(iter,args);
            args.dot_expression_context = any_cast<Symbol*>(symbol);
            iter=iter->next_sibling;
        }

        auto element = args.dot_expression_context->as<Prototype*>();
        auto ret_prototype = element;

        if((*annotation_node).array_size){
            ret_prototype = new type::Array(ret_prototype, getDigit((*annotation_node).array_size));

            if(element->getKind() == SymbolKind::Record)
                args.context->byteLengthDependencies.addDependent(ret_prototype->as<Domain*>(),element->as<Domain*>());
        }

        return ret_prototype;
    }

    std::any TypeAnalyzer::visitAnnotationUnit(ast::AnnotationUnit *unit_node, TypeAnalyzerArgs args) {
        auto name = getID((*unit_node).name);
        Symbol *symbol = nullptr;
        if(args.need_lookup){
            symbol = args.dot_expression_context->as<Domain*>()->lookUp(name);
        }
        else{
            symbol = args.dot_expression_context->as<Domain*>()->find(name);
        }

        if(!symbol){
            throw SymbolNotFound((*unit_node).name->location,args.dot_expression_context,name);
        }
        return symbol;
    }

}
