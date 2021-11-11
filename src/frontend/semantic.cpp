//
// Created by yorkin on 11/1/21.
//

#include "semantic.h"
#include "nullSafe.h"
#include "logger.h"

using namespace std;
using namespace evoBasic::type;
namespace evoBasic{

    void Semantic::collectSymbol(AST *ast, std::shared_ptr<Context> context) {
        SymbolCollector collector;
        SymbolCollectorArgs args;
        args.domain = context->getGlobal();
        collector.visitGlobal(ast,args);
    }

    void Semantic::collectDetail(AST *ast, std::shared_ptr<Context> context) {
        DetailCollector collector;
        BaseArgs args;
        args.domain = context->getGlobal();
        args.context = context;
        collector.visitGlobal(ast,args);
    }

    void Semantic::typeCheck(AST *ast, std::shared_ptr<Context> context) {
        TypeAnalyzer analyzer;
        BaseArgs args;
        args.domain = context->getGlobal();
        args.context = context;
        analyzer.visitGlobal(ast,args);
    }

    bool Semantic::solveTypeInferenceDependencies(std::shared_ptr<Context> context) {
        return false;
    }

    bool Semantic::solveByteLengthDependencies(std::shared_ptr<Context> context) {
        if(context->byteLengthDependencies.solve()){

        }


        return false;
    }

    bool is_name_valid(const string& name, const Location *location, const shared_ptr<type::Domain>& domain){
        NotNull(location);
        if(domain->find(name)){
            Logger::error(location,"Naming conflict in current scope");
            return false;
        }
        return true;
    }

    string getID(ast::ID *id) {
        NotNull(id);
        string ret = id->lexeme;
        transform(ret.begin(),ret.end(),ret.begin(),[](char c){
            return tolower(c);
        });
        return ret;
    }

    int getDigit(ast::expr::literal::Digit *digit){
        NotNull(digit);
        return digit->value;
    }

    string getString(ast::expr::literal::String *str){
        return str->value;
    }


    std::any SymbolCollector::visitGlobal(evoBasic::ast::Global *global, evoBasic::SymbolCollectorArgs args) {
        NotNull(global);
        args.domain->setLocation(global->location);
        for(const auto& member:global->member_list){
            auto object = visitMember(member,args);
            if(!object.has_value())continue;
            auto symbol = any_cast<shared_ptr<type::Symbol>>(object);
            if(is_name_valid(symbol->getName(), symbol->getLocation(), args.domain)){
                args.domain->add(symbol);
            }
        }
        return nullptr;
    }

    std::any SymbolCollector::visitModule(ast::Module *mod_node, SymbolCollectorArgs args) {
        NotNull(mod_node);
        auto mod = make_shared<type::Module>();
        auto name = any_cast<string>(visitID(mod_node->name,args));
        mod->setName(name);
        mod->setAccessFlag(mod_node->access);
        mod->setLocation(mod_node->location);
        args.domain = mod;
        for(const auto& member:mod_node->member_list){
            auto object = visitMember(member,args);
            if(!object.has_value())continue;
            auto symbol = any_cast<shared_ptr<type::Symbol>>(object);
            args.domain->add(symbol);
        }
        return mod->as_shared<type::Symbol>();
    }

    std::any SymbolCollector::visitClass(ast::Class *cls_node, SymbolCollectorArgs args) {
        NotNull(cls_node);
        auto cls = make_shared<type::Class>();
        cls->setName(any_cast<string>(visitID(cls_node->name,args)));
        cls->setAccessFlag(cls_node->access);
        cls->setLocation(cls_node->location);
        args.domain = cls;
        for(const auto& member:cls_node->member_list){
            auto object = visitMember(member,args);
            if(!object.has_value())continue;
            auto symbol = any_cast<shared_ptr<type::Symbol>>(object);
            args.domain->add(symbol);
        }
        return cls->as_shared<type::Symbol>();
    }

    std::any SymbolCollector::visitEnum(ast::Enum *em_node, SymbolCollectorArgs args) {
        NotNull(em_node);
        auto em = make_shared<type::Enumeration>();
        em->setName(any_cast<string>(visitID(em_node->name,args)));
        em->setAccessFlag(em_node->access);
        em->setLocation(em_node->location);
        return em->as_shared<type::Symbol>();
    }

    std::any SymbolCollector::visitType(ast::Type *ty_node, SymbolCollectorArgs args) {
        NotNull(ty_node);
        auto ty = make_shared<type::Record>();
        ty->setName(any_cast<string>(visitID(ty_node->name,args)));
        ty->setAccessFlag(ty_node->access);
        ty->setLocation(ty_node->location);
        return ty->as_shared<type::Symbol>();
    }

    std::any SymbolCollector::visitDim(ast::Dim *dim, SymbolCollectorArgs args) {
        NotNull(dim);
        shared_ptr<type::Symbol> symbol;
        for(const auto& var:dim->variable_list){
            symbol = any_cast<shared_ptr<type::Symbol>>(visitVariable(var,args));
            symbol->as_shared<Variable>()->setGlobal();
            symbol->setAccessFlag(dim->access);
            if(&var == &dim->variable_list.back())break;
            if(is_name_valid(symbol->getName(), var->location, args.domain)){
                args.domain->add(symbol);
            }
        }
        return symbol;
    }

    std::any SymbolCollector::visitVariable(ast::Variable *var, SymbolCollectorArgs args) {
        NotNull(var);
        auto field = make_shared<type::Variable>();
        auto name = any_cast<string>(visitID(var->name,args));
        field->setName(name);
        field->setLocation(var->location);
        return field->as_shared<type::Symbol>();
    }

    std::any SymbolCollector::visitID(ast::ID *id, SymbolCollectorArgs args) {
        NotNull(id);
        return id->lexeme;
    }

    std::any SymbolCollector::visitMember(ast::Member *member_node, SymbolCollectorArgs args) {
        switch (member_node->member_kind) {
            case ast::Member::class_:    return visitClass((ast::Class*)member_node,args);
            case ast::Member::module_:   return visitModule((ast::Module*)member_node,args);
            case ast::Member::type_:     return visitType((ast::Type*)member_node,args);
            case ast::Member::enum_:     return visitEnum((ast::Enum*)member_node,args);
            case ast::Member::dim_:      return visitDim((ast::Dim*)member_node,args);
        }
        return {};
    }




    //return shared_ptr<Prototype>
    std::any BaseVisitor::visitAnnotation(ast::Annotation *anno_node, BaseArgs args) {
        shared_ptr<type::Symbol> ptr;
        for(auto& unit_node:anno_node->unit_list){
            auto name = any_cast<string>(visitAnnotationUnit(unit_node,args));
            if(&unit_node == &anno_node->unit_list.front()){
                ptr = args.domain->lookUp(name);
            }
            else{
                if(!ptr){
                    Logger::error(anno_node->location,"Cannot find Object");
                    break;
                }
                auto domain = ptr->as_shared<type::Domain>();
                if(!domain){
                    Logger::error(unit_node->location,format()<<"'"<<name<<"' is not a Class,Enum,Type,Or Module");
                    break;
                }
                ptr = domain->find(name);
            }
        }

        auto prototype = ptr->as_shared<type::Prototype>();
        if(!prototype){
            Logger::error(anno_node->location,"Type expression invalid");
            return make_shared<type::Error>();
        }
        if(anno_node->is_array){
            prototype = make_shared<type::Array>(prototype);
        }
        return prototype;
    }

    std::any BaseVisitor::visitAnnotationUnit(ast::AnnotationUnit *unit_node, BaseArgs args) {
        NotNull(unit_node);
        return getID(unit_node->name);
    }




    std::any DetailCollector::visitGlobal(ast::Global *global, BaseArgs args) {
        NotNull(global);
        for(const auto& member:global->member_list)
            visitMember(member,args);
        return nullptr;
    }

    std::any DetailCollector::visitModule(ast::Module *mod_node, BaseArgs args) {
        NotNull(mod_node);
        auto name = getID(mod_node->name);
        auto mod = args.domain->find(name);
        NotNull(mod.get());
        args.domain = mod->as_shared<type::Domain>();
        for(const auto& member:mod_node->member_list)
            visitMember(member,args);
        return nullptr;
    }

    std::any DetailCollector::visitClass(ast::Class *cls_node, BaseArgs args) {
        NotNull(cls_node);
        auto name = getID(cls_node->name);
        auto cls = args.domain->find(name);
        NotNull(cls.get());
        args.domain = cls->as_shared<type::Domain>();
        for(const auto& member:cls_node->member_list)
            visitMember(member,args);
        return nullptr;
    }

    std::any DetailCollector::visitEnum(ast::Enum *em_node, BaseArgs args) {
        NotNull(em_node);
        auto name = getID(em_node->name);
        auto em = args.domain->find(name)->as_shared<type::Enumeration>();
        NotNull(em.get());
        int index = 0;
        for(auto& child:em_node->member_list){
            NotNull(child.first);
            if(child.second != nullptr){
                index = getDigit(child.second);
            }
            auto member_name = getID(child.first);
            if(is_name_valid(member_name,child.first->location,args.domain)){
                auto member = make_shared<type::EnumMember>(index);
                member->setName(member_name);
                member->setLocation(child.first->location);
                em->add(member);
                index++;
            }
        }
        return nullptr;
    }

    std::any DetailCollector::visitType(ast::Type *ty_node, BaseArgs args) {
        NotNull(ty_node);
        auto name = getID(ty_node->name);
        auto ty = args.domain->find(name)->as_shared<type::Record>();
        NotNull(ty.get());
        for(auto& p:ty_node->member_list){
            auto field_name = getID(p.first);
            if(is_name_valid(field_name,p.first->location,args.domain)){
                auto field = make_shared<type::Variable>();
                field->setName(field_name);
                auto prototype = any_cast<shared_ptr<type::Prototype>>(visitAnnotation(p.second,args));
                if(prototype->getKind() == DeclarationEnum::Type){

                }
                field->setPrototype(prototype);
                ty->add(field);
            }
        }
        return nullptr;
    }

    std::any DetailCollector::visitDim(ast::Dim *dim_node, BaseArgs args) {
        for(auto& variable_node:dim_node->variable_list){
            visitVariable(variable_node,args);
        }
        return nullptr;
    }

    std::any DetailCollector::visitVariable(ast::Variable *var_node, BaseArgs args) {
        auto name = getID(var_node->name);
        auto var = args.domain->find(name)->as_shared<type::Variable>();
        NotNull(var.get());
        auto prototype = any_cast<shared_ptr<type::Prototype>>(visitAnnotation(var_node->annotation,args));
        var->setPrototype(prototype);
        return nullptr;
    }


    std::any DetailCollector::visitFunction(ast::Function *func_node, BaseArgs args) {
        auto name = getID(func_node->name);
        if(is_name_valid(name,func_node->name->location,args.domain)){
            auto func = make_shared<type::UserFunction>(func_node->method_flag,func_node);
            func->setLocation(func_node->name->location);
            func->setName(name);
            args.domain->add(func);
            if(func_node->return_type){
                auto prototype = any_cast<shared_ptr<type::Prototype>>(visitAnnotation(func_node->return_type,args));
                func->setRetSignature(prototype);
            }

            args.domain = func;
            for(auto& param_node:func_node->parameter_list){
                visitParameter(param_node,args);
            }
        }
        return nullptr;
    }

    std::any DetailCollector::visitExternal(ast::External *ext_node, BaseArgs args) {
        auto name = getID(ext_node->name);
        if(is_name_valid(name,ext_node->name->location,args.domain)){
            auto lib = getString(ext_node->lib);
            //auto alias = getString(ext_node->alias); TODO alias
            auto func = make_shared<type::ExternalFunction>(lib,name);
            func->setLocation(ext_node->name->location);
            func->setName(name);
            args.domain->add(func);
            if(ext_node->return_annotation){
                auto prototype = any_cast<shared_ptr<type::Prototype>>(visitAnnotation(ext_node->return_annotation,args));
                func->setRetSignature(prototype);
            }
            args.domain = func;
            for(auto& param_node:ext_node->parameter_list){
                visitParameter(param_node,args);
            }
        }
        return nullptr;
    }

    std::any DetailCollector::visitParameter(ast::Parameter *param_node, BaseArgs args) {
        auto name = getID(param_node->name);
        auto prototype = any_cast<shared_ptr<type::Prototype>>(visitAnnotation(param_node->annotation,args));
        NotNull(prototype.get());
        auto arg = make_shared<type::Argument>(name,prototype,param_node->is_byval,param_node->is_optional);
        if(is_name_valid(name,param_node->name->location,args.domain)){
            args.domain->add(arg);
        }
        return nullptr;
    }

    std::any DetailCollector::visitMember(ast::Member *member_node, BaseArgs args) {
        switch (member_node->member_kind) {
            case ast::Member::function_: return visitFunction((ast::Function*)member_node,args);
            case ast::Member::class_:    return visitClass((ast::Class*)member_node,args);
            case ast::Member::module_:   return visitModule((ast::Module*)member_node,args);
            case ast::Member::type_:     return visitType((ast::Type*)member_node,args);
            case ast::Member::enum_:     return visitEnum((ast::Enum*)member_node,args);
            case ast::Member::dim_:      return visitDim((ast::Dim*)member_node,args);
            case ast::Member::external_: return visitExternal((ast::External*)member_node,args);
            //case ast::Member::operator_: return visitOperator((ast::Operator*)member_node,args);
            //case ast::Member::init_:     return visitInit((ast::Init*)member_node,args);
        }
        return {};
    }




    ExpressionType *ExpressionType::Error = new ExpressionType(make_shared<type::Error>(),error);


    std::any TypeAnalyzer::visitDigit(ast::expr::literal::Digit *digit_node, BaseArgs args) {
        auto prototype = args.context->getBuiltIn().getPrimitive(vm::Data::i32)->as_shared<type::Class>();
        return digit_node->type = new ExpressionType(prototype,ExpressionType::rvalue);
    }

    std::any TypeAnalyzer::visitDecimal(ast::expr::literal::Decimal *decimal, BaseArgs args) {
        auto prototype = args.context->getBuiltIn().getPrimitive(vm::Data::f64)->as_shared<type::Class>();
        return decimal->type = new ExpressionType(prototype,ExpressionType::rvalue);
    }

    std::any TypeAnalyzer::visitBoolean(ast::expr::literal::Boolean *bl_node, BaseArgs args) {
        auto prototype = args.context->getBuiltIn().getPrimitive(vm::Data::boolean)->as_shared<type::Class>();
        return bl_node->type = new ExpressionType(prototype,ExpressionType::rvalue);
    }

    std::any TypeAnalyzer::visitChar(ast::expr::literal::Char *ch_node, BaseArgs args) {
        auto prototype = args.context->getBuiltIn().getPrimitive(vm::Data::i8)->as_shared<type::Class>();
        return ch_node->type = new ExpressionType(prototype,ExpressionType::rvalue);
    }

    std::any TypeAnalyzer::visitString(ast::expr::literal::String *str_node, BaseArgs args) {
        throw "unimpl";//TODO String supprot
    }


    std::any TypeAnalyzer::visitCallee(ast::expr::Callee *callee_node, BaseArgs args) {
        auto name = getID(callee_node->name);
        shared_ptr<type::Symbol> target;
        if(args.need_lookup){
            target = args.in_terminal_list->lookUp(name);
        }
        else{
            target = args.in_terminal_list->find(name);
        }

        if(!target){
            Logger::error(callee_node->location,"object not found");
            return ExpressionType::Error;
        }

        //TODO process generic args
        ExpressionType::ValueKind value_kind = ExpressionType::lvalue;
        auto func = target->as_shared<type::Function>();
        if(callee_node->args){
            if(func){
                args.in_terminal_list = func;
                visitArgsList(callee_node->args,args);
                target = func->getRetSignature();
                value_kind = ExpressionType::rvalue;
            }
            else{
                Logger::error(callee_node->name->location,format()<<"'"<<name<<"' is not a callable target");
                return ExpressionType::Error;
            }
        }

        if(callee_node->index_arg){
            if(target->getKind() != type::DeclarationEnum::Array){
                Logger::error(callee_node->location,format()<<"type '"<<target->getName()<<"' is not Array");
                return ExpressionType::Error;
            }
            target = target->as_shared<type::Array>()->getElementPrototype();
        }

        if(target->getKind() == type::DeclarationEnum::EnumMember){
            target = target->getParent().lock();
            value_kind = ExpressionType::rvalue;
        }
        else if(target->getKind() == type::DeclarationEnum::Variable || target->getKind() == type::DeclarationEnum::Argument){
            target = target->as_shared<type::Variable>()->getPrototype();
        }
        else if(callee_node->index_arg == nullptr){
            value_kind = ExpressionType::rvalue;
        }

        return callee_node->type = new ExpressionType(target->as_shared<type::Prototype>(),value_kind);
    }

    std::any TypeAnalyzer::visitArgsList(ast::expr::ArgsList *args_list_node, BaseArgs args) {
        auto func = args.in_terminal_list->as_shared<evoBasic::type::Function>();
        NotNull(func.get());
        int i=0;
        for(auto &arg : args_list_node->arg_list){
            args.checking_args_index = i;
            visitArg(arg,args);
            i++;
        }
        auto args_count = args_list_node->arg_list.size();
        auto params_count = func->getArgsSignature().size();
        if(args_count > params_count){
            Logger::error(args_list_node->location,format()<<"too many arguments to function call, expected "
                                                    <<params_count<<", have "<<args_count);
        }
        else if(args_count < params_count){
            Logger::error(args_list_node->location,format()<<"too few arguments to function call, expected "
                                                     <<params_count<<", have "<<args_count);
        }

        return nullptr;
    }

    std::any TypeAnalyzer::visitArg(ast::expr::Arg *arg_node, BaseArgs args) {
        auto func = args.in_terminal_list->as_shared<evoBasic::type::Function>();
        auto param =  func->getArgsSignature()[args.checking_args_index];
        auto arg_type = any_cast<ExpressionType*>(visitExpression(arg_node->expr,args));
        if(arg_type->value_kind == ExpressionType::error)return {};

        auto arg_prototype = arg_type->prototype;
        auto param_prototype = param->getPrototype();

        auto report_type_error = [&](){
            Logger::error(arg_node->location,format()<<"parameter type is '"<<param_prototype->getName()
                                                     <<"' but the argument type is '"<<arg_prototype->getName()<<"'");
        };

        auto try_implicit_conversion = [&]()->bool{
            if(args.context->getConversionRules().isImplicitCastRuleExist(arg_prototype,param_prototype)){
                Logger::warning(arg_node->location,format()<<"implicit conversion from '"<<arg_prototype->getName()
                                                           <<"' to '"<<param_prototype->getName()<<"'");
                args.context->getConversionRules().insertCastAST(param_prototype,&arg_node->expr);
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
            switch (arg_node->pass_kind) {
                case ast::expr::Arg::undefined:
                case ast::expr::Arg::byval:
                    if(!param->getPrototype()->equal(arg_type->prototype)){
                        if(!try_implicit_conversion())report_type_error();
                    }
                    break;
                case ast::expr::Arg::byref:
                    Logger::error(arg_node->location,"require ByVal but declared ByRef");
                    break;
            }
        }
        else{
            switch (arg_node->pass_kind) {
                case ast::expr::Arg::byval:
                    if(!param->getPrototype()->equal(arg_type->prototype)){
                        if(!try_implicit_conversion())report_type_error();
                    }
                    break;
                case ast::expr::Arg::byref:
                    if(!param->getPrototype()->equal(arg_type->prototype)){
                        report_type_error();
                    }
                    break;
                case ast::expr::Arg::undefined:
                    if(arg_type->value_kind != ExpressionType::lvalue){
                        Logger::error(arg_node->location,format()<<"can not pass a temporary value ByRef."
                                                                 <<"Change parameter to Byval or explicit declare 'ByVal' here.\n"
                                                                 <<"Syntax: exampleFunction(Byval <Expression>) ");
                    }
                    else if(!param->getPrototype()->equal(arg_type->prototype)){
                        report_type_error();
                    }
                    break;
            }
        }
        arg_node->expr->type = arg_type;
        return {};
    }

    std::any TypeAnalyzer::visitLink(ast::expr::Link *link_node, BaseArgs args) {
        using sharedCls = shared_ptr<type::Class>;
        using sharedSym = shared_ptr<type::Symbol>;
        using namespace ast::expr::literal;
        using namespace ast::expr;
        using namespace ast;
        args.in_terminal_list = args.domain;
        ExpressionType *type = nullptr;
        for(auto& terminal:link_node->terminal_list){
            if(&terminal == &link_node->terminal_list.front()){
                switch (terminal->terminal_kind) {
                    case ast::expr::Terminal::boolean_:
                    case ast::expr::Terminal::digit_:
                    case ast::expr::Terminal::decimal_:
                    case ast::expr::Terminal::string_:
                    case ast::expr::Terminal::char_:
                    case ast::expr::Terminal::parentheses_:
                        type = any_cast<ExpressionType*>(visitTerminal(terminal,args));
                        break;
                    case ast::expr::Terminal::callee_:
                        args.need_lookup = true;
                        type = any_cast<ExpressionType*>(visitCallee((Callee*)terminal,args));
                        break;
                }
            }
            else{
                auto domain = type->prototype->as_shared<type::Domain>();
                if(domain){
                    args.in_terminal_list = domain;
                    args.need_lookup = false;
                    type = any_cast<ExpressionType*>(visitCallee((Callee*)terminal,args));
                }
                else{
                    Logger::error(link_node->location,"invalid expression");
                    return ExpressionType::Error;
                }
            }

            if(type->value_kind == ExpressionType::error)return ExpressionType::Error;
        }

        return link_node->type = type;
    }

    std::any TypeAnalyzer::visitBinary(ast::expr::Binary *logic_node, BaseArgs args) {
        auto lhs_type = any_cast<ExpressionType*>(visitExpression(logic_node->lhs,args));
        auto rhs_type = any_cast<ExpressionType*>(visitExpression(logic_node->rhs,args));
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
                    return ExpressionType::error;
                }
                return logic_node->type = new ExpressionType(boolean_prototype,ExpressionType::rvalue);
                break;
            case ast::expr::Binary::Not:
                if(!is_boolean(rhs_type,logic_node->rhs->location)){
                    return ExpressionType::error;
                }
                return logic_node->type = new ExpressionType(boolean_prototype,ExpressionType::rvalue);
                break;
            case ast::expr::Binary::EQ:
            case ast::expr::Binary::NE:
            case ast::expr::Binary::GE:
            case ast::expr::Binary::LE:
            case ast::expr::Binary::GT:
            case ast::expr::Binary::LT:
                if(!is_binary_op_vaild(lhs_type, rhs_type, &(logic_node->lhs), &(logic_node->rhs)))return ExpressionType::error;
                return logic_node->type = new ExpressionType(boolean_prototype,ExpressionType::rvalue);
                break;
            case ast::expr::Binary::ADD:
            case ast::expr::Binary::MINUS:
            case ast::expr::Binary::MUL:
            case ast::expr::Binary::DIV:
            case ast::expr::Binary::FDIV:
                if(!is_binary_op_vaild(lhs_type, rhs_type, &(logic_node->lhs), &(logic_node->rhs)))return ExpressionType::error;
                return logic_node->type = new ExpressionType(lhs_type->prototype,ExpressionType::rvalue);
                break;
            case ast::expr::Binary::ASSIGN:
                if(!is_binary_op_vaild(lhs_type, rhs_type, &(logic_node->lhs), &(logic_node->rhs)))return ExpressionType::error;
                if(lhs_type->value_kind != ExpressionType::lvalue){
                    Logger::error(logic_node->lhs->location,"lvalue required as left operand of assignment");
                    return ExpressionType::error;
                }
                return logic_node->type = new ExpressionType(lhs_type->prototype,ExpressionType::lvalue);
                break;
        }
    }

    std::any TypeAnalyzer::visitUnary(ast::expr::Unary *unit_node, BaseArgs args) {
        return unit_node->type = any_cast<ExpressionType*>(visitExpression(unit_node->terminal,args));
    }

    std::any TypeAnalyzer::visitCast(ast::expr::Cast *cast_node, BaseArgs args) {
        auto src_type = any_cast<ExpressionType*>(visitExpression(cast_node->src,args));
        auto dst_type = any_cast<shared_ptr<Prototype>>(visitAnnotation(cast_node->dst,args));
        if(!args.context->getConversionRules().isExplicitCastRuleExist(src_type->prototype, dst_type)){
            Logger::error(cast_node->location,format()<<"no known conversion from '"
                            <<src_type->prototype->getName()<<"' to '"<<dst_type->getName()<<"'");
            return ExpressionType::Error;
        }
        return cast_node->type = new ExpressionType(dst_type,ExpressionType::rvalue);
    }

    std::any TypeAnalyzer::visitGlobal(ast::Global *global_node, BaseArgs args) {
        for(auto &m:global_node->member_list)
            visitMember(m,args);
        return nullptr;
    }

    std::any TypeAnalyzer::visitModule(ast::Module *mod_node, BaseArgs args) {
        args.domain = args.domain->find(getID(mod_node->name))->as_shared<type::Domain>();
        for(auto &m:mod_node->member_list)
            visitMember(m,args);
        return nullptr;
    }

    std::any TypeAnalyzer::visitClass(ast::Class *cls_node, BaseArgs args) {
        args.domain = args.domain->find(getID(cls_node->name))->as_shared<type::Domain>();
        for(auto &m:cls_node->member_list)
            visitMember(m,args);
        return nullptr;
    }

    std::any TypeAnalyzer::visitFunction(ast::Function *func_node, BaseArgs args) {
        auto function = args.domain->find(getID(func_node->name))->as_shared<type::Function>();
        NotNull(function.get());
        args.domain = function;
        for(auto &s:func_node->statement_list)
            visitStatement(s,args);
        return nullptr;
    }

    std::any TypeAnalyzer::visitLet(ast::stmt::Let *let_node, BaseArgs args) {
        for(auto &var:let_node->variable_list){
            auto name = getID(var->name);
            if(is_name_valid(name,var->location,args.domain)){
                shared_ptr<type::Prototype> result_prototype;
                if(var->initial != nullptr && var->annotation != nullptr){
                    auto init_type = any_cast<ExpressionType*>(visitExpression(var->initial,args));
                    if(init_type->value_kind == ExpressionType::error)continue;
                    if(var->annotation){
                        auto anno_prototype = any_cast<shared_ptr<type::Prototype>>(BaseVisitor::visitAnnotation(var->annotation,args));
                        if(!init_type->prototype->equal(anno_prototype)){
                            if(args.context->getConversionRules().isImplicitCastRuleExist(init_type->prototype,anno_prototype)){
                                Logger::warning(var->initial->location,format()<<"implicit conversion from '"<<init_type->prototype->getName()
                                                                         <<"' to '"<<anno_prototype->getName()<<"'");
                                args.context->getConversionRules().insertCastAST(anno_prototype,&(var->initial));
                            }
                            else {
                                Logger::error(var->initial->location, format() << "initialize expression type '"
                                                                               << init_type->prototype->getName()
                                                                               << "' is not equivalent to variable type '"
                                                                               << anno_prototype->getName() << "'");
                                continue;
                            }
                        }
                    }
                    result_prototype = init_type->prototype;
                }
                else if(var->initial != nullptr){
                    auto init_type = any_cast<ExpressionType*>(visitExpression(var->initial,args));
                    result_prototype = init_type->prototype;
                }
                else if(var->annotation != nullptr){
                    result_prototype = any_cast<shared_ptr<type::Prototype>>(BaseVisitor::visitAnnotation(var->annotation,args));
                }
                else{
                    Logger::error(var->location,"need initial expression or type mark");
                    continue;
                }

                auto field = make_shared<type::Variable>();
                field->setName(name);
                field->setPrototype(result_prototype);
                args.domain->add(field);
            }
        }
        return nullptr;
    }

    std::any TypeAnalyzer::visitSelect(ast::stmt::Select *select_node, BaseArgs args) {
        auto condition_type = any_cast<ExpressionType*>(visitExpression(select_node->condition,args));
        for(auto& c:select_node->case_list){
            if(c->condition){
                auto case_type = any_cast<ExpressionType*>(visitExpression(c->condition,args));
                if(!condition_type->prototype->equal(case_type->prototype)){
                    Logger::error(c->location,format()<<"case conditon type '"
                                                      <<case_type->prototype->getName()<<"' is not equivalent to select expression type '"
                                                      <<condition_type->prototype->getName()<<"'");
                }
            }
            visitStatementList(c->statement_list,args);
        }
        return nullptr;
    }

    std::any TypeAnalyzer::visitLoop(ast::stmt::Loop *loop_node, BaseArgs args) {
        auto condition_type = any_cast<ExpressionType*>(visitExpression(loop_node->condition,args));
        if(!condition_type->prototype->equal(args.context->getBuiltIn().getPrimitive(vm::Data::boolean))){
            Logger::error(loop_node->condition->location,"expression type of loop condition must be Boolean");
        }
        visitStatementList(loop_node->statement_list,args);
        return nullptr;
    }

    std::any TypeAnalyzer::visitIf(ast::stmt::If *ifstmt_node, BaseArgs args) {
        for(auto& c:ifstmt_node->case_list){
            if(c->condition){
                auto case_type = any_cast<ExpressionType*>(visitExpression(c->condition,args));
                if(!case_type->prototype->equal(args.context->getBuiltIn().getPrimitive(vm::Data::boolean))){
                    Logger::error(c->location,format()<<"expression type of if condition must be boolean but here is '"
                                                        <<case_type->prototype->getName()<<"'");
                }
            }
            visitStatementList(c->statement_list,args);
        }
        return nullptr;
    }

    std::any TypeAnalyzer::visitFor(ast::stmt::For *forstmt_node, BaseArgs args) {
        auto iterator_type = any_cast<ExpressionType*>(visitExpression(forstmt_node->iterator,args));
        if(iterator_type->value_kind == ExpressionType::error){
            Logger::error(forstmt_node->iterator->location,"iterator not found");
        }
        else if(iterator_type->value_kind != ExpressionType::lvalue){
            Logger::error(forstmt_node->iterator->location,"iterator must be a lvalue");
        }
        else{
            auto begin_type = any_cast<ExpressionType*>(visitExpression(forstmt_node->begin,args)),
                    end_type = any_cast<ExpressionType*>(visitExpression(forstmt_node->end,args));

            if(!begin_type->prototype->equal(iterator_type->prototype)){
                Logger::error(forstmt_node->begin->location,format()<<"Begin expression type '"
                        <<begin_type->prototype->getName()<<"' is not equivalent to iterator type '"
                        <<iterator_type->prototype->getName()<<"'");
            }
            if(!end_type->prototype->equal(iterator_type->prototype)){
                Logger::error(forstmt_node->begin->location,format()<<"End expression type '"
                        <<end_type->prototype->getName()<< "' is not equivalent to iterator type '"
                        <<iterator_type->prototype->getName()<<"'");
            }
            if(forstmt_node->step){
                auto step_type = any_cast<ExpressionType*>(visitExpression(forstmt_node->step,args));
                if(!step_type->prototype->equal(iterator_type->prototype)){
                    Logger::error(forstmt_node->begin->location,format()<<"Step expression type '"
                        <<step_type->prototype->getName()<<"' is not equivalent to iterator type '"
                        <<iterator_type->prototype->getName()<<"'");
                }
            }
        }


        visitStatementList(forstmt_node->statement_list,args);
        return nullptr;
    }

    std::any TypeAnalyzer::visitReturn(ast::stmt::Return *ret_node, BaseArgs args) {
        visitExpression(ret_node->expr,args);
        return nullptr;
    }

    std::any TypeAnalyzer::visitExprStmt(ast::stmt::ExprStmt *expr_stmt_node, BaseArgs args) {
        visitExpression(expr_stmt_node->expr,args);
        return nullptr;
    }

    void TypeAnalyzer::visitStatementList(std::list<ast::stmt::Statement*> &stmt_list, BaseArgs args) {
        args.domain = make_shared<type::TemporaryDomain>(args.domain);
        for(auto& s:stmt_list)
            visitStatement(s,args);
    }

    std::any TypeAnalyzer::visitParentheses(ast::expr::Parentheses *parentheses_node, BaseArgs args) {
        auto type =  any_cast<ExpressionType*>(visitExpression(parentheses_node->expr,args));
        type->value_kind = ExpressionType::rvalue;
        return parentheses_node->type = type;
    }

}
