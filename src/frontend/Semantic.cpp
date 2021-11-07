//
// Created by yorkin on 7/20/21.
//

#include "Semantic.h"
#include "Exception.h"
#include <stack>
#include <utility>

namespace evoBasic{

    BuiltIn::BuiltIn() {
        using namespace Type::primitive;
        boolean_prototype = make_shared<Primitive<dataDef::boolean>>("boolean");
        byte_prototype = make_shared<Primitive<dataDef::i8>>("byte");
        short_prototype = make_shared<Primitive<dataDef::i16>>("short");
        integer_prototype = make_shared<Primitive<dataDef::i32>>("integer");
        long_prototype = make_shared<Primitive<dataDef::i64>>("long");
        single_prototype = make_shared<Primitive<dataDef::f32>>("single");
        double_prototype = make_shared<Primitive<dataDef::f64>>("double");

        variant_class = make_shared<VariantClass>();
        variant_class->setName("variant");

        error_symbol = make_shared<Type::Error>();
        error_symbol->setName("< Error >");
    }

    BuiltIn::doubleTypePtr BuiltIn::getDoublePrototype() {
        return double_prototype;
    }

    BuiltIn::longTypePtr BuiltIn::getLongPrototype() {
        return long_prototype;
    }

    BuiltIn::byteTypePtr BuiltIn::getBytePrototype() {
        return byte_prototype;
    }

    BuiltIn::shortTypePtr BuiltIn::getShortPrototype() {
        return short_prototype;
    }

    BuiltIn::singleTypePtr BuiltIn::getSinglePrototype() {
        return single_prototype;
    }

    std::shared_ptr<Type::primitive::VariantClass> BuiltIn::getVariantClass() {
        return variant_class;
    }

    shared_ptr<Type::primitive::Primitive<bool>> BuiltIn::getBooleanPrototype() {
        return boolean_prototype;
    }

    shared_ptr<Type::primitive::Primitive<int>> BuiltIn::getIntegerPrototype() {
        return integer_prototype;
    }

    std::shared_ptr<Type::Error> BuiltIn::getErrorPrototype() {
        return error_symbol;
    }


    ConversionRules::ConversionRules(BuiltIn* builtIn) {
        enum Types {bin=0,i08,i16,i32,i64,f32,f64};
        auto& in = *builtIn;
        vector<shared_ptr<Type::Prototype>> enumToPrototype = {
            in.getBooleanPrototype(),
            in.getBytePrototype(),
            in.getShortPrototype(),
            in.getIntegerPrototype(),
            in.getLongPrototype(),
            in.getSinglePrototype(),
            in.getDoublePrototype()
        };

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

        auto makeTypeAnnotation = [=](string type_name){
            return make_node(Tag::Annotation,{
                    make_node(Tag::Path,{
                            make_node(Tag::ID,{{Attr::Lexeme,std::move(type_name)}})
                    })
            });
        };


        auto promoteToTargetFunction = [=](SymbolPtr lhs,SymbolPtr rhs,
                                           const shared_ptr<Type::Prototype> target_type,
                                           const shared_ptr<Node>& node)->shared_ptr<Type::Prototype>{
            auto lhs_node = node->child[0], rhs_node = node->child[1];

            if(lhs != target_type.get())
                node->child[0] = make_node(Tag::Cast,{lhs_node,makeTypeAnnotation(target_type->getName())});

            if(rhs != target_type.get())
                node->child[1] = make_node(Tag::Cast,{rhs_node,makeTypeAnnotation(target_type->getName())});

            return target_type;
        };

        for(int l=0;l<enumToPrototype.size();l++){
            for(int r=0;r<enumToPrototype.size();r++){
                const auto& l_type = enumToPrototype[l];
                const auto& r_type = enumToPrototype[r];
                Types idx = promotion_table[l][r];
                auto result_prototype = enumToPrototype[(int)idx];
                auto handler = std::bind(promoteToTargetFunction,l_type.get(),r_type.get(),result_prototype,placeholders::_1);
                typePromotionRule.insert({{l_type.get(),r_type.get()},handler});
            }
        }
    }

    ConversionRules::promotion_iterator ConversionRules::promotion(SymbolPtr lhs, SymbolPtr rhs) {
        return typePromotionRule.find({lhs,rhs});
    }

    bool ConversionRules::isEmpty(ConversionRules::promotion_iterator iterator) {
        return typePromotionRule.end() == iterator;
    }

    bool ConversionRules::isCastRuleExist(SymbolPtr lhs,SymbolPtr rhs) {
        return castRuleFlag.contains({lhs,rhs});
    }


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
                        next_list.emplace_back(ptr->as_shared<Type::Domain>(),tree);
                        break;
                    case Tag::Class:
                        ptr = make_shared<Type::Class>();
                        access = tree->get<AccessFlag>(Attr::AccessFlag);
                        next_list.emplace_back(ptr->as_shared<Type::Domain>(),tree);
                        break;
                    case Tag::Type:
                        ptr = make_shared<Type::Record>();
                        access = tree->get<AccessFlag>(Attr::AccessFlag);
                        break;
                    case Tag::Enum:
                        ptr = make_shared<Type::Enumeration>();
                        access = tree->get<AccessFlag>(Attr::AccessFlag);
                        break;
                    case Tag::Let: {
                            access = tree->get<AccessFlag>(Attr::AccessFlag);
                            bool isConst = false;
                            for (auto &variable_node: tree->child) {
                                if (variable_node->tag != Tag::Variable)continue;
                                auto var_name = variable_node->child[0]->get<string>(Attr::Lexeme);
                                auto field = make_shared<Type::Field>(variable_node);
                                field->setName(var_name);
                                field->setAccessFlag(access);
                                field->setConstant(isConst);
                                if (domain->find(var_name))
                                    Logger::error(tree->pos(), Format() << "redefinition of '" << var_name << "'");
                                else
                                    domain->add(field);
                                variable_node->set<shared_ptr<Type::Symbol>>(Attr::Symbol,field);
                            }
                        }
                        continue;
                    default:
                        continue;
                        break;
                }
                auto name = tree->child[0]->get<std::string>(Attr::Lexeme);
                ptr->setParent(domain);
                ptr->setAccessFlag(access);
                ptr->setName(name);
                tree->set<shared_ptr<Type::Symbol>>(Attr::Symbol, ptr);

                //check name conflict
                if(domain->find(name)){
                    Logger::error(tree->pos(),Format()<<"redefinition of '"<<name<<"'");
                }else{
                    domain->add(ptr);
                }
            }
        }
    }

    void SymbolTable::collectDetail(AST ast){
        list<pair<shared_ptr<Type::Domain>,shared_ptr<Node>>> next_list{{global,ast.root}};
        while(!next_list.empty()) {
            auto domain = next_list.front().first;
            auto current_ast = next_list.front().second;
            next_list.pop_front();
            for (const auto &tree: current_ast->child) {
                auto tag = tree->tag;
                if (tag == Tag::Import) {

                    if (auto target = get<0>(visitPath(domain, tree->child[0]))) {
                        if (domain->getKind() == Type::DeclarationEnum::Module) {
                            domain->as_shared<Type::Module>()->addImport(target);
                        } else {
                            Logger::error(tree->pos(), "'Import' is only available in Module");
                        }
                    }

                } else if (tag == Tag::ExternalFunction) {

                    auto c0 = tree->child[0];
                    auto c1 = tree->child[1];
                    auto c2 = tree->child[2];
                    auto lib_name = c1->tag == Tag::Empty ? "" : c1->get<std::string>(Attr::Value);
                    auto entry_name = c2->tag == Tag::Empty ? c0->get<std::string>(Attr::Lexeme)
                                                            : c2->get<std::string>(Attr::Value);
                    auto func = make_shared<Type::ExternalFunction>(lib_name, entry_name);
                    func->setName(c0->get<std::string>(Attr::Lexeme));
                    visitParameterList(domain, func, tree->child[3]);
                    if (tree->child[4]->tag != Tag::Empty) {
                        auto ptr = dynamic_pointer_cast<Type::Prototype>(get<0>(visitPath(domain, tree->child[4])));
                        func->setRetSignature(ptr);
                    }
                    auto access = tree->get<AccessFlag>(Attr::AccessFlag);
                    func->setAccessFlag(access);
                    tree->set<shared_ptr<Type::Symbol>>(Attr::Symbol, func);
                    domain->add(func->as_shared<Type::Symbol>());
                } else if (tag == Tag::Function) {

                    auto method = tree->get<MethodFlag>(Attr::MethodFlag);
                    auto func = std::make_shared<Type::UserFunction>(method, tree->child[3]);
                    func->setName(tree->child[0]->get<std::string>(Attr::Lexeme));
                    visitParameterList(domain, func, tree->child[1]);
                    if (tree->child[2]->tag != Tag::Empty) {
                        auto ptr = dynamic_pointer_cast<Type::Prototype>(
                                get<0>(visitPath(domain, tree->child[2]->child[0])));
                        func->setRetSignature(ptr);
                    } else if (!entrance && func->getName() == "main" && domain->getName() == "global") {
                        entrance = func;
                    }
                    auto access = tree->get<AccessFlag>(Attr::AccessFlag);
                    func->setAccessFlag(access);
                    tree->set<shared_ptr<Type::Symbol>>(Attr::Symbol, func);
                    domain->add(func->as_shared<Type::Symbol>());

                } else if (tag == Tag::Init) {

                    auto func = make_shared<Type::UserFunction>(MethodFlag::Normal, tree->child[1]);
                    func->setName("init");
                    visitParameterList(domain, func, tree->child[0]);
                    func->setAccessFlag(AccessFlag::Public);
                    tree->set<shared_ptr<Type::Symbol>>(Attr::Symbol, func);
                    domain->add(func->as_shared<Type::Symbol>());

                } else if (tag == Tag::Operator) {

                    auto func = make_shared<Type::UserFunction>(MethodFlag::Normal, tree->child[3]);
                    func->setName("");
                    throw "umimpl";

                } else if (tag == Tag::Enum) {

                    auto symbol = tree->get<shared_ptr<Type::Symbol>>(Attr::Symbol);
                    auto em = static_pointer_cast<Type::Enumeration>(symbol);
                    int value = 0;
                    for (int i = 1; i < tree->child.size(); i++) {
                        auto member = tree->child[i];
                        auto name = member->child[0]->get<std::string>(Attr::Lexeme);
                        if (member->child.size() > 1)value = member->child[1]->get<int>(Attr::Value);
                        auto enum_member = make_shared<Type::EnumMember>(value);
                        enum_member->setAccessFlag(AccessFlag::Public);
                        enum_member->setName(name);
                        em->add(enum_member);
                        value++;
                    }

                } else if (tag == Tag::Type) {

                    auto symbol = tree->get<shared_ptr<Type::Symbol>>(Attr::Symbol);
                    auto type = static_pointer_cast<Type::Record>(symbol);
                    for (const auto &field_node: tree->child) {
                        if (field_node->tag != Tag::TypeField)continue;
                        auto name = field_node->child[0]->get<string>(Attr::Lexeme);
                        auto tup = visitAnnotation(domain, field_node->child[1]);
                        auto prototype = dynamic_pointer_cast<Type::Prototype>(get<0>(tup));
                        if (prototype) {
                            auto field = make_shared<Type::Field>(prototype);
                            field->setName(name);
                            field->setConstant(false);
                            field->setAccessFlag(AccessFlag::Public);
                            type->add(field);
                        }
                    }

                } else if (tag == Tag::Let) {

                    //find dependencies of variable and add into field_depend
//                    for (auto &variable_node: tree->child) {
//                        if (variable_node->tag != Tag::Variable)continue;
//                        auto field = variable_node->get<shared_ptr<Type::Symbol>>(
//                                Attr::Symbol)->as_shared<Type::Field>();
//
//                        auto annotation_node = variable_node->child[1];
//                        auto initial_node = variable_node->child[2];
//                        //collect symbols in initial expression
//                        list<Type::Field> depend_list;
//                        std::function<void(shared_ptr<Type::Domain>, shared_ptr<Node>)> collect_depend
//                                = [&](shared_ptr<Type::Domain> domain, shared_ptr<Node> node) -> void {
//                                    if (node->tag == Tag::Dot) {
//                                        auto tup = visitPath(domain, node);
//
//                                    } else
//                                        for (auto &child: node->child) {
//                                            collect_depend(domain, child);
//                                        }
//                                };
//
//                    }

                } else if (tag == Tag::Class) {

                    auto symbol = tree->get<shared_ptr<Type::Symbol>>(Attr::Symbol);
                    auto cls = dynamic_pointer_cast<Type::Class>(symbol);
                    auto access = tree->get<AccessFlag>(Attr::AccessFlag);
                    cls->setAccessFlag(access);
                    cls->setName(tree->child[0]->get<std::string>(Attr::Lexeme));

                    auto impl_node = tree->child[1];
                    for (const auto &path: impl_node->child) {
                        auto info = visitPath(domain, path);
                        if (!get<0>(info))continue;
                        auto base_class = dynamic_pointer_cast<Type::Class>(get<0>(info));
                        if (base_class) {
                            cls->setExtend(base_class);
                        } else {
                            Logger::error(get<1>(info), Format() << "'" << get<2>(info) << "'不是一个类");
                        }
                    }
                    next_list.emplace_back(cls, tree);

                    domain->add(cls);

                } else if (tag == Tag::Module) {

                    auto symbol = tree->get<shared_ptr<Type::Symbol>>(Attr::Symbol);
                    auto access = tree->get<AccessFlag>(Attr::AccessFlag);
                    auto mod = dynamic_pointer_cast<Type::Module>(symbol);
                    mod->setAccessFlag(access);
                    mod->setName(tree->child[0]->get<std::string>(Attr::Lexeme));
                    next_list.emplace_back(mod, tree);

                    domain->add(mod);

                }
            }
        }
    }


    SymbolTable::SymbolTable(const vector<AST>& ast_list): global(new Type::Module()) {
        global->setName("global");
        auto& in = Semantics::Instance()->getBuiltIn();
        global->add(in.getVariantClass());
        global->add(in.getBooleanPrototype());
        global->add(in.getBytePrototype());
        global->add(in.getShortPrototype());
        global->add(in.getIntegerPrototype());
        global->add(in.getLongPrototype());
        global->add(in.getSinglePrototype());
        global->add(in.getDoublePrototype());


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


    void TypeAnalyzer::check(std::vector<AST>& ast_list) {
        for(auto& ast:ast_list){
            visitStructure(table->global,ast.root);
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
                visitStatement(domain,user_function,tree->child[3]);
            }
            else if(tag == Tag::Let){ // let statement in Class or Module
                visitLoadedLetStmt(domain, tree);
            }
        }
    }

    void TypeAnalyzer::visitStatement(std::shared_ptr<Type::Domain> domain,
                                      std::shared_ptr<Type::UserFunction> function, std::shared_ptr<Node> root) {
        auto scope = make_shared<TemporaryScope>(domain,function);
        for(auto& stmt_node:root->child){
            auto tag = stmt_node->tag;
            if(tag == Tag::Let){
                visitLocalLetStmt(scope, stmt_node);
            }
            else if(tag == Tag::If){
                for(auto& child:stmt_node->child){
                    if(child->tag == Tag::ElseIf){
                        auto cond_type = visitExpression(scope,child->child[0]).first->as_shared<Type::Prototype>();
                        if(cond_type && !cond_type->equal(Semantics::Instance()->getBuiltIn().getBooleanPrototype())){
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
                if(!cond_type->equal(Semantics::Instance()->getBuiltIn().getBooleanPrototype())){
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

    void TypeAnalyzer::visitLoadedLetStmt(std::shared_ptr<Type::Domain> domain, std::shared_ptr<Node> node) {
        for(auto& variable_node:node->child){
            if(variable_node->tag != Tag::Variable)continue;

            auto name = variable_node->child[0]->get<string>(Attr::Lexeme);
            auto anno_node = variable_node->child[1];
            auto initial_node = variable_node->child[2];

            if(anno_node->tag == Tag::Empty && initial_node->tag != Tag::Empty){
                auto exp_type = visitExpression(domain,initial_node).first->as_shared<Type::Prototype>();

            }
            else if(anno_node->tag != Tag::Empty && initial_node->tag != Tag::Empty){

            }
            else{
                Logger::error(variable_node->pos(),"无法进行类型推断");
            }

            if(variable_node->tag != Tag::Variable || initial_node->tag == Tag::Empty)continue;
            auto var_type = variable_node->get<shared_ptr<Type::Symbol>>(Attr::Symbol)->as_shared<Type::Field>()->getPrototype();
            auto exp_type = visitExpression(domain,initial_node).first->as_shared<Type::Prototype>();
            if(!var_type->equal(exp_type)){
                Logger::error(variable_node->child[0]->pos(),Format()<<"初始化表达式类型与变量'"<<name<<"'类型'"<<var_type->getName()<<"'不一致");
            }
        }
    }

    void TypeAnalyzer::visitLocalLetStmt(shared_ptr<Type::Domain> domain,shared_ptr<Node> node){
        bool is_const = false;
        for(auto& variable_node:node->child){
            if(variable_node->tag != Tag::Variable)continue;
            auto name = variable_node->child[0]->get<string>(Attr::Lexeme);
            if(domain->find(name)){
                Logger::error(variable_node->pos(),"变量重复声明");
                return;
            }

            auto annotation_node = variable_node->child[1];
            auto initial_node = variable_node->child[2];

            shared_ptr<Type::Prototype> var_type(nullptr);

            if(annotation_node->tag == Tag::Empty){
                if(initial_node->tag == Tag::Empty){
                    Logger::error(variable_node->pos(),"无法进行类型推导，变量必须有初始化表达式或类型标记");
                }
                else{
                    auto result = visitExpression(domain,initial_node);
                    if(result.second != ExprKind::error)var_type = result.first->as_shared<Type::Prototype>();
                }
            }
            else{
                if(initial_node->tag == Tag::Empty){
                    var_type = get<0>(SymbolTable::visitAnnotation(domain,annotation_node))->as_shared<Type::Prototype>();
                }
                else{
                    auto result = visitExpression(domain,initial_node);
                    auto anno_type = get<0>(SymbolTable::visitAnnotation(domain,annotation_node))->as_shared<Type::Prototype>();
                    if(result.second != ExprKind::error){
                        auto expr_type = result.first->as_shared<Type::Prototype>();
                        if(!anno_type->equal(expr_type)){
                            Logger::error(variable_node->child[0]->pos(),Format()<<"初始化表达式类型与变量'"<<name<<"'类型不一致");
                        }
                        else{
                            var_type = anno_type;
                        }
                    }
                }
            }

            if(var_type){
                auto tmp = make_shared<Type::Field>(var_type);
                tmp->setConstant(false);
                tmp->setName(name);
                domain->add(tmp);
            }
            else{
                auto tmp = make_shared<Type::Field>(Semantics::Instance()->getBuiltIn().getErrorPrototype());
                tmp->setConstant(false);
                tmp->setName(name);
                domain->add(tmp);
            }
        }
    }


    ExprResult TypeAnalyzer::visitExpression(std::shared_ptr<Type::Domain> domain,std::shared_ptr<Node> node){
        try{
            return visitLogic(domain,node->child[0]);
        }
        catch (SkipExprException &e){
            return {nullptr, ExprKind::error};
        }
        catch (ExprException &e){
            Logger::error(e.getPos(),e.getMsg());
            return {nullptr, ExprKind::error};
        }

    }

    ExprResult TypeAnalyzer::visitExprWithoutCatch(std::shared_ptr<Type::Domain> domain,std::shared_ptr<Node> node){
        return visitLogic(domain,node->child[0]);
    }

    ExprResult TypeAnalyzer::visitLogic(std::shared_ptr<Type::Domain> domain, std::shared_ptr<Node> node) {
        auto tag = node->tag;
        switch (node->tag){
            case Tag::And:
            case Tag::Or:
            case Tag::Xor:
            {
                auto lhs_type = visitLogic(domain,node->child[0]).first->as_shared<Type::primitive::Primitive<bool>>();
                auto rhs_type = visitLogic(domain,node->child[1]).first->as_shared<Type::primitive::Primitive<bool>>();
                if(!lhs_type){
                    throw ExprException(node->pos(),"左值类型不是Boolean");
                }
                else if(!rhs_type){
                    throw ExprException(node->pos(),"右值类型不是Boolean");
                }
                return {Semantics::Instance()->getBuiltIn().getBooleanPrototype(), ExprKind::rvalue};
            }
            break;
            case Tag::Not:
            {
                auto rhs_type = visitLogic(domain,node->child[0]).first->as_shared<Type::primitive::Primitive<bool>>();
                if(!rhs_type){
                    throw ExprException(node->pos(),"右值类型不是Boolean");
                }
                return {Semantics::Instance()->getBuiltIn().getBooleanPrototype(), ExprKind::rvalue};
            }
            break;
            default:
                return visitTermAddCmp(domain,node);
        }
    }

    ExprResult TypeAnalyzer::visitTermAddCmp(std::shared_ptr<Type::Domain> domain, std::shared_ptr<Node> node) {

        switch (node->tag) {
            case Tag::Mul:
            case Tag::Div:
            case Tag::FDiv:
            case Tag::Add:
            case Tag::Sub:
            case Tag::LE:
            case Tag::GE:
            case Tag::LT:
            case Tag::GT:
            {
                auto lhs_type = visitTermAddCmp(domain, node->child[0]).first;
                auto rhs_type = visitTermAddCmp(domain, node->child[1]).first;
                auto rule = Semantics::Instance()->getConversionRules().promotion(lhs_type.get(), rhs_type.get());
                if(Semantics::Instance()->getConversionRules().isEmpty(rule)){
                    throw ExprException(node->pos(),Format()<<"无法进行'"<<lhs_type->getName()
                                                            <<" op "<<rhs_type->getName()<<"'的二元运算");
                }
                auto ret_type = rule->second.operator ()(node);
                return {ret_type,ExprKind::rvalue};
            }
            break;
            case Tag::EQ:
            case Tag::NE:
            {
                auto lhs_type = visitTermAddCmp(domain, node->child[0]).first;
                auto rhs_type = visitTermAddCmp(domain, node->child[1]).first;
                auto lhs_kind = lhs_type->getKind();
                auto rhs_kind = rhs_type->getKind();
                if(lhs_kind == Type::DeclarationEnum::Primitive &&
                   rhs_kind == Type::DeclarationEnum::Primitive) {
                    auto rule = Semantics::Instance()->getConversionRules().promotion(lhs_type.get(), rhs_type.get());
                    if(Semantics::Instance()->getConversionRules().isEmpty(rule)) {
                        throw ExprException(node->pos(),Format()<<"无法进行'"<<lhs_type->getName()
                                                                <<" op "<<rhs_type->getName()<<"'的二元运算");
                    }
                    rule->second.operator ()(node);
                    return {Semantics::Instance()->getBuiltIn().getBooleanPrototype(),ExprKind::rvalue};
                }
                else if(lhs_kind == Type::DeclarationEnum::Enum_ &&
                        rhs_kind == Type::DeclarationEnum::Enum_ &&
                        lhs_type == rhs_type) {
                    return {Semantics::Instance()->getBuiltIn().getBooleanPrototype(),ExprKind::rvalue};
                }
            }
            break;
            default:
                return visitFactor(domain,node);
        }
    }

    ExprResult
    TypeAnalyzer::visitFactor(std::shared_ptr<Type::Domain> domain, std::shared_ptr<Node> node) {
        if(node->tag == Tag::Cast){
            auto lhs = visitFactor(domain,node->child[0]);
            auto anno_info = SymbolTable::visitAnnotation(domain,node->child[1]);
            auto anno_type = get<0>(anno_info)->as_shared<Type::Prototype>();
            if(Semantics::Instance()->getConversionRules().isCastRuleExist(lhs.first.get(),anno_type.get())){
                return {anno_type,ExprKind::rvalue};
            }
            else{
                throw ExprException(node->pos(),"未定义的类型转换");
            }
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
                case Type::DeclarationEnum::Enum_:
                    return {symbol,ExprKind::rvalue};
                case Type::DeclarationEnum::EnumMember:
                    return {symbol->getParent().lock(),ExprKind::rvalue};
                case Type::DeclarationEnum::Field:
                    if(symbol->as_shared<Type::Field>()->getPrototype()->getKind() == Type::DeclarationEnum::Error){
                        throw SkipExprException();
                    }
                    else return {
                        symbol->as_shared<Type::Field>()->getPrototype(),
                        static_pointer_cast<Type::Field>(symbol)->isConstant() ? ExprKind::rvalue : ExprKind::lvalue
                    };
                case Type::DeclarationEnum::Primitive:
                    return {symbol,ExprKind::rvalue};
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
                            auto tricky_function_return = make_shared<Type::Field>(function->getRetSignature());
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
            visitParameterList(domain,function,node->child[0]);
            return function->getRetSignature();
        }
        else if(node->tag == Tag::Digit){
            return Semantics::Instance()->getBuiltIn().getIntegerPrototype();
        }
        else if(node->tag == Tag::Decimal){
            return Semantics::Instance()->getBuiltIn().getDoublePrototype();
        }
        else if(node->tag == Tag::Exprssion){
            return visitExprWithoutCatch(domain,node).first;
        }
        else{
            throw "unimplement";
        }
    }

    void
    TypeAnalyzer::visitParameterList(std::shared_ptr<Type::Domain> domain, std::shared_ptr<Type::Function> function,
                                     std::shared_ptr<Node> node) {

    }

    TypeAnalyzer::TypeAnalyzer(const shared_ptr<SymbolTable> table):table(table) {}


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

    std::shared_ptr<Semantics> Semantics::instance;

    BuiltIn &Semantics::getBuiltIn() {
        return *builtIn;
    }

    std::shared_ptr<Semantics> Semantics::Instance() {
        if(!instance)instance.reset(new Semantics());
        return instance;
    }

    ConversionRules &Semantics::getConversionRules() {
        return *conversionRules;
    }

    Semantics::Semantics() {
        builtIn = new BuiltIn();
        conversionRules = new ConversionRules(builtIn);
    }

    Semantics::~Semantics() {
        delete builtIn;
        delete conversionRules;
    }
}