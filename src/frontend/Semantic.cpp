//
// Created by yorkin on 7/20/21.
//

#include "Semantic.h"
#include "Exception.h"
#include <stack>
#include <utility>

namespace evoBasic{


    void SymbolTable::collectSymbol(const shared_ptr<Node>& ast){
        list<pair<shared_ptr<Type::Domain>,shared_ptr<Node>>> next_list{{global,ast}};
        while(!next_list.empty()){
            auto domain = next_list.front().first;
            auto current_ast = next_list.front().second;
            next_list.pop_front();
            for(const auto& tree:current_ast->child) {
                shared_ptr<Type::Symbol> ptr;
                AccessFlag access;
                switch(tree->tag){
                    case Tag::Module:
                        ptr = make_shared<Type::Module>();
                        access = tree->get<AccessFlag>(Attr::AccessFlag);
                        break;
                    case Tag::Class:
                        ptr = make_shared<Type::Class>();
                        access = tree->get<AccessFlag>(Attr::AccessFlag);
                        break;
                    case Tag::Type:
                        ptr = make_shared<Type::Record>();
                        access = tree->get<AccessFlag>(Attr::AccessFlag);
                        break;
                    case Tag::Enum:
                        ptr = make_shared<Type::Enumeration>();
                        access = tree->get<AccessFlag>(Attr::AccessFlag);
                        break;
                    default:
                        continue;
                        break;
                }
                auto next_domain = dynamic_pointer_cast<Type::Domain>(ptr);
                tree->set(Attr::Symbol, ptr);

                ptr->setAccessFlag(access);
                ptr->setName(tree->child[0]->get<std::string>(Attr::Lexeme));

                if(next_domain){
                    next_list.emplace_back(next_domain,tree);
                }
                domain->add(ptr);
            }
        }
    }

    void SymbolTable::collectDetail(AST ast){
        list<pair<shared_ptr<Type::Domain>,shared_ptr<Node>>> next_list{{global,ast.root}};
        while(!next_list.empty()){
            auto domain = next_list.front().first;
            auto current_ast = next_list.front().second;
            next_list.pop_front();
            for(const auto& tree:current_ast->child) {
                auto tag = tree->tag;
                if(tag == Tag::Import){

                    if(auto target = get<0>(visitPath(domain,tree->child[0]))){
                        domain->addImport(target);
                    }

                }
                else if(tag == Tag::ExternalFunction){

                    auto c0 = tree->child[0];
                    auto c1 = tree->child[1];
                    auto c2 = tree->child[2];
                    auto lib_name = c1->tag==Tag::Empty ? "" : c1->get<std::string>(Attr::Value);
                    auto entry_name = c2->tag==Tag::Empty ? c0->get<std::string>(Attr::Lexeme)
                                                        : c2->get<std::string>(Attr::Value);
                    auto func = make_shared<Type::ExternalFunction>(lib_name,entry_name);
                    func->setName(c0->get<std::string>(Attr::Lexeme));
                    visitParameterList(domain,func,tree->child[3]);
                    if(tree->child[4]->tag!=Tag::Empty){
                        auto ptr = dynamic_pointer_cast<Type::Prototype>(get<0>(visitPath(domain,tree->child[4])));
                        func->setRetSignature(ptr);
                    }
                    auto access = tree->get<AccessFlag>(Attr::AccessFlag);
                    func->setAccessFlag(access);
                    domain->add(func);
                }
                else if(tag == Tag::Function){

                    auto method = tree->get<MethodFlag>(Attr::MethodFlag);
                    auto func = std::make_shared<Type::UserFunction>(method,tree->child[3]);
                    func->setName(tree->child[0]->get<std::string>(Attr::Lexeme));
                    visitParameterList(domain,func,tree->child[1]);
                    if(tree->child[2]->tag!=Tag::Empty){
                        auto ptr = dynamic_pointer_cast<Type::Prototype>(get<0>(visitPath(domain,tree->child[2]->child[0])));
                        func->setRetSignature(ptr);
                    }
                    else if(!entrance && func->getName()=="main" && domain->getName()=="global"){
                        entrance = func;
                    }
                    auto access = tree->get<AccessFlag>(Attr::AccessFlag);
                    func->setAccessFlag(access);
                    domain->add(func);

                }
                else if(tag == Tag::Init){

                    auto func = make_shared<Type::UserFunction>(MethodFlag::Normal,tree->child[1]);
                    func->setName("init");
                    visitParameterList(domain,func,tree->child[0]);
                    func->setAccessFlag(AccessFlag::Public);
                    domain->add(func);

                }
                else if(tag == Tag::Operator){

                    auto func = make_shared<Type::UserFunction>(MethodFlag::Normal,tree->child[3]);
                    func->setName("");
                    throw "umimpl";

                }
                else if(tag == Tag::Enum){

                    auto symbol = tree->get<shared_ptr<Type::Symbol>>(Attr::Symbol);
                    auto em = static_pointer_cast<Type::Enumeration>(symbol);
                    int value = 0;
                    for(int i=1;i<tree->child.size();i++){
                        auto member = tree->child[i];
                        auto name = member->child[0]->get<std::string>(Attr::Lexeme);
                        if(member->child.size()>1)value = member->child[1]->get<int>(Attr::Value);
                        auto enum_member = make_shared<Type::EnumMember>(value);
                        enum_member->setAccessFlag(AccessFlag::Public);
                        em->add(enum_member);
                        value++;
                    }

                }
                else if(tag == Tag::Type){

                    auto symbol = tree->get<shared_ptr<Type::Symbol>>(Attr::Symbol);
                    auto type = static_pointer_cast<Type::Record>(symbol);
                    for(const auto& field_node:tree->child){
                        if(field_node->tag!=Tag::TypeField)continue;
                        auto name = field_node->child[0]->get<string>(Attr::Lexeme);
                        auto tup = visitAnnotation(domain,field_node->child[1]);
                        auto prototype = dynamic_pointer_cast<Type::Prototype>(get<0>(tup));
                        if(prototype){
                            auto field = make_shared<Type::Field>(prototype,false);
                            field->setName(name);
                            field->setAccessFlag(AccessFlag::Public);
                            type->add(field);
                        }
                    }

                }
                else if(tag == Tag::Let){

                    auto access = tree->get<AccessFlag>(Attr::AccessFlag);
                    for(const auto& variable_node:tree->child){
                        auto name = variable_node->child[0]->get<string>(Attr::Lexeme);
                        auto annotation_node = variable_node->child[1];
                        shared_ptr<Type::Symbol> target_type;
                        if(annotation_node->tag == Tag::Empty){
                            target_type = variant_class;
                        }
                        else{
                            target_type = get<0>(visitAnnotation(domain,annotation_node));
                        }

                        auto tmp = dynamic_pointer_cast<Type::Prototype>(target_type);
                        if(tmp){
                            auto field = make_shared<Type::Field>(tmp,false);
                            field->setName(name);
                            field->setAccessFlag(access);
                            domain->add(field);
                            variable_node->set(Attr::Symbol, field);
                        }
                        else{
                            Logger::error(variable_node->child[0]->pos(),"无法实例化的类型");
                            variable_node->tag = Tag::Error;
                        }

                    }

                }
                else if(tag == Tag::Class){

                    auto symbol = tree->get<shared_ptr<Type::Symbol>>(Attr::Symbol);
                    auto cls = dynamic_pointer_cast<Type::Class>(symbol);
                    auto access = tree->get<AccessFlag>(Attr::AccessFlag);
                    cls->setAccessFlag(access);
                    cls->setName(tree->child[0]->get<std::string>(Attr::Lexeme));

                    auto impl_node = tree->child[1];
                    for(const auto& path:impl_node->child){
                        auto info = visitPath(domain,path);
                        if(!get<0>(info))continue;
                        auto base_class = dynamic_pointer_cast<Type::Class>(get<0>(info));
                        if(base_class){
                            cls->addInherit(base_class);
                        }
                        else{
                            Logger::error(get<1>(info),Format()<<"'"<<get<2>(info)<<"'不是一个类");
                        }
                    }
                    next_list.emplace_back(cls,tree);

                    domain->add(cls);

                }
                else if(tag == Tag::Module){

                    auto symbol = tree->get<shared_ptr<Type::Symbol>>(Attr::Symbol);
                    auto access = tree->get<AccessFlag>(Attr::AccessFlag);
                    auto mod = dynamic_pointer_cast<Type::Module>(symbol);
                    mod->setAccessFlag(access);
                    mod->setName(tree->child[0]->get<std::string>(Attr::Lexeme));
                    next_list.emplace_back(mod,tree);

                    domain->add(mod);

                }
            }
        }
    }

    std::shared_ptr<Type::primitive::VariantClass> SymbolTable::variant_class;
    std::shared_ptr<Type::primitive::Primitive<bool>> SymbolTable::boolean_prototype;
    std::shared_ptr<Type::primitive::Primitive<int>> SymbolTable::integer_prototype;
    std::shared_ptr<Type::primitive::Primitive<double>> SymbolTable::double_prototype;

    std::shared_ptr<Type::primitive::VariantClass> SymbolTable::getVariantClass() {
        if(!variant_class) variant_class = make_shared<Type::primitive::VariantClass>();
        return variant_class;
    }

    shared_ptr<Type::primitive::Primitive<bool>> SymbolTable::getBooleanPrototype() {
        if(!boolean_prototype) boolean_prototype = make_shared<Type::primitive::Primitive<bool>>("boolean");
        return boolean_prototype;
    }

    shared_ptr<Type::primitive::Primitive<int>> SymbolTable::getIntegerPrototype() {
        if(!integer_prototype) integer_prototype = make_shared<Type::primitive::Primitive<int>>("integer");
        return integer_prototype;
    }


    SymbolTable::SymbolTable(const vector<AST>& ast_list): global(new Type::Module()) {
        global->setName("global");
        global->add(getVariantClass());
        global->add(getIntegerPrototype());
        global->add(getBooleanPrototype());

        for(auto& ast:ast_list){
            collectSymbol(ast.root);
        }
        for(auto& ast:ast_list){
            collectDetail(ast);
        }
    }

    list<pair<Position,string>> getPositionLexemeListFromDot(const shared_ptr<Node> ptr){
        if(ptr->tag == Tag::ID) return {make_pair(ptr->pos(),ptr->get<string>(Attr::Lexeme))};
        auto ls = getPositionLexemeListFromDot(ptr->child[0]);
        auto rhs_lexeme = ptr->child[1]->get<string>(Attr::Lexeme);
        auto rhs_pos = ptr->child[1]->pos();
        ls.emplace_back(rhs_pos,rhs_lexeme);
        return ls;
    }

    std::tuple<std::shared_ptr<Type::Symbol>,Position,std::string>
    SymbolTable::visitPath(std::shared_ptr<Type::Domain> domain, const std::shared_ptr<Node> path_node) {
        shared_ptr<Type::Symbol> target;
        auto path = getPositionLexemeListFromDot(path_node->child[0]);
        auto path_position = Position::accross(path.front().first,path.back().first);
        auto last_string = path.back().second;
        auto beginSymbol = domain->lookUp(path.begin()->second);
        if(!beginSymbol){
            Logger::error(path.begin()->first,Format()<<"找不到对象'"<<path.begin()->second<<"'");
            return make_tuple(shared_ptr<Type::Symbol>(nullptr), Position::Empty, "");
        }
        path.pop_front();
        shared_ptr<Type::Domain> ptr = dynamic_pointer_cast<Type::Domain>(beginSymbol);
        for(const auto& p : path){
            const auto& id = p.second;
            auto next = ptr->find(id);
            if(next){
                if(&p != &path.back()){
                    ptr = dynamic_pointer_cast<Type::Domain>(next);
                    if(!ptr){
                        Logger::error(p.first,Format()<<"'"<<id<<"'不是个模块");
                        break;
                    }
                }
                else{
                    target = next;
                }
            }else{
                Logger::error(p.first,Format()<<"找不到对象'"<<id<<"'");
                break;
            }
        }
        if(path.empty())target = beginSymbol;
        return make_tuple(target,path_position,last_string);
    }


    std::tuple<std::shared_ptr<Type::Symbol>,Position,std::string>
    SymbolTable::visitAnnotation(std::shared_ptr<Type::Domain> domain,std::shared_ptr<Node> node) {
        return visitPath(std::move(domain), node->child[0]);
    }


    void SymbolTable::visitParameterList(std::shared_ptr<Type::Domain> domain, std::shared_ptr<Type::Function> function,
                                         std::shared_ptr<Node> parameter_list) {
        for(auto param : parameter_list->child){
            auto isByval = param->get<bool>(Attr::IsByval);
            auto isOptional = param->get<bool>(Attr::IsOptional);
            auto name = param->child[0]->get<string>(Attr::Lexeme);
            auto prototype = dynamic_pointer_cast<Type::Prototype>(get<0>(visitAnnotation(domain,param->child[1])));
            function->addArgument(Type::Function::Argument(name,prototype,isByval,isOptional));
        }
    }

    shared_ptr<Type::primitive::Primitive<double>> SymbolTable::getDoublePrototype() {
        if(!double_prototype)double_prototype = make_shared<Type::primitive::Primitive<double>>("double");
        return double_prototype;
    }


    std::map<TypeAnalyzer::BinaryOpSignature,TypeAnalyzer::PromotionRuleFunction> TypeAnalyzer::typePromotionRule;
    std::set<TypeAnalyzer::BinaryOpSignature> TypeAnalyzer::isCastRuleExist;

    void TypeAnalyzer::check(std::vector<AST>& ast_list,SymbolTable& table) {
        for(auto& ast:ast_list){
            visitStructure(table.global,ast.root);
        }
    }

    void TypeAnalyzer::visitStructure(std::shared_ptr<Type::Domain> domain, std::shared_ptr<Node> root) {
        for(auto& tree:root->child){
            auto tag = tree->tag;
            if(tag == Tag::Module){
                auto mod = static_pointer_cast<Type::Domain>(tree->get<shared_ptr<Type::Symbol>>(Attr::Symbol));
                visitStructure(mod,tree);
            }
            else if(tag == Tag::Class){
                auto cls = static_pointer_cast<Type::Domain>(tree->get<shared_ptr<Type::Symbol>>(Attr::Symbol));
                visitStructure(cls,tree);
            }
            else if(tag == Tag::Function){
                auto user_function = tree->get<std::shared_ptr<Type::Symbol>>(Attr::Symbol)->as_shared<Type::UserFunction>();
                visitStatement(domain,user_function,tree);
            }
            else if(tag == Tag::Let){ // let statement in Class or Module
                visitLetStmt(domain,tree);
            }
        }
    }

    void TypeAnalyzer::visitStatement(std::shared_ptr<Type::Domain> domain,
                                      std::shared_ptr<Type::UserFunction> function, std::shared_ptr<Node> root) {
        auto scope = make_shared<TemporaryScope>(domain,function);
        for(auto& stmt_node:root->child){
            auto tag = stmt_node->tag;
            if(tag == Tag::Let){
                visitLetStmt(scope,stmt_node);
            }
            else if(tag == Tag::If){
                for(auto& child:stmt_node->child){
                    if(child->tag == Tag::ElseIf){
                        auto cond_type = visitExpression(scope,child->child[0]).first->as_shared<Type::Prototype>();
                        if(!cond_type->equal(SymbolTable::getBooleanPrototype())){
                            Logger::error(child->pos(),"条件表达式类型不是'Boolean'");
                        }
                        visitStatement(scope,function,child->child[1]);
                    }
                    else if(child->tag == Tag::Else){
                        visitStatement(scope,function,child->child[0]);
                    }
                }
            }
            else if(tag == Tag::Loop){
                auto cond_type = visitExpression(scope,stmt_node->child[0]).first->as_shared<Type::Prototype>();
                if(!cond_type->equal(SymbolTable::getBooleanPrototype())){
                    Logger::error(stmt_node->pos(),"条件表达式类型不是'Boolean'");
                }
                visitStatement(scope,function,stmt_node->child[1]);
            }
            else if(tag == Tag::Select){
                auto select_exp_type = visitExpression(scope,stmt_node->child[0]).first->as_shared<Type::Prototype>();
                for(auto& case_node:stmt_node->child){
                    if(case_node->tag == Tag::Case){
                        auto case_exp_type = visitExpression(scope,case_node->child[0]).first->as_shared<Type::Prototype>();
                        if(!select_exp_type->equal(case_exp_type)){
                            Logger::error(stmt_node->pos(),"'Select'语句表达式类型与'Case'分支表达式类型不一致");
                            //TODO 优化Position标记，输出更详细的出错位置
                        }
                        visitStatement(scope,function,stmt_node->child[1]);
                    }
                    else if(case_node->tag == Tag::DefaultCase){
                        visitStatement(scope,function,stmt_node->child[0]);
                    }
                }
            }
            else if(tag == Tag::For){
                //TODO
            }
            else if(tag == Tag::Return){
                auto exp_type = dynamic_pointer_cast<Type::Prototype>(visitExpression(scope,stmt_node->child[0]).first);
                if(!exp_type->equal(function->getRetSignature())){
                    Logger::error(stmt_node->pos(),"返回表达式类型与函数签名不一致");
                }
            }
        }
    }

    void TypeAnalyzer::visitLetStmt(std::shared_ptr<Type::Domain> domain, std::shared_ptr<Node> node) {
        for(auto& variable_node:node->child){
            auto name = variable_node->child[0]->get<string>(Attr::Lexeme);
            auto initial_node = variable_node->child[2];
            if(variable_node->tag != Tag::Variable || initial_node->tag == Tag::Empty)continue;
            auto var_type = static_pointer_cast<Type::Prototype>(variable_node->get<shared_ptr<Type::Symbol>>(Attr::Symbol));
            auto exp_type = visitExpression(domain,initial_node).first->as_shared<Type::Prototype>();
            if(!var_type->equal(exp_type)){
                Logger::error(variable_node->child[0]->pos(),Format()<<"初始化表达式类型与变量'"<<name<<"'类型不一致");
            }
        }
    }


    ExprResult TypeAnalyzer::visitExpression(std::shared_ptr<Type::Domain> domain,std::shared_ptr<Node> node){

    }

    ExprResult TypeAnalyzer::visitTermAddCmp(std::shared_ptr<Type::Domain> domain, std::shared_ptr<Node> node) {
        set<Tag> condition = {Tag::Mul,Tag::Div,Tag::FDiv,Tag::Add,Tag::Sub,Tag::EQ,Tag::LE,Tag::GE,Tag::LT,Tag::GT,Tag::NE};
        if(condition.contains(node->tag)){
            auto lhs_type = visitTermAddCmp(domain, node->child[0]).first;
            auto rhs_type = visitTermAddCmp(domain, node->child[1]).first;
            if(lhs_type->getKind() == Type::DeclarationEnum::Primitive &&
                rhs_type->getKind() == Type::DeclarationEnum::Primitive){

                auto rule = typePromotionRule.find({lhs_type.get(),rhs_type.get()});
                if(rule == typePromotionRule.end()){
                    throw ExprException(node->pos(),Format()<<"无法进行'"<<lhs_type->getName()
                                                            <<" op "<<rhs_type->getName()<<"'的二元运算");
                }
                auto ret_type = rule->second.operator()(node);
                return {ret_type,ExprKind::rvalue};
            }
            else if((node->tag == Tag::NE || node->tag == Tag::EQ) &&
                    lhs_type->getKind() == Type::DeclarationEnum::Field &&
                    rhs_type->getKind() == Type::DeclarationEnum::Enum_){


            }
            else if(node->tag == Tag::EQ &&
                    lhs_type->getKind() == Type::DeclarationEnum::Type &&
                    rhs_type->getKind() == Type::DeclarationEnum::Type){


            }
        }
        else{
            return visitFactor(domain,node);
        }
    }

    ExprResult
    TypeAnalyzer::visitFactor(std::shared_ptr<Type::Domain> domain, std::shared_ptr<Node> node) {
        if(node->tag == Tag::Cast){
            auto lhs = visitFactor(domain,node->child[0]);
            auto anno_info = SymbolTable::visitAnnotation(domain,node);
            auto anno_type = get<0>(anno_info)->as_shared<Type::Prototype>();
            isCastRuleExist.contains({lhs.first.get(),anno_type.get()});
            return {anno_type,ExprKind::rvalue};
        }
        else if(node->tag == Tag::Assign){
            auto lhs = visitFactor(domain,node->child[0]);
            auto rhs = visitFactor(domain,node->child[1]);
            auto lhs_prototype = lhs.first->as_shared<Type::Prototype>();
            auto rhs_prototype = rhs.first->as_shared<Type::Prototype>();

            if(lhs.second != ExprKind::lvalue){
                throw ExprException(node->child[0]->pos(),"非左值无法被赋值");
            }
            else if(!lhs_prototype->equal(rhs_prototype)){
                throw ExprException(node->pos(),"赋值运算符左右边类型不一致");
            }
            return lhs;
        }
        else{
            auto symbol = visitUnit(domain,node);
            switch (symbol->getKind()) {
                case Type::DeclarationEnum::EnumMember:
                    return {symbol->getParent().lock(),ExprKind::rvalue};
                case Type::DeclarationEnum::Field:
                    return {
                        symbol->as_shared<Type::Field>()->getPrototype(),
                        static_pointer_cast<Type::Field>(symbol)->isConstant() ? ExprKind::rvalue : ExprKind::lvalue
                    };
                default:
                    throw ExprException(node->pos(),"无法赋值或转型的表达式");
            }
        }
    }

    /*
     *  检查 ModuleA.ClassB.FunctionC(Args).MemberD 形式的路径
     */
    std::shared_ptr<Type::Symbol>
    TypeAnalyzer::visitUnit(std::shared_ptr<Type::Domain> domain, std::shared_ptr<Node> node) {
        if(node->tag == Tag::SelfNeg || node->tag == Tag::SelfPot)node = node->child[0];

        if(node->tag == Tag::Dot){
            auto lhs = node->child[0];
            auto rhs = node->child[1];

            if(lhs->tag == Tag::SelfNeg || lhs->tag == Tag::SelfPot)lhs = lhs->child[0];
            if(rhs->tag == Tag::SelfNeg || rhs->tag == Tag::SelfPot)rhs = rhs->child[0];

            auto prefix = visitUnit(domain, lhs)->as_shared<Type::Domain>();
            if(!prefix){
                throw ExprException(lhs->pos(), "不是一个成员集合");
            }

            switch(rhs->tag){
                case Tag::Digit:
                case Tag::Decimal:
                case Tag::String:
                case Tag::Char:
                    throw ExprException(rhs->pos(), "非法表达式");
                    break;
                case Tag::ID:
                case Tag::Callee:
                    {
                        auto name = rhs->get<string>(Attr::Lexeme);
                        auto member = prefix->find(name);
                        if(!member){
                            throw ExprException(node->pos(), "找不到对象");
                        }
                        if(rhs->tag == Tag::Callee){
                            auto function = member->as_shared<Type::Function>();
                            if(!function) {
                                throw ExprException(node->pos(), Format() << "'" << name << "'不是一个函数或者过程");
                            }
                            visitParameterList(domain,function,rhs->child[0]);
                            auto tricky_function_return = make_shared<Type::Field>(function->getRetSignature(),false);
                            return tricky_function_return;
                        }
                        else {
                            return member;
                        }
                    }
                    break;
            }
        }
        else if(node->tag == Tag::ID){
            auto member = domain->lookUp(node->get<string>(Attr::Lexeme));
            if(!member){
                throw UnitException(node->pos(), "找不到对象");
            }
            return member;
        }
        else if(node->tag == Tag::Callee){
            auto name = node->get<string>(Attr::Lexeme);
            auto member = domain->lookUp(name);
            if(!member) {
                throw UnitException(node->pos(), "找不到对象");
            }
            auto function = member->as_shared<Type::Function>();
            if(!function) {
                throw UnitException(node->pos(), Format() << "'" << name << "'不是一个函数或者过程");
            }
            visitParameterList(domain,function,node->child[1]);
            return function->getRetSignature();
        }
        else if(node->tag == Tag::Digit){
            return SymbolTable::getIntegerPrototype();
        }
        else if(node->tag == Tag::Decimal){
            return SymbolTable::getDoublePrototype();
        }
        else{
            throw "unimplement";
        }
    }

    void
    TypeAnalyzer::visitParameterList(std::shared_ptr<Type::Domain> domain, std::shared_ptr<Type::Function> function,
                                     std::shared_ptr<Node> node) {

    }


    TemporaryScope::TemporaryScope(std::weak_ptr<Type::Domain> parent, std::shared_ptr<Type::UserFunction> current)
        : Domain(Type::DeclarationEnum::FunctionScope),current_function(current){
        setParent(std::move(parent));
    }

    shared_ptr<Type::Symbol> TemporaryScope::find(const string &name) {
        auto target = local_variables.find(name);
        if(target==local_variables.end())return {nullptr};
        return target->second;
    }

    void TemporaryScope::add(std::shared_ptr<Type::Symbol> symbol) {
        this->local_variables.insert({symbol->getName(),symbol});
    }


}