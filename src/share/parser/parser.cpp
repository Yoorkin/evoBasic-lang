//
// Created by yorkin on 10/27/21.
//

#include "parser.h"
#include <utils/logger.h>
#include <utils/format.h>
using namespace std;
using namespace evoBasic::parseTree;
using namespace evoBasic::parseTree::stmt;
using namespace evoBasic::parseTree::expr;

namespace evoBasic{
#define statement_follows Token::for_,Token::let_,Token::select_,Token::if_,Token::while_, \
        Token::ID,Token::DIGIT,Token::DECIMAL,Token::STRING,Token::CHAR,Token::MINUS,Token::ADD, \
        Token::return_,Token::exit_,Token::continue_

    set<Token::Enum> combine(set<Token::Enum> a,set<Token::Enum> b){
        a.merge(b);
        return a;
    }

    Parser::Parser(Lexer *lexer):lexer(lexer){}
    
    parseTree::Global *Parser::parseGlobal(){
        set<Token::Enum> member_follows = {
                Token::function_,Token::operator_,Token::type_,
                Token::enum_,Token::dim_,Token::END_CLASS,Token::EOF_
        };

        auto global = new Global;
        Member *tail = nullptr;
        while(!lexer->predict(Token::EOF_)){
            Member *member = parseMember(member_follows);
            if(tail){
                tail->next_sibling = member;
                member->prv_sibling = tail;
                tail = tail->next_sibling;
            }
            else global->member = tail = member;
        }

        global->location = global->member->location;
        lexer->match(Token::EOF_);
        return global;
    }

    parseTree::Class *Parser::parseClass(Follows follows){
        auto member_follows = combine(follows,{
                Token::function_,Token::operator_,Token::type_,
                Token::enum_,Token::dim_,Token::END_CLASS
        });

        auto cls = new Class;
        lexer->match(Token::class_);
        cls->name = parseID(combine(member_follows,{Token::extend_,Token::impl,Token::COMMA}));
        cls->location = cls->name->location;

        if(lexer->predict(Token::extend_)){
            lexer->match(Token::extend_);
            cls->extend = parseAnnotation(combine(follows,{Token::impl,Token::COMMA}));
        }

        if(lexer->predict(Token::impl)){
            auto impl_follows = combine(follows,{Token::COMMA});

            cls->impl = parseImplement(impl_follows);
            Implement *tail = cls->impl;
            while(lexer->predict(Token::COMMA)){
                lexer->match(Token::COMMA);
                auto impl = parseImplement(impl_follows);
                tail->next_sibling = impl;
                impl->prv_sibling = tail;
                tail = tail->next_sibling;
            }
        }

        Member *tail = nullptr;
        while(!lexer->predict(Token::END_CLASS)){
            auto member = parseMember(member_follows);
            if(tail){
                tail->next_sibling = member;
                member->prv_sibling = tail;
                tail = tail->next_sibling;
            }
            else cls->member = tail = member;
        }
        lexer->match(Token::END_CLASS,follows,"expected 'End Class'");
        return cls;
    }

    parseTree::Module *Parser::parseModule(Follows follows){
        auto member_follows = combine(follows,{
                Token::private_,Token::public_,Token::friend_,Token::protected_,
                Token::function_,Token::sub_,Token::operator_,Token::type_,
                Token::enum_,Token::dim_,Token::END_CLASS
        });

        auto mod = new Module;
        lexer->match(Token::module_);
        mod->name = parseID(member_follows);
        mod->location = mod->name->location;

        Member *tail = nullptr;
        while(!lexer->predict(Token::END_MODULE)){
            auto member = parseMember(member_follows);
            if(tail){
                tail->next_sibling = member;
                member->prv_sibling = tail;
                tail = tail->next_sibling;
            }
            else mod->member = tail = member;
        }

        lexer->match(Token::END_MODULE,follows,"expected 'End Module'");
        return mod;
    }

    parseTree::Type *Parser::parseType(Follows follows){
        auto member_follows = combine(follows,{Token::END_TYPE});

        auto type = new Type;
        lexer->match(Token::type_);
        type->name = parseID(member_follows);
        type->location = type->name->location;

        Variable *tail = nullptr;
        while(lexer->predict(Token::ID)){
            auto variable = parseVariable(member_follows);
            if(tail){
                tail->next_sibling = variable;
                variable->prv_sibling = tail;
                tail = tail->next_sibling;
            }
            else type->member = tail = variable;
        }

        lexer->match(Token::END_TYPE,follows,"expected 'End Type'");
        return type;
    }

    parseTree::Enum *Parser::parseEnum(Follows follows){
        auto member_follows = combine(follows,{Token::END_ENUM});

        auto enum_ = new Enum;
        lexer->match(Token::enum_);
        enum_->name = parseID(member_follows);
        enum_->location = enum_->name->location;

        EnumMember *tail = nullptr;
        while(!lexer->predict(Token::END_ENUM)){
            auto member = parseEnumMember(follows);
            if(tail){
                tail->next_sibling = member;
                member->prv_sibling = tail;
                tail = tail->next_sibling;
            }
            else enum_->member = tail = member;
        }
        lexer->match(Token::END_ENUM);
        return enum_;
    }



    parseTree::Member *Parser::parseMember(Follows follows){
        auto access = parseAccessFlag();
        bool is_static = false;
        if(lexer->predict(Token::static_)){
            lexer->match(Token::static_);
            is_static = true;
        }
        Member *member = nullptr;
        switch (lexer->getNextToken()->getKind()) {
            case Token::override_:
            case Token::virtual_:
            case Token::sub_:
            case Token::function_:
                member = parseFunction(follows);
                break;
            case Token::declare_:
                member = parseExternal(follows);
                break;
            case Token::new_:
                member = parseConstructor(follows);
                break;
            case Token::type_:
                member = parseType(follows);
                break;
            case Token::enum_:
                member = parseEnum(follows);
                break;
            case Token::dim_:
            case Token::const_:
            case Token::ID:
                member = parseDim(follows);
                break;
            case Token::class_:
                member = parseClass(follows);
                break;
            case Token::module_:
                member = parseModule(follows);
                break;
            case Token::interface_:
                member = parseInterface(follows);
                break;
            default:
                Logger::error(lexer->getNextToken()->getLocation(),"unexpected token");
                lexer->skipUntil(follows);
        }
        if(member){
            member->access = access;
            member->is_static = is_static;
        }
        return member;
    }


    AccessFlag Parser::parseAccessFlag(){
        switch (lexer->getNextToken()->getKind()) {
            case Token::public_:
                lexer->match(Token::public_);
                return AccessFlag::Public;
            case Token::friend_:
                lexer->match(Token::friend_);
                return AccessFlag::Friend;
            case Token::protected_:
                lexer->match(Token::protected_);
                return AccessFlag::Protected;
            case Token::private_:
                lexer->match(Token::private_);
            default:
                 return AccessFlag::Private;
        }
    }

    parseTree::Import *Parser::parseImport(Follows follows){
        auto import = new Import;
        lexer->match(Token::import_);
        import->location = lexer->getToken()->getLocation();
        import->annotation = parseAnnotation(follows);
        return import;
    }
    parseTree::Dim *Parser::parseDim(Follows follows){
        auto member_follows = combine(follows,{Token::COMMA});
        auto dim = new Dim;

        if(lexer->predict(Token::const_)){
            lexer->match(Token::const_);
            dim->is_const = true;
        }
        else if(lexer->predict(Token::dim_)){
            lexer->match(Token::dim_);
        }

        dim->location = lexer->getToken()->getLocation();

        dim->variable = parseVariable(member_follows);
        Variable *tail = dim->variable;
        while(lexer->predict(Token::COMMA)){
            lexer->match(Token::COMMA);
            auto member = parseVariable(member_follows);
            tail->next_sibling = member;
            member->prv_sibling = tail;
            tail = tail->next_sibling;
        }
        return dim;
    }
    parseTree::Variable *Parser::parseVariable(Follows follows){
        auto var = new Variable;
        var->name = parseID(combine(follows,{Token::as_}));
        var->location = var->name->location;

        if(lexer->predict(Token::as_)){
            lexer->match(Token::as_);
            var->annotation = parseAnnotation(follows);
        }

        if(lexer->predict(Token::ASSIGN)){
            lexer->match(Token::ASSIGN);
            var->initial = parseLogic(follows);
        }

        return var;
    }
    parseTree::Function *Parser::parseFunction(Follows follows){
        auto stmt_follows = combine(follows,{Token::END_FUNCTION,Token::END_SUB});

        auto func = new Function;

        switch(lexer->getNextToken()->getKind()){
            case Token::override_:
                lexer->match(Token::override_);
                func->method_flag = MethodFlag::Override;
                break;
            case Token::virtual_:
                lexer->match(Token::virtual_);
                func->method_flag = MethodFlag::Virtual;
                break;
            default:
                func->method_flag = MethodFlag::None;
        }

        if(lexer->predict(Token::sub_)){
            lexer->match(Token::sub_);

            func->name = parseID(stmt_follows);
            func->location = func->name->location;
            func->parameter = parseParameterList(stmt_follows);
            func->return_annotation = nullptr;
            func->statement = parseStmtList(stmt_follows);
            lexer->match(Token::END_SUB,follows,"expected 'End Sub'");
        }
        else if(lexer->predict(Token::function_)){
            lexer->match(Token::function_);
            func->name = parseID(stmt_follows);
            func->location = func->name->location;
            func->parameter = parseParameterList(combine(stmt_follows,{Token::as_}));
            lexer->match(Token::as_);
            func->return_annotation = parseAnnotation(stmt_follows);
            func->statement = parseStmtList(stmt_follows);
            lexer->match(Token::END_FUNCTION,follows,"expected 'End Function'");
        }
        return func;
    }

    parseTree::External *Parser::parseExternal(Follows follows){
        auto ext = new External;
        lexer->match(Token::declare_);
        bool hasReturn = false;
        if(lexer->predict(Token::sub_)){
            lexer->match(Token::sub_);
            ext->name = parseID(follows);
            ext->location = ext->name->location;
        }
        else if(lexer->predict(Token::function_)){
            lexer->match(Token::function_);
            ext->name = parseID(follows);
            ext->location = ext->name->location;
            hasReturn = true;
        }

        if(lexer->predict(Token::lib_)){
            lexer->match(Token::lib_);
            ext->lib = parseString(combine(follows,{Token::LP,Token::alias_}));
        }
        if(lexer->predict(Token::alias_)){
            lexer->match(Token::alias_);
            ext->alias = parseString(combine(follows,{Token::LP}));
        }

        if(hasReturn){
            ext->parameter = parseParameterList(combine(follows,{Token::as_}));
            lexer->match(Token::as_);
            ext->return_annotation = parseAnnotation(follows);
        }
        else{
            ext->parameter = parseParameterList(follows);
            ext->return_annotation = nullptr;
        }
        return ext;
    }


    parseTree::Constructor *Parser::parseConstructor(const set<Token::Enum> &follows) {
        auto stmt_follows = combine(follows,{Token::END_NEW});
        auto constructor = new Constructor;

        lexer->match(Token::new_);
        constructor->location = lexer->getToken()->getLocation();
        constructor->parameter = parseParameterList(follows);

        constructor->statement = parseStmtList(stmt_follows);
        lexer->match(Token::END_NEW,follows,"expected 'End New'");
        return constructor;
    }

    parseTree::EnumMember *Parser::parseEnumMember(Follows follows){
        auto member = new EnumMember;
        member->name = parseID(combine(follows,{Token::ASSIGN}));
        auto begin_location = member->name->location;
        if(lexer->predict(Token::ASSIGN)){
            lexer->match(Token::ASSIGN);
            member->value = parseDigit(combine(follows,{Token::ID}));
        }
        else member->value = nullptr;
        member->location = new Location(begin_location,lexer->getToken()->getLocation());
        return member;
    }

    parseTree::Interface *Parser::parseInterface(const set<Token::Enum> &follows) {
        auto interface = new Interface;
        lexer->match(Token::interface_);
        interface->name = parseID(combine(follows,{Token::function_,Token::sub_,Token::END_INTERFACE}));
        interface->location = interface->name->location;
        Member *tail = nullptr;
        while(!lexer->predict(Token::END_INTERFACE)){
            Function *func = nullptr;
            switch (lexer->getNextToken()->getKind()) {
                case Token::sub_:
                    func = parseSubInterface(follows);
                    break;
                case Token::function_:
                    func = parseFunctionInterface(follows);
                    break;
            }
            if(tail){
                tail->next_sibling = func;
                func->prv_sibling = tail;
                tail = tail->next_sibling;
            }
            else{
                tail = func;
                interface->function = func;
            }
        }
        lexer->match(Token::END_INTERFACE);
        return interface;
    }


    parseTree::Function *Parser::parseFunctionInterface(const set<Token::Enum> &follows) {
        auto func = new Function;
        lexer->match(Token::function_);
        func->name = parseID(follows);
        func->location = func->name->location;
        func->parameter = parseParameterList(combine(follows,{Token::as_}));
        lexer->match(Token::as_);
        func->return_annotation = parseAnnotation(follows);
        func->method_flag = MethodFlag::Virtual;
        return func;
    }

    parseTree::Function *Parser::parseSubInterface(const set<Token::Enum> &follows) {
        auto func = new Function;
        lexer->match(Token::sub_);
        func->name = parseID(follows);
        func->location = func->name->location;
        func->parameter = parseParameterList(follows);
        func->return_annotation = nullptr;
        func->method_flag = MethodFlag::Virtual;
        return func;
    }



    parseTree::Parameter* Parser::parseParameterList(Follows follows){
        auto member_follows = combine(follows,{Token::RP, Token::COMMA});

        lexer->match(Token::LP);
        Parameter *head = nullptr,*tail = nullptr;
        if(!lexer->predict(Token::RP)){
            head = tail = parseParameter(member_follows);
            while(lexer->predict(Token::COMMA)){
                lexer->match(Token::COMMA);
                auto param = parseParameter(member_follows);
                tail->next_sibling = param;
                param->prv_sibling = tail;
                tail = tail->next_sibling;
            }
        }
        lexer->match(Token::RP, member_follows, "expected ')' after parameter list");
        return head;
    }

    parseTree::stmt::Statement* Parser::parseStmtList(Follows follows){
        set<Token::Enum> stmt_follows = {statement_follows};
        auto member_follows = combine(follows,stmt_follows);

        Statement *head = nullptr,*tail = nullptr;
        while(stmt_follows.contains(lexer->getNextToken()->getKind())){
            Statement *statement;
            switch (lexer->getNextToken()->getKind()) {
                case Token::for_:
                    statement = parseFor(member_follows);
                    break;
                case Token::while_:
                    statement = parseLoop(member_follows);
                    break;
                case Token::let_:
                    statement = parseLet(member_follows);
                    break;
                case Token::select_:
                    statement = parseSelect(member_follows);
                    break;
                case Token::if_:
                    statement = parseIf(member_follows);
                    break;
                case Token::ID:
                case Token::DIGIT:
                case Token::DECIMAL:
                case Token::STRING:
                case Token::CHAR:
                case Token::MINUS:
                case Token::ADD:
                    statement = parseExprStmt(member_follows);
                    break;
                case Token::exit_:
                    statement = parseExit(member_follows);
                    break;
                case Token::return_:
                    statement = parseReturn(member_follows);
                    break;
                case Token::continue_:
                    lexer->match(Token::continue_);
                    statement = new Continue;
                    break;
                default:
                    Logger::error(lexer->getNextToken()->getLocation(),"unexpected token");
                    lexer->skipUntil(member_follows);
            }

            if(tail){
                tail->next_sibling = statement;
                statement->prv_sibling = tail;
                tail = tail->next_sibling;
            }
            else head = tail = statement;
        }
        return head;
    }

    parseTree::Parameter *Parser::parseParameter(Follows follows){
        auto param = new Parameter;

        switch (lexer->getNextToken()->getKind()) {
            case Token::optional_:
                lexer->match(Token::optional_);
                param->is_optional = true;
                break;
            case Token::paramArray_:
                lexer->match(Token::paramArray_);
                param->is_param_array = true;
                break;
        }

        switch (lexer->getNextToken()->getKind()) {
            case Token::byval_:
                lexer->match(Token::byval_);
                param->is_byval = true;
                break;
            case Token::byref_:
                lexer->match(Token::byref_);
            default:
                param->is_byval = false;
        }

        param->name = parseID(follows);
        param->location = param->name->location;
        lexer->match(Token::as_);
        param->annotation = parseAnnotation(follows);

        if(lexer->predict(Token::ASSIGN)){
            lexer->match(Token::ASSIGN);
            param->initial = parseLogic(combine(follows,{Token::COMMA}));
        }

        return param;
    }

    parseTree::expr::ID *Parser::parseID(Follows follows){
        lexer->match(Token::ID);
        auto id = new ID;
        id->location = lexer->getToken()->getLocation();
        id->lexeme = lexer->getToken()->getLexeme();
        return id;
    }

    parseTree::stmt::Let *Parser::parseLet(Follows follows){
        auto member_follows = combine(follows,{Token::COMMA});

        auto let = new Let;
        lexer->match(Token::let_);
        let->location = lexer->getToken()->getLocation();

        let->variable = parseVariable(member_follows);
        Variable *tail = let->variable;
        while(lexer->predict(Token::COMMA)){
            lexer->match(Token::COMMA);
            auto variable = parseVariable(member_follows);
            tail->next_sibling = variable;
            variable->prv_sibling = tail;
            tail = tail->next_sibling;
        }
        return let;
    }

    parseTree::stmt::Select *Parser::parseSelect(Follows follows){
        auto member_follows = combine(follows,{Token::case_,Token::END_SELECT});

        auto select = new Select;
        lexer->match(Token::select_);
        if(lexer->predict(Token::case_))lexer->match(Token::case_);
        select->condition = parseLogic(member_follows);

        Case *tail = nullptr;
        while(lexer->predict(Token::case_)){
            auto case_ = new Case;
            lexer->match(Token::case_);
            case_->location = lexer->getToken()->getLocation();

            if(lexer->predict(Token::else_)) {
                lexer->match(Token::else_);
                case_->condition = nullptr;
            }
            else {
                case_->condition = parseLogic(member_follows);
            }

            case_->statement = parseStmtList(member_follows);

            if(tail){
                tail->next_sibling = case_;
                case_->prv_sibling = tail;
                tail = tail->next_sibling;
            }
            else select->case_ = tail = case_;
        }
        lexer->match(Token::END_SELECT,follows,"expected 'End Select'");
        return select;
    }

    parseTree::stmt::Loop *Parser::parseLoop(Follows follows){
        auto member_follows = combine(follows,{Token::wend_});

        auto loop = new Loop;
        lexer->match(Token::while_);
        loop->location = lexer->getToken()->getLocation();
        loop->condition = parseLogic(member_follows);
        loop->statement = parseStmtList(member_follows);
        lexer->match(Token::wend_,follows,"expected 'Wend'");
        return loop;
    }

    parseTree::stmt::If *Parser::parseIf(Follows follows){
        auto member_follows = combine(follows,{Token::END_IF,Token::else_,Token::elseif_});

        auto if_ = new If;
        bool looping = true;

        Case *tail = nullptr;
        while(looping){
            auto case_ = new Case;
            switch (lexer->getNextToken()->getKind()) {
                case Token::if_:
                    lexer->match(Token::if_);
                    if_->location = case_->location = lexer->getToken()->getLocation();
                    case_->condition = parseLogic(member_follows);
                    lexer->match(Token::then_);
                    case_->statement = parseStmtList(member_follows);
                    break;
                case Token::elseif_:
                    lexer->match(Token::elseif_);
                    case_->location = lexer->getToken()->getLocation();
                    case_->condition = parseLogic(member_follows);
                    lexer->match(Token::then_);
                    case_->statement = parseStmtList(member_follows);
                    break;
                case Token::else_:
                    lexer->match(Token::else_);
                    case_->location = lexer->getToken()->getLocation();
                    case_->condition = nullptr;
                    case_->statement = parseStmtList(member_follows);
                    break;
                default:
                    looping = false;
            }
            if(looping){
                if(tail){
                    tail->next_sibling = case_;
                    case_->prv_sibling = tail;
                    tail = tail->next_sibling;
                }
                else if_->case_ = tail = case_;
            }
        }
        lexer->match(Token::END_IF,follows,"expected 'End If'");
        return if_;
    }

    parseTree::stmt::For *Parser::parseFor(Follows follows){
        auto addition_follows = combine(follows, {Token::next_,statement_follows});

        auto for_ = new For;
        lexer->match(Token::for_);
        for_->location = lexer->getToken()->getLocation();
        if(lexer->predict(Token::let_)){
            lexer->match(Token::let_);
            for_->iterator_has_let = true;
        }
        else for_->iterator_has_let = false;

        for_->iterator = parseTerminal(combine(addition_follows, {Token::ASSIGN}));
        lexer->match(Token::ASSIGN);
        for_->begin = parseLogic(combine(addition_follows, {Token::to_}));
        lexer->match(Token::to_);
        for_->end = parseLogic(combine(addition_follows, {Token::step_}));
        if(lexer->predict(Token::step_)){
            lexer->match(Token::step_);
            for_->step = parseLogic(addition_follows);
        }

        for_->statement = parseStmtList(combine(follows,{Token::next_}));
        lexer->match(Token::next_,follows,"Expected 'Next'");
        return for_;
    }

    parseTree::stmt::Return *Parser::parseReturn(Follows follows){
        auto tmp = new Return;
        lexer->match(Token::return_);
        tmp->location = lexer->getToken()->getLocation();
        tmp->expr = parseLogic(follows);
        return tmp;
    }

    parseTree::stmt::Exit *Parser::parseExit(Follows follows){
        auto tmp = new Exit;
        lexer->match(Token::exit_);
        tmp->location = lexer->getToken()->getLocation();
        switch (lexer->getNextToken()->getKind()) {
            case Token::for_:
                lexer->match(Token::for_);
                tmp->exit_flag = Exit::For;
                break;
            case Token::while_:
                lexer->match(Token::while_);
                tmp->exit_flag = Exit::While;
                break;
            case Token::sub_:
                lexer->match(Token::sub_);
                tmp->exit_flag = Exit::Sub;
                break;
        }
        return tmp;
    }

    parseTree::expr::Expression *Parser::parseLogic(Follows follows){
        auto addition_follows = combine(follows,{Token::and_,Token::or_,Token::xor_,Token::not_});

        Expression *rhs,*lhs = parseCmp(addition_follows);
        while(true){
            switch (lexer->getNextToken()->getKind()) {
                case Token::and_:
                    lexer->match(Token::and_);
                    rhs = parseCmp(addition_follows);
                    lhs = new Binary(lhs,Binary::And,rhs);
                    break;
                case Token::or_:
                    lexer->match(Token::or_);
                    rhs = parseCmp(addition_follows);
                    lhs = new Binary(lhs,Binary::Or,rhs);
                    break;
                case Token::xor_:
                    lexer->match(Token::xor_);
                    rhs = parseCmp(addition_follows);
                    lhs = new Binary(lhs,Binary::Xor,rhs);
                    break;
                default:
                    return lhs;
            }
        }
    }

    parseTree::expr::Expression *Parser::parseCmp(Follows follows){
        auto addition_follows = combine(follows,{Token::EQ,Token::NE,Token::LT,Token::GT,Token::LE,Token::GE});

        Expression *rhs,*lhs = parseAdd(addition_follows);
        while(true){
            switch (lexer->getNextToken()->getKind()) {
                case Token::EQ:
                    lexer->match(Token::EQ);
                    rhs = parseAdd(addition_follows);
                    lhs = new Binary(lhs,Binary::EQ,rhs);
                    break;
                case Token::NE:
                    lexer->match(Token::NE);
                    rhs = parseAdd(addition_follows);
                    lhs = new Binary(lhs,Binary::NE,rhs);
                    break;
                case Token::LT:
                    lexer->match(Token::LT);
                    rhs = parseAdd(addition_follows);
                    lhs = new Binary(lhs,Binary::LT,rhs);
                    break;
                case Token::GT:
                    lexer->match(Token::GT);
                    rhs = parseAdd(addition_follows);
                    lhs = new Binary(lhs,Binary::GT,rhs);
                    break;
                case Token::LE:
                    lexer->match(Token::LE);
                    rhs = parseAdd(addition_follows);
                    lhs = new Binary(lhs,Binary::LE,rhs);
                    break;
                case Token::GE:
                    lexer->match(Token::GE);
                    rhs = parseAdd(addition_follows);
                    lhs = new Binary(lhs,Binary::GE,rhs);
                    break;
                default:
                    return lhs;
            }
        }
    }

    parseTree::expr::Expression *Parser::parseAdd(Follows follows){
        auto addition_follows = combine(follows,{Token::ADD,Token::MINUS});

        Expression *rhs,*lhs = parseTerm(addition_follows);
        while(true){
            switch (lexer->getNextToken()->getKind()) {
                case Token::ADD:
                    lexer->match(Token::ADD);
                    rhs = parseTerm(addition_follows);
                    lhs = new Binary(lhs,Binary::ADD,rhs);
                    break;
                case Token::MINUS:
                    lexer->match(Token::MINUS);
                    rhs = parseTerm(addition_follows);
                    lhs = new Binary(lhs,Binary::MINUS,rhs);
                    break;
                default:
                    return lhs;
            }
        }
    }

    parseTree::expr::Expression *Parser::parseTerm(Follows follows){
        auto addition_follows = combine(follows,{Token::MUL,Token::FDIV,Token::DIV});

        Expression *rhs,*lhs = parseFactor(addition_follows);
        while(true){
            switch (lexer->getNextToken()->getKind()) {
                case Token::MUL:
                    lexer->match(Token::MUL);
                    rhs = parseFactor(addition_follows);
                    lhs = new Binary(lhs,Binary::MUL,rhs);
                    break;
                case Token::FDIV:
                    lexer->match(Token::FDIV);
                    rhs = parseFactor(addition_follows);
                    lhs = new Binary(lhs,Binary::FDIV,rhs);
                    break;
                case Token::DIV:
                    lexer->match(Token::DIV);
                    rhs = parseFactor(addition_follows);
                    lhs = new Binary(lhs,Binary::DIV,rhs);
                    break;
                default:
                    return lhs;
            }
        }
    }

    parseTree::expr::Expression *Parser::parseFactor(Follows follows){
        auto addition_follows = combine(follows,{Token::as_,Token::ASSIGN});

        Expression *lhs = parseUnary(addition_follows);
        while(true){
            switch (lexer->getNextToken()->getKind()) {
                case Token::as_:
                    lexer->match(Token::as_);
                    lhs = new Cast(lhs,parseAnnotation(follows));
                    break;
                case Token::ASSIGN:
                    lexer->match(Token::ASSIGN);
                    //ASSIGN 语法树向右生长
                    return new Assign(lhs,parseLogic(addition_follows));
                    break;
                case Token::COLON:
                    lexer->match(Token::COLON);
                    return new Colon((ID*)lhs, parseLogic(addition_follows));
                default:
                    return lhs;
            }
        }
    }

    parseTree::expr::Expression *Parser::parseUnary(Follows follows){
        switch(lexer->getNextToken()->getKind()){
            case Token::MINUS:
                lexer->match(Token::MINUS);
                return new Unary(Unary::MINUS, parseUnary(follows));
            case Token::ADD:
                lexer->match(Token::ADD);
                return new Unary(Unary::ADD, parseUnary(follows));
            default:
                return parseDot(follows);
        }
    }

    parseTree::expr::Expression *Parser::parseDot(Follows follows) {
        auto addition_follows = combine(follows,{Token::DOT});

        Expression *rhs,*lhs = parseTerminal(follows);
        while(true){
            if(lexer->predict(Token::DOT)){
                lexer->match(Token::DOT);
                rhs = parseTerminal(follows);
                lhs = new Dot(lhs,rhs);
            }
            else{
                return lhs;
            }
        }

    }

    parseTree::expr::Expression *Parser::parseTerminal(Follows follows){
        auto addition_follows = combine(follows,{Token::DOT});
        bool panic = false;
        while(true){
            switch (lexer->getNextToken()->getKind()) {
                case Token::DIGIT:
                    return parseDigit(addition_follows);
                case Token::DECIMAL:
                    return parseDecimal(addition_follows);
                case Token::STRING:
                    return parseString(addition_follows);
                case Token::CHAR:
                    return parseChar(addition_follows);
                case Token::ID:
                    return parseUnit(addition_follows);
                case Token::true_:
                case Token::false_:
                    return parseBoolean(addition_follows);
                case Token::LP:
                    return parseParentheses(addition_follows);
                case Token::new_:
                    return parseNew(addition_follows);
                default:
                    if(panic){
                        auto err = new parseTree::expr::Expression;
                        err->location = lexer->getToken()->getLocation();
                        return err;
                    }
                    panic = true;
                    Logger::error(lexer->getNextToken()->getLocation(),"Unexpected token. Missing expression here");
                    lexer->skipUntil(addition_follows);
            }
        }
    }

    parseTree::expr::Expression *Parser::parseUnit(Follows follows){
        auto begin_location = lexer->getNextToken()->getLocation();
        Expression *ret = parseID(follows);

        if(lexer->predict(Token::LP)){
            auto callee = new Callee;
            callee->name = (ID*)ret;
            callee->argument = parseArgsList(follows);
            callee->location = new Location(ret->location,lexer->getToken()->getLocation());
            ret = callee;
        }

        if(lexer->predict(Token::LB)){
            lexer->match(Token::LB);
            auto location_begin = ret->location;
            ret = new Index(ret,parseLogic(combine(follows,{Token::RB})));
            lexer->match(Token::RB);
            ret->location = new Location(location_begin,lexer->getToken()->getLocation());
        }

        ret->location = new Location(begin_location,lexer->getToken()->getLocation());
        return ret;
    }


    parseTree::expr::Argument* Parser::parseArgsList(Follows follows){
        auto addition_follows = combine(follows,{Token::COMMA});

        lexer->match(Token::LP);
        Argument *tail = nullptr,*head = nullptr;

        if(!lexer->predict(Token::RP)){
            tail = head = parseArg(addition_follows);

            while(lexer->predict(Token::COMMA)){
                lexer->match(Token::COMMA);
                auto arg = parseArg(addition_follows);
                tail->next_sibling = arg;
                arg->prv_sibling = tail;
                tail = tail->next_sibling;
            }
        }

        lexer->match(Token::RP);
        return head;
    }


    parseTree::expr::Argument *Parser::parseArg(Follows follows) {
        auto arg = new Argument;
        auto begin_location = lexer->getNextToken()->getLocation();

        if(lexer->predict(Token::byval_)){
            lexer->match(Token::byval_);
            arg->pass_kind = parseTree::expr::Argument::byval;
        }
        else if(lexer->predict(Token::byref_)){
            lexer->match(Token::byref_);
            arg->pass_kind = parseTree::expr::Argument::byref;
        }
        arg->expr = parseLogic(follows);
        arg->location = new Location(begin_location,lexer->getToken()->getLocation());
        return arg;
    }

    parseTree::expr::Digit *Parser::parseDigit(Follows follows){
        if(lexer->predict(Token::DIGIT)){
            lexer->match(Token::DIGIT);
            auto digit = new expr::Digit;
            digit->location = lexer->getToken()->getLocation();
            digit->value = unicode::stoi(lexer->getToken()->getLexeme());
            return digit;
        }
        else{
            Logger::error(lexer->getToken()->getLocation(),"missing digit here");
            return nullptr;
        }
    }

    parseTree::expr::Decimal *Parser::parseDecimal(Follows follows){
        lexer->match(Token::DECIMAL);
        auto decimal = new expr::Decimal;
        decimal->location = lexer->getToken()->getLocation();
        decimal->value = unicode::stod(lexer->getToken()->getLexeme());
        return decimal;
    }

    parseTree::expr::String *Parser::parseString(Follows follows){
        lexer->match(Token::STRING);
        auto str = new expr::String;
        str->location = lexer->getToken()->getLocation();
        auto &lexeme = lexer->getToken()->getLexeme();
        str->value = lexeme.substr(1,lexeme.size()-2);
        return str;
    }

    parseTree::expr::Char *Parser::parseChar(Follows follows){
        lexer->match(Token::CHAR);
        auto ch = new expr::Char;
        ch->location = lexer->getToken()->getLocation();
        auto &lexeme = lexer->getToken()->getLexeme();
        ch->value = *(lexer->getToken()->getLexeme().begin());
        return ch;
    }

    parseTree::expr::Boolean *Parser::parseBoolean(Follows follows){
        auto boo = new Boolean;
        if(lexer->predict(Token::false_)){
            lexer->match(Token::false_);
            boo->value = false;
            boo->location = lexer->getToken()->getLocation();
        }
        else{
            lexer->match(Token::true_);
            boo->value = true;
            boo->location = lexer->getToken()->getLocation();
        }
        return boo;
    }

    parseTree::expr::Expression *Parser::parseParentheses(Follows follows) {
        lexer->match(Token::LP);
        auto location_begin = lexer->getToken()->getLocation();
        auto parentheses = new parseTree::expr::Parentheses;
        parentheses->expr = parseLogic(follows);
        parentheses->location = new Location(location_begin,lexer->getNextToken()->getLocation());
        lexer->match(Token::RP, follows, "Expected token ')'");
        return parentheses;
    }

    parseTree::stmt::ExprStmt *Parser::parseExprStmt(Follows follows) {
        auto tmp = new ExprStmt;
        tmp->expr = parseLogic(follows);
        return tmp;
    }

    parseTree::Annotation *Parser::parseAnnotation(const set<Token::Enum> &follows) {
        auto annotation = new Annotation;
        auto begin_location = lexer->getNextToken()->getLocation();

        annotation->unit = parseAnnotationUnit(follows);
        AnnotationUnit *tail = annotation->unit;

        while(lexer->predict(Token::DOT)){
            lexer->match(Token::DOT);
            auto unit = parseAnnotationUnit(follows);

            tail->next_sibling = unit;
            unit->prv_sibling = tail;
            tail = tail->next_sibling;
        }
        if(lexer->predict(Token::LB)){
            lexer->match(Token::LB);
            annotation->array_size = parseDigit(combine(follows,{Token::RB}));
            lexer->match(Token::RB);
        }
        annotation->location = new Location(begin_location,lexer->getToken()->getLocation());
        return annotation;
    }

    parseTree::AnnotationUnit *Parser::parseAnnotationUnit(const set<Token::Enum> &follows) {
        auto unit = new AnnotationUnit;
        auto begin_location = lexer->getNextToken()->getLocation();
        unit->name = parseID(follows);
        unit->location = new Location(begin_location,lexer->getToken()->getLocation());
        return unit;
    }


    parseTree::Implement *Parser::parseImplement(const std::set<Token::Enum> &follows) {
        auto impl = new Implement;
        lexer->match(Token::impl);
        impl->annotation = parseAnnotation(follows);
        impl->location = impl->annotation->location;
        return impl;
    }

    parseTree::expr::New *Parser::parseNew(const set<Token::Enum> &follows) {
        lexer->match(Token::new_);
        auto ret = new New;
        auto location_begin = lexer->getToken()->getLocation();
        ret->annotation = parseAnnotation(combine(follows,{Token::LP}));
        ret->argument = parseArgsList(follows);
        ret->location = new Location(location_begin,lexer->getToken()->getLocation());
        return ret;
    }

    parseTree::Annotation *constructAnnotationAST(std::string code) {
        auto source = new StringSource(std::move(code));
        Lexer lexer(source);
        Parser parser(&lexer);
        auto ast = parser.parseAnnotation({Token::EOF_});
        return ast;
    }

}