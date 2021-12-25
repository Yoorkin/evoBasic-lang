//
// Created by yorkin on 11/24/21.
//

#include "typeAnalyzer.h"
#include "semantic.h"
#include "logger.h"
#include "i18n.h"
#include "ast.h"

namespace evoBasic{
    using namespace std;
    using namespace type;
    using namespace i18n;



    /*   ==============================Parameter passing strategy====================================
     *   Param\Arg            ByVal                  ByRef                Undefined
     *   ByVal      Yes,allow implicit conversion.    Error         Yes,allow implicit conversion.
     *   ByRef      store value to tmp address,    Error when arg        Error when arg
     *              allow implicit conversion.      is not lvalue.        is not lvalue.
     *   ============================================================================================
     */

    void try_implicit_conversion(Location *location,ast::Expression **expr,type::Prototype *target,Context *context){
        auto prototype = (*expr)->type->getPrototype();
        if(context->getConversionRules().isImplicitCastRuleExist(prototype,target)){
            Logger::warning(location,lang->fmtImplicitCvtFromAToB(prototype->getName(),target->getName()));
            context->getConversionRules().insertCastAST(target,expr);
        }
        else{
            Logger::error(location, lang->fmtCannotImplicitCvtAToB(prototype->getName(),target->getName()));
        }
    }

    void passValToVal(Location *location,ast::Argument *ast_node,Context *context){
        if(!ast_node->parameter->getPrototype()->equal(ast_node->expr->type->getPrototype())){
            try_implicit_conversion(location,&ast_node->expr,ast_node->parameter->getPrototype(),context);
        }
        ast_node->byval = true;
    }

    void passRefToRef(Location *location,ast::Argument *ast_node){
        if(ast_node->expr->type->value_kind != ExpressionType::lvalue){
            Logger::error(location, lang->msgCannotPassTmpValByRefImplicit());
        }
        else if(!ast_node->parameter->getPrototype()->equal(ast_node->expr->type->getPrototype())){
            auto arg_prototype = ast_node->expr->type->getPrototype();
            auto param_prototype = ast_node->parameter->getPrototype();
            Logger::error(location, lang->fmtArgCannotMatchParam(param_prototype->getName(),arg_prototype->getName()));
        }
        ast_node->byval = false;
    }

    void passValToRef(Location *location,ast::Argument *ast_node,
                      type::Function *function,Context *context){
        auto arg_prototype = ast_node->expr->type->getPrototype();
        auto param_prototype = ast_node->parameter->getPrototype();


        if(!ast_node->parameter->getPrototype()->equal(ast_node->expr->type->getPrototype())){
            try_implicit_conversion(location,&ast_node->expr,ast_node->parameter->getPrototype(),context);
        }

        auto tmp_var = new type::Variable;
        tmp_var->setPrototype(param_prototype);
        function->addMemoryLayout(tmp_var);

        auto assign = new ast::Assign(new ast::Local(tmp_var,new ExpressionType(param_prototype,ExpressionType::lvalue)),ast_node->expr);
        ast_node->expr = assign;

        switch (ast_node->parameter->getPrototype()->getKind()) {
            case SymbolKind::Record:
            case SymbolKind::Array:
                context->byteLengthDependencies.addDependent(function, ast_node->parameter->getPrototype()->as<Domain*>());
                break;
        }
        ast_node->byval = false;
    }


    void parameterPassingResolution(Location *location,parseTree::Argument::PassKind pass_kind,ast::Argument *ast_node,
                                    type::Function *current_function, Context *context){

        using passBy = parseTree::expr::Argument;
        if(ast_node->parameter->isByval()){
            switch(pass_kind) {
            case passBy::undefined:
            case passBy::byval:
                passValToVal(location,ast_node,context);
                break;
            case passBy::byref:
                Logger::error(location, lang->msgExpectedByValButByRef());
                break;
            }
        }
        else{
            switch (pass_kind) {
            case passBy::byval:
                passValToRef(location, ast_node, current_function, context);
                break;
            case passBy::byref:
            case passBy::undefined:
                passRefToRef(location,ast_node);
                break;
            }
        }
    }

    std::any TypeAnalyzer::visitArg(parseTree::expr::Argument *argument_node, TypeAnalyzerArgs args) {
        NotNull(args.checking_function);
        auto function = args.checking_function;
        type::Parameter *param = nullptr;
        auto *ast_node = new ast::Argument;
        if(argument_node->expr->expression_kind == parseTree::expr::Expression::colon_){
            // parameter initialization
            auto colon_node = (parseTree::Colon*)(argument_node->expr);
            if(colon_node->lhs->expression_kind != parseTree::expr::Expression::ID_){
                Logger::error(colon_node->lhs->location,lang->msgExpectedParamNameInOptInitialization());
                return ast::Expression::error;
            }
            auto init_name = getID((parseTree::ID*)colon_node->lhs);
            auto opt_index = args.checking_function->findOptionIndex(init_name);
            if(!opt_index.has_value()){
                Logger::error(colon_node->lhs->location,lang->fmtOptNotFoundInFtn(init_name,args.checking_function->getName()));
                return ast::Expression::error;
            }
            param = args.checking_function->getArgsOptions()[opt_index.value()];
        }
        else{
            // regular parameter
            if(args.checking_arg_index >= function->getArgsSignature().size()){
                if(!function->getParamArray())return ast::Expression::error;
                param = function->getParamArray(); // paramArray
            }
            else {
                param = function->getArgsSignature()[args.checking_arg_index];
            }
        }

        args.dot_prefix = nullptr;
        auto ast_arg = any_cast<ast::Expression*>(visitExpression(argument_node->expr, args));
        if(ast_arg->type->value_kind == ExpressionType::error)return ast::Expression::error;

        ast_node->expr = ast_arg;
        ast_node->parameter = param;
        ast_node->type = ast_arg->type;

        parameterPassingResolution(argument_node->location,argument_node->pass_kind,ast_node,args.function,args.context);

        return (ast::Expression*)ast_node;
    }

    std::any TypeAnalyzer::visitMember(parseTree::Member *member_node, TypeAnalyzerArgs args) {
        switch ((*member_node).member_kind) {
            case parseTree::Member::function_:      return visitFunction((parseTree::Function*)member_node, args);
            case parseTree::Member::class_:         return visitClass((parseTree::Class*)member_node, args);
            case parseTree::Member::module_:        return visitModule((parseTree::Module*)member_node, args);
            case parseTree::Member::type_:          return visitType((parseTree::Type*)member_node,args);
            case parseTree::Member::enum_:          return visitEnum((parseTree::Enum*)member_node,args);
            case parseTree::Member::dim_:           return visitDim((parseTree::Dim*)member_node,args);
            case parseTree::Member::external_:      return visitExternal((parseTree::External*)member_node, args);
            case parseTree::Member::interface_:     return visitInterface((parseTree::Interface*)member_node, args);
            case parseTree::Member::constructor_:   return visitConstructor((parseTree::Constructor*)member_node, args);
        }
        PANIC;
    }

    std::any TypeAnalyzer::visitStatement(parseTree::stmt::Statement *stmt_node, TypeAnalyzerArgs args) {
        switch ((*stmt_node).stmt_flag) {
            case parseTree::stmt::Statement::let_:      return visitLet((parseTree::stmt::Let*)stmt_node, args);
            case parseTree::stmt::Statement::loop_:     return visitLoop((parseTree::stmt::Loop*)stmt_node, args);
            case parseTree::stmt::Statement::if_:       return visitIf((parseTree::stmt::If*)stmt_node, args);
            case parseTree::stmt::Statement::for_:      return visitFor((parseTree::stmt::For*)stmt_node, args);
            case parseTree::stmt::Statement::select_:   return visitSelect((parseTree::stmt::Select*)stmt_node, args);
            case parseTree::stmt::Statement::return_:   return visitReturn((parseTree::stmt::Return*)stmt_node, args);
            case parseTree::stmt::Statement::continue_: return visitContinue((parseTree::stmt::Continue*)stmt_node, args);
            case parseTree::stmt::Statement::exit_:     return visitExit((parseTree::stmt::Exit*)stmt_node, args);
            case parseTree::stmt::Statement::expr_:     return visitExprStmt((parseTree::stmt::ExprStmt*)stmt_node, args);
        }
        PANIC;
    }

    std::any TypeAnalyzer::visitGlobal(parseTree::Global *global_node, TypeAnalyzerArgs args) {
        auto ast_node = new ast::Global;
        ast_node->global_symbol = args.context->getGlobal();
        ast_node->member = visitAllMember(global_node->global_symbol,global_node->member,args);
        return ast_node;
    }
    std::any TypeAnalyzer::visitModule(parseTree::Module *module_node, TypeAnalyzerArgs args) {
        auto ast_node = new ast::Module;
        ast_node->module_symbol = module_node->module_symbol;
        ast_node->member = visitAllMember(module_node->module_symbol,module_node->member,args);
        return (ast::Member*)ast_node;
    }
    std::any TypeAnalyzer::visitClass(parseTree::Class *class_node, TypeAnalyzerArgs args) {
        auto ast_node = new ast::Class;
        ast_node->class_symbol = class_node->class_symbol;
        ast_node->member = visitAllMember(class_node->class_symbol,class_node->member,args);
        return (ast::Member*)ast_node;
    }
    
    ast::Member *TypeAnalyzer::visitAllMember(type::Domain *domain, parseTree::Member *member, TypeAnalyzerArgs args) {
        args.domain = args.current_class_or_module = domain;
        ast::Member *tail = nullptr,*head = nullptr;
        FOR_EACH(iter,member){
            auto ast_member = any_cast<ast::Member*>(visitMember(iter,args));
            if(tail == nullptr){
                head = tail = ast_member;
            }
            else{
                tail->next_sibling = ast_member;
                ast_member->prv_sibling = tail;
                tail = ast_member;
            }
        }
        return head;
    }

    ast::Statement *TypeAnalyzer::visitStatementList(parseTree::stmt::Statement *stmt_list, TypeAnalyzerArgs args) {
        args.domain = new type::TemporaryDomain(args.domain,args.function);
        ast::Statement *tail = nullptr,*head = nullptr;
        FOR_EACH(iter,stmt_list){
            auto statement = any_cast<ast::Statement*>(visitStatement(iter,args));
            if(tail == nullptr){
                tail = head = statement;
            }
            else{
                tail->next_sibling = statement;
                statement->prv_sibling = tail;
                tail = statement;
            }
        }
        return head;
    }

    std::any TypeAnalyzer::visitFunction(parseTree::Function *function_node, TypeAnalyzerArgs args) {
        auto ast_node = new ast::Function;
        auto function = function_node->function_symbol;
        args.domain = args.function = ast_node->function_symbol = function;
        //verify default expression of optional parameter
        args.checking_arg_index = 0;
        args.checking_function = function;
        FOR_EACH(iter,function_node->parameter){
            if(iter->is_optional){
                visitParameter(iter,args);
                args.checking_arg_index++;
            }
        }
        ast_node->statement = visitStatementList(function_node->statement,args);
        return (ast::Member*)ast_node;
    } 

    std::any TypeAnalyzer::visitConstructor(parseTree::Constructor *ctor_node, TypeAnalyzerArgs args) {
        auto ast_node = new ast::Constructor;
        auto function = ctor_node->constructor_symbol;
        args.domain = args.function = ast_node->constructor_symbol = function;
        ast_node->statement = visitStatementList(ctor_node->statement,args);
        return (ast::Member*)ast_node;
    }

    std::any TypeAnalyzer::visitLet(parseTree::stmt::Let *let_node, TypeAnalyzerArgs args) {
        auto ast_node = new ast::Let;
        auto tail = ast_node->variable;
        FOR_EACH(iter,let_node->variable){
            auto name = getID(iter->name);
            if(is_name_valid(name,iter->location,args.domain)){
                auto ast_node_var = new ast::Variable;
                if(tail == nullptr){
                    ast_node->variable = tail = ast_node_var;
                }
                else{
                    tail->next_sibling = ast_node_var;
                    ast_node_var->prv_sibling = tail;
                    tail = ast_node_var;
                }
                
                type::Prototype *result_prototype = nullptr;
                if(iter->initial != nullptr && iter->annotation != nullptr){
                    auto ast_initial = any_cast<ast::Expression*>(visitExpression(iter->initial,args));
                    if(ast_initial->type->value_kind == ExpressionType::error)continue;
                    if(iter->annotation){
                        auto anno_prototype = any_cast<Prototype*>(visitAnnotation(iter->annotation,args));
                        if(!ast_initial->type->getPrototype()->equal(anno_prototype)){
                            if(args.context->getConversionRules().isImplicitCastRuleExist(ast_initial->type->getPrototype(),anno_prototype)){
                                Logger::warning(iter->initial->location,
                                                lang->fmtImplicitCvtFromAToB(ast_initial->type->getPrototype()->getName(),anno_prototype->getName()));
                                //args.context->getConversionRules().insertCastAST(anno_prototype,&(iter->initial)); todo
                            }
                            else {
                                Logger::error(iter->initial->location,
                                              lang->fmtLetStmtVariableInitialNotMatch(ast_initial->type->getPrototype()->getName(),anno_prototype->getName()));
                                continue;
                            }
                        }
                        result_prototype = anno_prototype;
                    }
                    else{
                        result_prototype = ast_initial->type->getPrototype();
                    }
                    ast_node_var->initial = ast_initial;
                }
                else if(iter->initial != nullptr){
                    auto ast_initial = any_cast<ast::Expression*>(visitExpression(iter->initial,args));
                    result_prototype = ast_initial->type->getPrototype();
                    ast_node_var->initial = ast_initial;
                }
                else if(iter->annotation != nullptr){
                    result_prototype = any_cast<Prototype*>(visitAnnotation(iter->annotation,args));
                }
                else{
                    Logger::error(iter->location, lang->msgLetStmtNeedInitialValueOrTypeMark());
                    continue;
                }

                switch (result_prototype->getKind()) {
                    case type::SymbolKind::Record:
                    case type::SymbolKind::Array:
                        args.context->byteLengthDependencies.addDependent(args.function, result_prototype->as<Domain*>());
                }

                auto field = new type::Variable;
                field->setName(name);
                field->setPrototype(result_prototype);
                args.domain->add(field);
                ast_node_var->variable_symbol = field;
            }
        }
        return (ast::Statement*)ast_node;
    }


    std::any TypeAnalyzer::visitSelect(parseTree::stmt::Select *select_node, TypeAnalyzerArgs args) {
        auto ast_node = new ast::Select;
        auto ast_condition = any_cast<ast::Expression*>(visitExpression(select_node->condition,args));
        auto tail = ast_node->case_;
        FOR_EACH(iter,select_node->case_){
            auto ast_case = new ast::Case;
            if(tail == nullptr){
                ast_node->case_ = tail = ast_case;
            }
            else{
                tail->next_sibling = ast_case;
                ast_case->prv_sibling = tail;
                tail = ast_case;
            }
            
            if(iter->condition){
                auto ast_case_condition = any_cast<ast::Expression*>(visitExpression(iter->condition,args));
                ast_case->condition = ast_case_condition;
                if(!ast_condition->type->getPrototype()->equal(ast_case_condition->type->getPrototype())){
                    Logger::error(iter->location, lang->fmtSelectCaseTypeNotMatch(ast_case_condition->type->getPrototype()->getName(),ast_condition->type->getPrototype()->getName()));
                }
            }
            ast_case->statement = visitStatementList(iter->statement,args);
        }

        return (ast::Statement*)ast_node;
    }

    std::any TypeAnalyzer::visitLoop(parseTree::stmt::Loop *loop_node, TypeAnalyzerArgs args) {
        auto ast_node = new ast::Loop;
        auto ast_condition = any_cast<ast::Expression*>(visitExpression(loop_node->condition,args));
        if(!ast_condition->type->getPrototype()->equal(args.context->getBuiltIn().getPrimitive(vm::Data::boolean))){
            Logger::error(loop_node->condition->location, lang->msgExpExpectedBoolean());
        }
        ast_node->condition = ast_condition;
        ast_node->statement = visitStatementList(loop_node->statement,args);
        return (ast::Statement*)ast_node;
    }

    std::any TypeAnalyzer::visitIf(parseTree::stmt::If *ifstmt_node, TypeAnalyzerArgs args) {
        auto ast_node = new ast::If;
        auto tail = ast_node->case_;
        FOR_EACH(iter,ifstmt_node->case_){
            auto ast_case = new ast::Case;
            if(tail == nullptr){
                ast_node->case_ = tail = ast_case;
            }
            else{
                tail->next_sibling = ast_case;
                ast_case->prv_sibling = tail;
                tail = ast_case;
            }
            
            if(iter->condition){
                auto ast_case_condition = any_cast<ast::Expression*>(visitExpression(iter->condition,args));
                ast_case->condition = ast_case_condition;
                if(!ast_case_condition->type->getPrototype()->equal(args.context->getBuiltIn().getPrimitive(vm::Data::boolean))){
                    Logger::error(iter->location, lang->fmtIfConditionExpectedBooleanButA(ast_case_condition->type->getPrototype()->getName()));
                }
            }
            ast_case->statement = visitStatementList(iter->statement,args);
        }
        return (ast::Statement*)ast_node;
    }

    std::any TypeAnalyzer::visitFor(parseTree::stmt::For *forstmt_node, TypeAnalyzerArgs args) {
        auto ast_node = new ast::For;
        auto ast_iterator = any_cast<ast::Expression*>(visitExpression(forstmt_node->iterator,args));
        if(ast_iterator->type->value_kind == ExpressionType::error){
            Logger::error(forstmt_node->iterator->location, lang->msgForStmtIteratorNotFound());
        }
        else if(ast_iterator->type->value_kind != ExpressionType::lvalue){
            Logger::error(forstmt_node->iterator->location, lang->msgForStmtIteratorMustBeLValue());
        }
        else{
            auto ast_begin = any_cast<ast::Expression*>(visitExpression(forstmt_node->begin,args)),
                    ast_end = any_cast<ast::Expression*>(visitExpression(forstmt_node->end,args));
            ast_node->begin = ast_begin;
            ast_node->end = ast_end;

            if(!ast_begin->type->getPrototype()->equal(ast_iterator->type->getPrototype())){
                Logger::error(forstmt_node->begin->location,
                              lang->fmtForStmtBeginExpNotMatch(ast_begin->type->getPrototype()->getName(),ast_iterator->type->getPrototype()->getName()));
            }
            if(!ast_end->type->getPrototype()->equal(ast_iterator->type->getPrototype())){
                Logger::error(forstmt_node->begin->location,
                              lang->fmtForStmtEndExpNotMatch(ast_end->type->getPrototype()->getName(),ast_iterator->type->getPrototype()->getName()));
            }



            auto beg_var = new type::Variable,
                 end_var = new type::Variable;
            beg_var->setPrototype(ast_begin->type->getPrototype());
            args.function->addMemoryLayout(beg_var);
            ast_node->begin_variable = beg_var;
            end_var->setPrototype(ast_end->type->getPrototype());
            args.function->addMemoryLayout(end_var);
            ast_node->end_variable = end_var;

            if((*forstmt_node).step){
                auto ast_step = any_cast<ast::Expression*>(visitExpression(forstmt_node->step,args));
                ast_node->step = ast_step;

                auto step_var = new type::Variable;
                step_var->setPrototype(ast_step->type->getPrototype());
                args.function->addMemoryLayout(step_var);
                ast_node->step_variable = step_var;

                if(!ast_step->type->getPrototype()->equal(ast_iterator->type->getPrototype())){
                    Logger::error(forstmt_node->begin->location,
                                  lang->fmtForStmtStepExpNotMatch(ast_step->type->getPrototype()->getName(),ast_iterator->type->getPrototype()->getName()));
                }
            }
        }
        ast_node->iterator = ast_iterator;
        ast_node->statement = visitStatementList(forstmt_node->statement,args);
        return (ast::Statement*)ast_node;
    }

    std::any TypeAnalyzer::visitReturn(parseTree::stmt::Return *ret_node, TypeAnalyzerArgs args) {
        auto ast_node = new ast::Return;
        auto ast_expr = any_cast<ast::Expression*>(visitExpression((*ret_node).expr,args));
        ast_node->expr = ast_expr;

        if(ast_expr->type->value_kind == ExpressionType::error)return {};

        auto dst_prototype = args.function->getRetSignature();
        if(!dst_prototype->equal(ast_expr->type->getPrototype())){
            if(args.context->getConversionRules().isImplicitCastRuleExist(ast_expr->type->getPrototype(),dst_prototype)){
                Logger::warning(ret_node->expr->location, lang->fmtImplicitCvtFromAToB(ast_expr->type->getPrototype()->getName(),dst_prototype->getName()));
                args.context->getConversionRules().insertCastAST(dst_prototype,&ast_node->expr);
            }
            else{
                Logger::error(ret_node->location, lang->fmtCannotImplicitCvtAToB(ast_expr->type->getPrototype()->mangling('.'),"'Integer'"));
            }
        }

        return (ast::Statement*)ast_node;
    }

    
    std::any TypeAnalyzer::visitExpression(parseTree::expr::Expression *expr_node, TypeAnalyzerArgs args) {
        using exp = parseTree::expr::Expression;
        using namespace parseTree::expr;
        switch ((*expr_node).expression_kind) {
            case exp::binary_:      return visitBinary((Binary*)expr_node,args);
            case exp::unary_:       return visitUnary((Unary*)expr_node, args);
            case exp::digit_:       return visitDigit((Digit*)expr_node,args);
            case exp::decimal_:     return visitDecimal((Decimal *)expr_node,args);
            case exp::string_:      return visitString((String*)expr_node,args);
            case exp::char_:        return visitChar((Char*)expr_node,args);
            case exp::parentheses_: return visitParentheses((Parentheses*)expr_node,args);
            case exp::boolean_:     return visitBoolean((Boolean*)expr_node,args);
            case exp::new_:         return visitNew((New*)expr_node,args);
            case exp::assign_:      return visitAssign((Assign*)expr_node,args);
            case exp::index_:       return visitIndex((Index*)expr_node,args);
            case exp::dot_:         return visitDot((Dot*)expr_node,args);
            case exp::callee_:      return visitCallee((Callee*)expr_node,args);
            case exp::ID_:          return visitID((ID*)expr_node,args);
            case exp::colon_:       return visitColon((Colon*)expr_node,args);
            case exp::cast_:        return visitCast((Cast*)expr_node,args);
        }
        PANIC;
    }

    std::any TypeAnalyzer::visitUnary(parseTree::expr::Unary *unary_node, TypeAnalyzerArgs args) {
        auto ast_node = new ast::Unary(
                unary_node->op,
                any_cast<ast::Expression*>(visitExpression(unary_node->terminal,args))
                );
        ast_node->type = ast_node->terminal->type;
        ast_node->type->value_kind = ExpressionType::rvalue;
        return (ast::Expression*)ast_node;
    }

    std::any TypeAnalyzer::visitDigit(parseTree::expr::Digit *digit_node, TypeAnalyzerArgs args) {
        auto primitive = args.context->getBuiltIn().getPrimitive(vm::Data::i32);
        auto ast_node = new ast::Digit;
        ast_node->value = digit_node->value;
        ast_node->type = new ExpressionType(primitive,ExpressionType::rvalue);
        return (ast::Expression*)ast_node;
    }

    std::any TypeAnalyzer::visitDecimal(parseTree::expr::Decimal *decimal_node, TypeAnalyzerArgs args) {
        auto primitive = args.context->getBuiltIn().getPrimitive(vm::Data::f64);
        auto ast_node = new ast::Decimal;
        ast_node->value = decimal_node->value;
        ast_node->type = new ExpressionType(primitive,ExpressionType::rvalue);
        return (ast::Expression*)ast_node;
    }

    std::any TypeAnalyzer::visitBoolean(parseTree::expr::Boolean *bl_node, TypeAnalyzerArgs args) {
        auto primitive = args.context->getBuiltIn().getPrimitive(vm::Data::boolean);
        auto ast_node = new ast::Boolean;
        ast_node->value = bl_node->value;
        ast_node->type = new ExpressionType(primitive,ExpressionType::rvalue);
        return (ast::Expression*)ast_node;
    }

    std::any TypeAnalyzer::visitChar(parseTree::expr::Char *ch_node, TypeAnalyzerArgs args) {
        auto primitive = args.context->getBuiltIn().getPrimitive(vm::Data::u16);
        auto ast_node = new ast::Char;
        ast_node->value = ch_node->value;
        ast_node->type = new ExpressionType(primitive,ExpressionType::rvalue,DataType::u16);
        return (ast::Expression*)ast_node;
    }

    std::any TypeAnalyzer::visitString(parseTree::expr::String *str_node, TypeAnalyzerArgs args) {
        auto cls = args.context->getBuiltIn().getStringClass();
        // todo: create string Object and push ref into operand stack
        // return (*str_node).type = new ExpressionType(cls,ExpressionType::rvalue,il::ref);
        return {};
    }
    
    std::any TypeAnalyzer::visitBinary(parseTree::expr::Binary *binary_node, TypeAnalyzerArgs args) {
        auto ast_lhs = any_cast<ast::Expression*>(visitExpression(binary_node->lhs,args));
        auto ast_rhs = any_cast<ast::Expression*>(visitExpression(binary_node->rhs,args));
        auto ast_node = new ast::Binary(ast_lhs,binary_node->op,ast_rhs);

        if(ast_lhs->type->value_kind == ExpressionType::error || ast_rhs->type->value_kind == ExpressionType::error)
            return ast::Expression::error;

        auto boolean_prototype = args.context->getBuiltIn().getPrimitive(vm::Data::boolean);
        auto is_boolean = [&](ExpressionType *type,Location *location)->bool{
            if(!type->getPrototype()->equal(boolean_prototype)){
                Logger::error(location, lang->msgExpExpectedBoolean());
                return false;
            }
            return true;
        };
        
        using Op = parseTree::expr::Binary;
        switch (binary_node->op) {
            case Op::And:
            case Op::Or:
            case Op::Xor:{
                if(!is_boolean(ast_lhs->type,binary_node->lhs->location) ||
                   !is_boolean(ast_rhs->type,binary_node->rhs->location)){
                    return ast::Expression::error;
                }
                ast_node->type = new ExpressionType(boolean_prototype,ExpressionType::rvalue);
                break;
            }
            case Op::Not:{
                if(!is_boolean(ast_rhs->type,binary_node->rhs->location)){
                    return ast::Expression::error;
                }
                ast_node->type = new ExpressionType(boolean_prototype,ExpressionType::rvalue);
                break;
            }
            case Op::EQ:
            case Op::NE:
            case Op::GE:
            case Op::LE:
            case Op::GT:
            case Op::LT:{
                if(!check_binary_op_valid(binary_node->location,args.context->getConversionRules(),
                                          ast_lhs->type->getPrototype(),ast_rhs->type->getPrototype(),
                                          binary_node->lhs,binary_node->rhs,
                                          &ast_node->lhs,&ast_node->rhs))
                    return ast::Expression::error;
                ast_node->type = new ExpressionType(boolean_prototype,ExpressionType::rvalue);
                break;
            }
            case Op::ADD:
            case Op::MINUS:
            case Op::MUL:
            case Op::DIV:
            case Op::FDIV:{
                auto result_type = check_binary_op_valid(binary_node->location,args.context->getConversionRules(),
                                                         ast_lhs->type->getPrototype(),ast_rhs->type->getPrototype(),
                                                         binary_node->lhs,binary_node->rhs,
                                                         &ast_node->lhs,&ast_node->rhs);
                if(!result_type) return ast::Expression::error;
                ast_node->type = new ExpressionType(ast_lhs->type->getPrototype(),ExpressionType::rvalue);
                break;
            }
            default:
                PANIC;
        }
        return (ast::Expression*)ast_node;
    }
    
    std::any TypeAnalyzer::visitDot(parseTree::expr::Dot *dot_node, TypeAnalyzerArgs args) {
        auto ast_lhs = any_cast<ast::Expression*>(visitExpression(dot_node->lhs,args));

        switch (ast_lhs->type->value_kind) {
            case ExpressionType::error:
                return ast::Expression::error;
            case ExpressionType::void_:
                Logger::error(dot_node->location,lang->msgInvalidExp());
                return ast::Expression::error;
        }

        args.dot_prefix = ast_lhs->type;
        auto ast_rhs = any_cast<ast::Expression*>(visitExpression(dot_node->rhs,args));

        ast::Expression *ret = nullptr;
        switch(ast_rhs->expression_kind){
            case ast::Expression::VFtn:{
                auto vftn = (ast::VFtnCall*)ast_rhs;
                vftn->ref = ast_lhs;
                ret = vftn;
                break;
            }
            case ast::Expression::Ftn:{
                auto ftn = (ast::FtnCall*)ast_rhs;
                ftn->ref = ast_lhs;
                ret = ftn;
                break;
            }
            case ast::Expression::Fld:{
                auto fld = (ast::Fld*)ast_rhs;
                fld->ref = ast_lhs;
                ret = fld;
                break;
            }
            case ast::Expression::Delegate:{
                auto delegate = (ast::Delegate*)ast_rhs;
                if(!delegate->is_static)delegate->ref = ast_lhs;
                ret = delegate;
                break;
            }
            case ast::Expression::SFtn:
            case ast::Expression::Element:
            case ast::Expression::Local:
            case ast::Expression::ArgUse:
            case ast::Expression::SFld:
            case ast::Expression::Assign:
            case ast::Expression::EnumMember:
                ret = ast_rhs;
                break;
            default:
                PANIC;
        }
        return ret;
    }

    std::any TypeAnalyzer::visitID(parseTree::expr::ID *id_node, TypeAnalyzerArgs args) {
        try{
            auto name = getID(id_node);
            Symbol *target = nullptr;
            if(!args.dot_prefix){
                target = args.domain->lookUp(name);
                if(!target) throw SymbolNotFound(id_node->location,nullptr,name);
            }
            else{
                auto domain = args.dot_prefix->getPrototype()->as<Domain*>();
                if(!(domain && (target = domain->find(name)))){
                    throw SymbolNotFound(id_node->location,args.dot_prefix->getPrototype(),name);
                }
            }

            check_access(id_node->location,target,args.domain,args.current_class_or_module);

            switch (target->getKind()) {
                case type::SymbolKind::Variable:{
                    if(args.dot_prefix)check_static_access(id_node->location,args.dot_prefix,target->isStatic());
                    auto variable = target->as<type::Variable*>();
                    auto type = new ExpressionType(variable->getPrototype(),ExpressionType::lvalue,target->isStatic());
                    switch (variable->getVariableKind()) {
                        case VariableKind::Local:       return (ast::Expression*)new ast::Local(variable,type);
                        case VariableKind::StaticField: return (ast::Expression*)new ast::SFld(variable,type);
                        case VariableKind::Field:       return (ast::Expression*)new ast::Fld(variable,type);
                    }
                }
                case type::SymbolKind::Parameter:{
                    if(args.dot_prefix)check_static_access(id_node->location,args.dot_prefix,target->isStatic());
                    auto variable = target->as<type::Variable*>();
                    auto type = new ExpressionType(variable->getPrototype(),ExpressionType::lvalue,target->isStatic());
                    return (ast::Expression*)new ast::Arg(variable,type);
                }
                case type::SymbolKind::Function:{
                    if(args.dot_prefix)check_static_access(id_node->location,args.dot_prefix,target->isStatic());
                    auto type = new ExpressionType(target, ExpressionType::path, target->isStatic());
                    return (ast::Expression*)new ast::Delegate(target->as<Function*>(),target->isStatic(),type);
                }
                case type::SymbolKind::Module:
                case type::SymbolKind::Class:
                case type::SymbolKind::Enum:{
                    if(args.dot_prefix)check_static_access(id_node->location,args.dot_prefix,true);
                    return (ast::Expression*)new ast::TmpPath(new ExpressionType(target,ExpressionType::path,true));
                }
                case type::SymbolKind::EnumMember:{
                    return (ast::Expression*)new ast::EnumMember(target->as<type::EnumMember*>(),
                            new ExpressionType(target->getParent(),ExpressionType::rvalue,true));
                }
                default:
                    PANIC;
            }
        }
        catch (SymbolNotFound& e){
            if(e.search_domain){
                Logger::error(e.location, lang->fmtObjectNotFound(e.search_domain->mangling('.')+'.'+e.search_name));
            }
            else{
                Logger::error(e.location, lang->fmtObjectNotFound(e.search_name));
            }
            return ast::Expression::error;
        }
    }

    std::any TypeAnalyzer::visitAssign(parseTree::expr::Assign *assign_node, TypeAnalyzerArgs args) {
        auto ast_lhs = any_cast<ast::Expression*>(visitExpression(assign_node->lhs,args));
        auto ast_rhs = any_cast<ast::Expression*>(visitExpression(assign_node->rhs,args));
        auto ast_node = new ast::Assign(ast_lhs,ast_rhs);

        if(ast_lhs->type->value_kind == ExpressionType::error || ast_rhs->type->value_kind == ExpressionType::error)
            return ast::Expression::error;

        if(ast_lhs->type->value_kind != ExpressionType::lvalue){
            Logger::error(assign_node->location, lang->msgAssignmentRequireLvalue());
        }

        if(!ast_lhs->type->getPrototype()->equal(ast_rhs->type->getPrototype())){
            try_implicit_conversion(assign_node->location,&ast_node->rhs,ast_lhs->type->getPrototype(),args.context);
        }

        ast_node->type = ast_lhs->type;
        return (ast::Expression*)ast_node;
    }

    std::any TypeAnalyzer::visitIndex(parseTree::expr::Index *index_node, TypeAnalyzerArgs args) {
        auto ast_target = any_cast<ast::Expression*>(visitExpression(index_node->target,args));
        auto ast_value = any_cast<ast::Expression*>(visitExpression(index_node->value,args));
        auto ast_node = new ast::ArrayElement(ast_target,ast_value);

        switch (ast_target->type->value_kind) {
            case ExpressionType::error:
                return ast::Expression::error;
            case ExpressionType::void_:
                Logger::error(index_node->location, lang->msgInvalidExp());
                return ast::Expression::error;
        }

        Prototype *dst_prototype = nullptr,*ret_prototype = nullptr;
        switch(ast_target->type->getPrototype()->getKind()){
            case SymbolKind::Array:
                dst_prototype = args.context->getBuiltIn().getPrimitive(vm::Data::i32);
                ret_prototype = ast_target->type->getPrototype()->as<Array*>()->getElementPrototype();
                break;
            case SymbolKind::Class:{
                //TODO index operator overload
                break;
            }
        }

        if(!dst_prototype->equal(ast_value->type->getPrototype())){
            if(args.context->getConversionRules().isImplicitCastRuleExist(ast_value->type->getPrototype(),dst_prototype)){
                Logger::warning(index_node->value->location, lang->fmtImplicitCvtFromAToB(ast_value->type->getPrototype()->getName(),dst_prototype->getName()));
                args.context->getConversionRules().insertCastAST(dst_prototype,&ast_node->offset);
            }
            else{
                Logger::error(index_node->location, lang->fmtCannotImplicitCvtAToB(ast_value->type->getPrototype()->mangling('.'),"'Integer'"));
            }
        }

        ast_node->type = new ExpressionType(ret_prototype,ExpressionType::lvalue);
        return (ast::Expression*)ast_node;
    }

    std::any TypeAnalyzer::visitCallee(parseTree::expr::Callee *callee_node, TypeAnalyzerArgs args) {
        auto ast_path = any_cast<ast::Expression*>(visitExpression(callee_node->name,args));

        auto func = ast_path->type->getPrototype()->as<type::Function*>();
        if(!func){
            Logger::error(callee_node->name->location, lang->fmtNotCallableTarget(ast_path->type->getPrototype()->getName()));
            return ast::Expression::error;
        }

        ast::Call *ast_node = nullptr;

        switch (func->getFunctionKind()) {
            case FunctionKind::InterfaceFunction: ast_node = new ast::VFtnCall; break;
            case FunctionKind::Constructor: ast_node = new ast::SFtnCall; break;
            case FunctionKind::UserFunction:{
                auto user_ftn = func->as<UserFunction*>();
                switch(user_ftn->getFunctionFlag()){
                    case FunctionFlag::Method: ast_node = new ast::FtnCall; break;
                    case FunctionFlag::Static: ast_node = new ast::SFtnCall; break;
                    case FunctionFlag::Virtual:
                    case FunctionFlag::Override:
                        ast_node = new ast::VFtnCall;
                        break;
                }
                break;
            }
            case FunctionKind::External: ast_node = new ast::ExtCall; break;
            case FunctionKind::Operator: PANIC;
        }

        check_callee(ast_node,callee_node->location,callee_node->argument,func,args);

        auto ret = func->getRetSignature();
        if(ret){
            ast_node->type = new ExpressionType(ret->as<type::Prototype*>(),ExpressionType::rvalue);
        }
        else{
            ast_node->type = ExpressionType::Void;
        }

        return (ast::Expression*)ast_node;
    }


    std::any TypeAnalyzer::visitNew(parseTree::expr::New *new_node, TypeAnalyzerArgs args) {
        auto prototype = any_cast<Prototype*>(visitAnnotation(new_node->annotation,args));
        auto cls = prototype->as<type::Class*>();

        if(!cls){
            Logger::error(new_node->location,lang->msgNotClass());
            return ast::Expression::error;
        }

        if(cls->isAbstract()){
            Logger::error(new_node->location,lang->msgCannotInitAbstractCls());
        }

        auto constructor = cls->getConstructor();

        auto ast_node = new ast::New(cls,constructor);

        if(!constructor){
            Logger::error(new_node->location,lang->fmtCtorUndefined(cls->mangling('.')));
            return ast::Expression::error;
        }

        check_callee(ast_node,new_node->location,new_node->argument,constructor,args);

        ast_node->type = new ExpressionType(cls,ExpressionType::rvalue);
        return (ast::Expression*)ast_node;
    }

    std::any TypeAnalyzer::visitExprStmt(parseTree::stmt::ExprStmt *expr_stmt_node, TypeAnalyzerArgs args) {
        args.dot_prefix = nullptr;
        ast::Statement *statement = new ast::ExprStmt(any_cast<ast::Expression*>(visitExpression(expr_stmt_node->expr,args)));
        return statement;
    }

    std::any TypeAnalyzer::visitParentheses(parseTree::expr::Parentheses *parentheses_node, TypeAnalyzerArgs args) {
        args.dot_prefix = nullptr;
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
                Logger::error(code_location,lang->fmtIsPrivate(target->mangling('.')));
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
                Logger::error(code_location,lang->fmtIsProtected(target->mangling('.')));
                break;
            }
        }
    }
    
    Prototype *TypeAnalyzer::check_binary_op_valid(Location *code,ConversionRules &rules,
                                                   Prototype *lhs,Prototype *rhs,
                                                   parseTree::Expression *lhs_node,parseTree::Expression *rhs_node,
                                                   ast::Expression **ast_lhs,ast::Expression **ast_rhs) {
        if(!rhs->equal(lhs)){
            auto result = rules.getImplicitPromotionRule(lhs,rhs);
            if(result.has_value()){
                auto rule = result.value();
                auto result_type = rule.second;
                if(!lhs->equal(result_type)) {
                    Logger::warning(lhs_node->location,lang->fmtImplicitCvtFromAToB(lhs->getName(),result_type->getName()));
                    rules.insertCastAST(result_type, ast_lhs);
                }
                if(!rhs->equal(result_type)) {
                    Logger::warning(rhs_node->location,lang->fmtImplicitCvtFromAToB(rhs->getName(),result_type->getName()));
                    rules.insertCastAST(result_type, ast_rhs);
                }
                return result_type;
            }
            else{
                Logger::error(code,lang->fmtBinaryOpInvalid(lhs->getName(),rhs->getName()));
                return nullptr;
            }
        }
        else{
            return lhs;
        }
    }

    std::any TypeAnalyzer::visitAnnotation(parseTree::Annotation *annotation_node, TypeAnalyzerArgs args) {
        auto iter = (*annotation_node).unit;
        args.need_lookup = true;
        args.dot_prefix = new ExpressionType(args.domain,ExpressionType::rvalue,false);
        auto symbol = visitAnnotationUnit(iter,args);
        iter = iter->next_sibling;
        args.dot_prefix->symbol = any_cast<Symbol*>(symbol);

        args.need_lookup = false;
        while(iter!=nullptr){
            symbol = visitAnnotationUnit(iter,args);
            args.dot_prefix->symbol = any_cast<Symbol*>(symbol);
            iter=iter->next_sibling;
        }

        auto element = args.dot_prefix->getPrototype()->as<Prototype*>();
        auto ret_prototype = element;

        if((*annotation_node).array_size){
            ret_prototype = new type::Array(ret_prototype, getDigit((*annotation_node).array_size));

            if(element->getKind() == SymbolKind::Record)
                args.context->byteLengthDependencies.addDependent(ret_prototype->as<Domain*>(),element->as<Domain*>());
        }

        return ret_prototype;
    }

    std::any TypeAnalyzer::visitAnnotationUnit(parseTree::AnnotationUnit *unit_node, TypeAnalyzerArgs args) {
        auto name = getID((*unit_node).name);
        Symbol *symbol = nullptr;
        if(args.need_lookup){
            symbol = args.dot_prefix->symbol->as<Domain*>()->lookUp(name);
        }
        else{
            symbol = args.dot_prefix->symbol->as<Domain*>()->find(name);
        }

        if(!symbol){
            throw SymbolNotFound((*unit_node).name->location,args.dot_prefix->symbol,name);
        }
        return symbol;
    }

    void TypeAnalyzer::check_static_access(Location *code_location, ExpressionType *lhs, bool is_rhs_static) {

        if(lhs->is_static xor is_rhs_static){
            auto lhs_state = lhs->is_static ? "Static" : "Non-static";
            auto rhs_state = is_rhs_static ? "Static" : "Non-static";
            Logger::error(code_location,lang->fmtStaticAccessInvalid(lhs_state,rhs_state));
        }
    }

    std::any TypeAnalyzer::visitColon(parseTree::expr::Colon *colon_node, TypeAnalyzerArgs args) {
        if(colon_node->lhs->expression_kind != parseTree::expr::Expression::ID_){
            Logger::error(colon_node->location,lang->msgParamInitialExpectedIDInLhs());
            return ast::Expression::error;
        }
        if(!args.checking_function){
            Logger::error(colon_node->location,lang->msgParamInitialNotAllowed());
            return ast::Expression::error;
        }
        else{
            return visitExpression(colon_node->rhs,args);
        }
    }

    void TypeAnalyzer::check_callee(ast::Call *ast_node, Location *location, parseTree::expr::Argument *argument, type::Function *target,
                                    TypeAnalyzerArgs args) {
        ast_node->function = target;

        std::size_t args_count = 0;
        auto params_count = target->getArgsSignature().size();
        auto arg = argument;
        bool opt_flag = false;

        while(arg){
            if(arg->expr->expression_kind != parseTree::expr::Expression::colon_){
                if(opt_flag){
                    Logger::error(arg->location,lang->msgRegularAppearAfterOpt());
                }
                args_count++;
            }
            else opt_flag = true;
            arg = arg->next_sibling;
        }

        if(target->getParamArray() && args_count > params_count){
            args_count = params_count;
        }

        if(args_count > params_count){
            Logger::error(location,lang->fmtFtnCallTooManyArg(params_count,args_count));
        }
        else if(args_count < params_count){
            Logger::error(location,lang->fmtFtnCallTooFewArg(params_count,args_count));
        }
        else{
            args.checking_function = target;
            args.checking_arg_index = 0;
            ast::Argument *tail = nullptr;
            auto insert_ast_argument = [&](ast::Argument *ast_argument){
                if(tail == nullptr) tail = ast_node->argument = ast_argument;
                else{
                    tail->next_sibling = ast_argument;
                    ast_argument->prv_sibling = tail;
                    tail = ast_argument;
                }
            };

            std::map<string,parseTree::Argument*> optional_arguments;
            FOR_EACH(iter,argument){
                if(iter->expr->expression_kind == parseTree::Expression::colon_){
                    // optional parameter initialization
                    auto colon = (parseTree::Colon*)iter->expr;
                    optional_arguments.insert({getID((parseTree::ID*)colon->lhs),iter});
                }
                else{
                    auto ast_argument = (ast::Argument*)any_cast<ast::Expression*>(visitArg(iter,args));
                    insert_ast_argument(ast_argument);
                    args.checking_arg_index++;
                }
            }

            for(auto opt_parameter : target->getArgsOptions()){
                auto used_opt = optional_arguments.find(opt_parameter->getName());
                if(used_opt != optional_arguments.end()){
                    insert_ast_argument((ast::Argument*)any_cast<ast::Expression*>(visitArg(used_opt->second,args)));
                }
                else{
                    insert_ast_argument(opt_parameter->getDefaultArgument());
                }
            }

        }

    }

    std::any TypeAnalyzer::visitType(parseTree::Type *ty_node, TypeAnalyzerArgs args) {
        return (ast::Member*)new ast::Type(ty_node->type_symbol);
    }

    std::any TypeAnalyzer::visitEnum(parseTree::Enum *em_node, TypeAnalyzerArgs args) {
        return (ast::Member*)new ast::Enum(em_node->enum_symbol);
    }

    std::any TypeAnalyzer::visitDim(parseTree::Dim *dim_node, TypeAnalyzerArgs args) {
       // return (ast::Member*)new ast::Dim(dim_node->);
        auto ast_node = new ast::Dim;
        auto tail = ast_node->variable;
        FOR_EACH(iter,dim_node->variable){
            auto ast_var = any_cast<ast::Variable*>(visitVariable(iter,args));
            if(tail == nullptr){
                tail = ast_node->variable = ast_var;
            }
            else{
                tail->next_sibling = ast_var;
                ast_var->prv_sibling = tail;
                tail = ast_var;
            }
        }
        return (ast::Member*)ast_node;
    }

    std::any TypeAnalyzer::visitVariable(parseTree::Variable *var_node, TypeAnalyzerArgs args) {
        if(var_node->initial){
            return new ast::Variable(var_node->variable_symbol,any_cast<ast::Expression*>(visitExpression(var_node->initial,args)));
        }
        else{
            return new ast::Variable(var_node->variable_symbol,nullptr);
        }
    }

    std::any TypeAnalyzer::visitParameter(parseTree::Parameter *param_node, TypeAnalyzerArgs args) {
        if(param_node->is_optional){
            if(!param_node->initial){
                Logger::error(param_node->location,lang->msgOptionalNeedDefaultValue());
            }
            auto ast_exp = any_cast<ast::Expression*>(visitExpression(param_node->initial,args));
            auto ast_node = new ast::Argument;
            ast_node->expr = ast_exp;
            ast_node->parameter = param_node->parameter_symbol;
            ast_node->type = ast_exp->type;
            ast_node->byval = param_node->parameter_symbol->isByval();
            parameterPassingResolution(param_node->location,parseTree::Argument::PassKind::undefined,ast_node,nullptr,args.context);
            param_node->parameter_symbol->setDefaultArgument(ast_node);
            return (ast::Expression*)ast_node;
        }
        return {};
    }

    std::any TypeAnalyzer::visitInterface(parseTree::Interface *interface_node, TypeAnalyzerArgs args) {
        return (ast::Member*)new ast::Interface(interface_node->interface_symbol);
    }

    std::any TypeAnalyzer::visitExternal(parseTree::External *ext_node, TypeAnalyzerArgs args) {
        return (ast::Member*)new ast::External(ext_node->function_symbol);
    }

    std::any TypeAnalyzer::visitCast(parseTree::expr::Cast *cast_node, TypeAnalyzerArgs args) {
        //todo
        PANIC;
    }


}
