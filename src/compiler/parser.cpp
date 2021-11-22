//
// Created by yorkin on 10/27/21.
//

#include "parser.h"
#include "logger.h"
#include "formator.h"
using namespace std;
using namespace evoBasic::ast;
using namespace evoBasic::ast::stmt;
using namespace evoBasic::ast::expr;

namespace evoBasic{
#define statement_follows Token::for_,Token::let_,Token::select_,Token::if_,Token::while_, \
        Token::ID,Token::DIGIT,Token::DECIMAL,Token::STRING,Token::CHAR,Token::MINUS,Token::ADD, \
        Token::return_,Token::exit_,Token::continue_

    set<Token::Enum> combine(set<Token::Enum> a,set<Token::Enum> b){
        a.merge(b);
        return a;
    }

    Parser::Parser(Lexer *lexer):lexer(lexer){}
    
    ast::Global *Parser::parseGlobal(){
        set<Token::Enum> member_follows = {
                Token::function_,Token::init_,Token::operator_,Token::type_,
                Token::enum_,Token::dim_,Token::END_CLASS,Token::EOF_
        };

        auto global = new Global;
        while(!lexer->predict(Token::EOF_)){
            auto member = parseMember(member_follows);
            if(member->member_kind == Member::init_ || member->member_kind == Member::operator_){
                Logger::error(member->location,"Init or Operator cannot be declared in Global");
            }
            else global->member_list.push_back(member);
        }

        global->location = global->member_list.front()->location;
        lexer->match(Token::EOF_);
        return global;
    }

    ast::Class *Parser::parseClass(const set<Token::Enum>& follows){
        auto member_follows = combine(follows,{
                Token::function_,Token::init_,Token::operator_,Token::type_,
                Token::enum_,Token::dim_,Token::END_CLASS
        });

        auto cls = new Class;
        lexer->match(Token::class_);
        cls->name = parseID(combine(member_follows,{Token::extend_,Token::impl,Token::COMMA}));
        cls->location = cls->name->location;

        if(lexer->predict(Token::extend_)){
            cls->extend = parseAnnotation(combine(follows,{Token::impl,Token::COMMA}));
        }

        if(lexer->predict(Token::impl)){
            auto impl_follows = combine(follows,{Token::COMMA});
            cls->impl_list.push_back(parseAnnotation(impl_follows));
            while(lexer->predict(Token::COMMA)){
                lexer->match(Token::COMMA);
                cls->impl_list.push_back(parseAnnotation(impl_follows));
            }
        }

        while(!lexer->predict(Token::END_CLASS)){
            auto member = parseMember(member_follows);
            if(member->member_kind == Member::module_ || member->member_kind == Member::class_){
                Logger::error(member->location,"Module or Class can not be nested in Class");
            }
            else cls->member_list.push_back(member);
        }
        lexer->match(Token::END_CLASS,follows,"expected 'End Class'");
        return cls;
    }

    ast::Module *Parser::parseModule(const set<Token::Enum>& follows){
        auto member_follows = combine(follows,{
                Token::private_,Token::public_,Token::friend_,Token::protected_,
                Token::function_,Token::sub_,Token::init_,Token::operator_,Token::type_,
                Token::enum_,Token::dim_,Token::END_CLASS
        });

        auto mod = new Module;
        lexer->match(Token::module_);
        mod->name = parseID(member_follows);
        mod->location = mod->name->location;

        while(!lexer->predict(Token::END_MODULE)){
            auto member = parseMember(member_follows);
            if(!member)continue;
            if(member->member_kind == Member::init_ || member->member_kind == Member::operator_){
                Logger::error(member->location,"Init or Operator cannot be a member of the Module");
            }
            else mod->member_list.push_back(member);
        }

        lexer->match(Token::END_MODULE,follows,"expected 'End Module'");
        return mod;
    }

    ast::Type *Parser::parseType(const set<Token::Enum>& follows){
        auto member_follows = combine(follows,{Token::END_TYPE});

        auto type = new Type;
        lexer->match(Token::type_);
        type->name = parseID(member_follows);
        type->location = type->name->location;

        while(lexer->predict(Token::ID)){
            type->member_list.push_back(parseVariable(member_follows));
        }

        lexer->match(Token::END_TYPE,follows,"expected 'End Type'");
        return type;
    }

    ast::Enum *Parser::parseEnum(const set<Token::Enum>& follows){
        auto member_follows = combine(follows,{Token::END_ENUM});

        auto enum_ = new Enum;
        lexer->match(Token::enum_);
        enum_->name = parseID(member_follows);
        enum_->location = enum_->name->location;

        while(!lexer->predict(Token::END_ENUM)){
            EnumMember enum_member;
            enum_member.first = parseID(follows);
            if(lexer->predict(Token::ASSIGN)){
                lexer->match(Token::ASSIGN);
                enum_member.second = parseDigit(member_follows);
            }
            else enum_member.second = nullptr;
            enum_->member_list.push_back(enum_member);
        }
        lexer->match(Token::END_ENUM);
        return enum_;
    }



    ast::Member *Parser::parseMember(const set<Token::Enum>& follows){
        auto access = parseAccessFlag();
        Member *member = nullptr;
        switch (lexer->getNextToken()->getKind()) {
            case Token::override_:
            case Token::virtual_:
            case Token::static_:
            case Token::sub_:
            case Token::function_:
                member = parseFunction(follows);
                break;
            case Token::declare_:
                member = parseExternal(follows);
                break;
            case Token::init_:
                member = parseInit(follows);
                break;
            case Token::operator_:
                member = parseOperator(follows);
                break;
            case Token::type_:
                member = parseType(follows);
                break;
            case Token::enum_:
                member = parseEnum(follows);
                break;
            case Token::dim_:
                member = parseDim(follows);
                break;
            case Token::class_:
                member = parseClass(follows);
                break;
            case Token::module_:
                member = parseModule(follows);
                break;
            default:
                Logger::error(lexer->getNextToken()->getLocation(),"unexpected token");
                lexer->skipUntil(follows);
        }
        if(member)member->access = access;
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

    ast::Import *Parser::parseImport(const set<Token::Enum>& follows){
        auto import = new Import;
        lexer->match(Token::import_);
        import->location = lexer->getToken()->getLocation();
        import->annotation = parseAnnotation(follows);
        return import;
    }
    ast::Dim *Parser::parseDim(const set<Token::Enum>& follows){
        auto member_follows = combine(follows,{Token::COMMA});
        auto dim = new Dim;
        lexer->match(Token::dim_);
        dim->location = lexer->getToken()->getLocation();
        dim->variable_list.push_back(parseVariable(member_follows));
        while(lexer->predict(Token::COMMA)){
            lexer->match(Token::COMMA);
            dim->variable_list.push_back(parseVariable(member_follows));
        }
        return dim;
    }
    ast::Variable *Parser::parseVariable(const set<Token::Enum>& follows){
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
    ast::Function *Parser::parseFunction(const set<Token::Enum>& follows){
        auto stmt_follows = combine(follows,{Token::END_FUNCTION,Token::END_SUB});

        auto func = new Function;

        switch(lexer->getNextToken()->getKind()){
            case Token::static_:
                lexer->match(Token::static_);
                func->method_flag = MethodFlag::Static;
                break;
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
            func->parameter_list = parseParameterList(stmt_follows);
            func->return_type = nullptr;
            func->statement_list = parseStmtList(stmt_follows);
            lexer->match(Token::END_SUB,follows,"expected 'End Sub'");
        }
        else if(lexer->predict(Token::function_)){
            lexer->match(Token::function_);
            func->name = parseID(stmt_follows);
            func->location = func->name->location;
            func->parameter_list = parseParameterList(combine(stmt_follows,{Token::as_}));
            lexer->match(Token::as_);
            func->return_type = parseAnnotation(stmt_follows);
            func->statement_list = parseStmtList(stmt_follows);
            lexer->match(Token::END_FUNCTION,follows,"expected 'End Function'");
        }
        return func;
    }
    ast::External *Parser::parseExternal(const set<Token::Enum>& follows){
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
            ext->parameter_list = parseParameterList(combine(follows,{Token::as_}));
            lexer->match(Token::as_);
            ext->return_annotation = parseAnnotation(follows);
        }
        else{
            ext->parameter_list = parseParameterList(follows);
            ext->return_annotation = nullptr;
        }
        return ext;
    }
    ast::Init *Parser::parseInit(const set<Token::Enum>& follows){
        auto init = new Init;
        lexer->match(Token::init_);
        init->location = lexer->getToken()->getLocation();
        init->parameter_list = parseParameterList(follows);
        init->statement_list = parseStmtList(follows);
        lexer->match(Token::END_INIT);
        return init;
    }

    ast::Operator *Parser::parseOperator(const set<Token::Enum>& follows){
        auto operator_ = new Operator;
        lexer->match(Token::operator_);
        operator_->location = lexer->getToken()->getLocation();
        operator_->parameter_list = parseParameterList(combine(follows,{Token::as_,Token::END_OPERATOR,statement_follows}));
        if(lexer->predict(Token::as_)){
            lexer->match(Token::as_);
            operator_->return_annotation = parseAnnotation(combine(follows,{Token::END_OPERATOR,statement_follows}));
        }
        else {
            operator_->return_annotation = nullptr;
            Logger::error(lexer->getNextToken()->getLocation(),"overloaded operator must have a return type");
        }
        operator_->statement_list = parseStmtList(combine(follows,{Token::END_OPERATOR}));
    }

    ast::EnumMember Parser::parseEnumMember(const set<Token::Enum>& follows){
        EnumMember member;
        member.first = parseID(combine(follows,{Token::ASSIGN}));
        if(lexer->predict(Token::ASSIGN)){
            lexer->match(Token::ASSIGN);
            member.second = parseDigit(combine(follows,{Token::ID}));
        }
        else member.second = nullptr;
        return member;
    }

    std::list<ast::Parameter*> Parser::parseParameterList(const set<Token::Enum>& follows){
        auto member_follows = combine(follows,{Token::RP, Token::COMMA});
        list<ast::Parameter*> ls;
        lexer->match(Token::LP);
        if(!lexer->predict(Token::RP)){
            ls.push_back(parseParameter(member_follows));
            while(lexer->predict(Token::COMMA)){
                lexer->match(Token::COMMA);
                ls.push_back(parseParameter(member_follows));
            }
        }
        lexer->match(Token::RP, member_follows, "expected ')' after parameter list");
        return ls;
    }

    std::list<ast::stmt::Statement*> Parser::parseStmtList(const set<Token::Enum>& follows){
        set<Token::Enum> stmt_follows = {statement_follows};
        auto member_follows = combine(follows,stmt_follows);

        list<Statement*> ls;
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
            ls.push_back(statement);
        }
        return ls;
    }

    ast::Parameter *Parser::parseParameter(const set<Token::Enum>& follows){
        auto param = new Parameter;
        if(lexer->predict(Token::optional_)){
            lexer->match(Token::optional_);
            param->is_optional = true;
        }
        else param->is_optional = false;

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
        return param;
    }

    ast::expr::ID *Parser::parseID(const set<Token::Enum>& follows){
        lexer->match(Token::ID);
        auto id = new ID;
        id->location = lexer->getToken()->getLocation();
        id->lexeme = lexer->getToken()->getLemexe();
        return id;
    }

    ast::stmt::Let *Parser::parseLet(const set<Token::Enum>& follows){
        auto member_follows = combine(follows,{Token::COMMA});

        auto let = new Let;
        lexer->match(Token::let_);
        let->location = lexer->getToken()->getLocation();
        let->variable_list.push_back(parseVariable(member_follows));
        while(lexer->predict(Token::COMMA)){
            lexer->match(Token::COMMA);
            let->variable_list.push_back(parseVariable(member_follows));
        }
        return let;
    }

    ast::stmt::Select *Parser::parseSelect(const set<Token::Enum>& follows){
        auto member_follows = combine(follows,{Token::case_,Token::END_SELECT});

        auto select = new Select;
        lexer->match(Token::select_);
        if(lexer->predict(Token::case_))lexer->match(Token::case_);
        select->condition = parseLogic(member_follows);
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

            case_->statement_list = parseStmtList(member_follows);
            select->case_list.push_back(case_);
        }
        lexer->match(Token::END_SELECT,follows,"expected 'End Select'");
        return select;
    }

    ast::stmt::Loop *Parser::parseLoop(const set<Token::Enum>& follows){
        auto member_follows = combine(follows,{Token::wend_});

        auto loop = new Loop;
        lexer->match(Token::while_);
        loop->location = lexer->getToken()->getLocation();
        loop->condition = parseLogic(member_follows);
        loop->statement_list = parseStmtList(member_follows);
        lexer->match(Token::wend_,follows,"expected 'Wend'");
        return loop;
    }

    ast::stmt::If *Parser::parseIf(const set<Token::Enum>& follows){
        auto member_follows = combine(follows,{Token::END_IF,Token::else_,Token::elseif_});

        auto if_ = new If;
        bool looping = true;
        while(looping){
            auto case_ = new Case;
            switch (lexer->getNextToken()->getKind()) {
                case Token::if_:
                    lexer->match(Token::if_);
                    if_->location = case_->location = lexer->getToken()->getLocation();
                    case_->condition = parseLogic(member_follows);
                    lexer->match(Token::then_);
                    case_->statement_list = parseStmtList(member_follows);
                    break;
                case Token::elseif_:
                    lexer->match(Token::elseif_);
                    case_->location = lexer->getToken()->getLocation();
                    case_->condition = parseLogic(member_follows);
                    lexer->match(Token::then_);
                    case_->statement_list = parseStmtList(member_follows);
                    break;
                case Token::else_:
                    lexer->match(Token::else_);
                    case_->location = lexer->getToken()->getLocation();
                    case_->condition = nullptr;
                    case_->statement_list = parseStmtList(member_follows);
                    break;
                default:
                    looping = false;
            }
            if(looping)if_->case_list.push_back(case_);
        }
        lexer->match(Token::END_IF,follows,"expected 'End If'");
        return if_;
    }

    ast::stmt::For *Parser::parseFor(const set<Token::Enum>& follows){
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

        for_->statement_list = parseStmtList(combine(follows,{Token::next_}));
        lexer->match(Token::next_,follows,"Expected 'Next'");
        return for_;
    }

    ast::stmt::Return *Parser::parseReturn(const set<Token::Enum>& follows){
        auto tmp = new Return;
        lexer->match(Token::return_);
        tmp->location = lexer->getToken()->getLocation();
        tmp->expr = parseLogic(follows);
        return tmp;
    }

    ast::stmt::Exit *Parser::parseExit(const set<Token::Enum>& follows){
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

    ast::expr::Expression *Parser::parseLogic(const set<Token::Enum>& follows){
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

    ast::expr::Expression *Parser::parseCmp(const set<Token::Enum>& follows){
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

    ast::expr::Expression *Parser::parseAdd(const set<Token::Enum>& follows){
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

    ast::expr::Expression *Parser::parseTerm(const set<Token::Enum>& follows){
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

    ast::expr::Expression *Parser::parseFactor(const set<Token::Enum>& follows){
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
                    return new Binary(lhs,Binary::ASSIGN, parseLogic(addition_follows));
                    break;
                default:
                    return lhs;
            }
        }
    }

    ast::expr::Expression *Parser::parseUnary(const set<Token::Enum>& follows){
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

    ast::expr::Expression *Parser::parseDot(const set<Token::Enum>& follows) {
        auto addition_follows = combine(follows,{Token::DOT});

        Expression *rhs,*lhs = parseTerminal(follows);
        while(true){
            if(lexer->predict(Token::DOT)){
                lexer->match(Token::DOT);
                rhs = parseTerminal(follows);
                lhs = new Binary(lhs,Binary::Dot,rhs);
            }
            else{
                return lhs;
            }
        }

    }

    ast::expr::Expression *Parser::parseTerminal(const set<Token::Enum>& follows){
        auto addition_follows = combine(follows,{Token::DOT});

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
                default:
                    Logger::error(lexer->getNextToken()->getLocation(),"Unexpected token");
                    lexer->skipUntil(addition_follows);
            }
        }
    }

    ast::expr::Expression *Parser::parseUnit(const set<Token::Enum>& follows){
        auto begin_location = lexer->getNextToken()->getLocation();
        Expression *ret = parseID(follows);

        if(lexer->predict(Token::LP)){
            auto callee = new Callee;
            callee->name = (ID*)ret;
            callee->arg_list = parseArgsList(follows);
            ret = callee;
        }

        if(lexer->predict(Token::LB)){
            lexer->match(Token::LB);
            auto index = new Binary(ret,Binary::Index,parseLogic(combine(follows,{Token::RB})));
            lexer->match(Token::RB);
            ret = index;
        }

        ret->location = new Location(begin_location,lexer->getToken()->getLocation());
        return ret;
    }


    std::vector<ast::expr::Callee::Argument*> Parser::parseArgsList(const set<Token::Enum>& follows){
        auto addition_follows = combine(follows,{Token::COMMA});

        vector<Callee::Argument*> args;
        lexer->match(Token::LP);

        if(!lexer->predict(Token::RP)){
            args.push_back(parseArg(addition_follows));

            while(lexer->predict(Token::COMMA)){
                lexer->match(Token::COMMA);
                args.push_back(parseArg(addition_follows));
            }
        }

        lexer->match(Token::RP);
        return args;
    }


    ast::expr::Callee::Argument *Parser::parseArg(const set<Token::Enum>& follows) {
        auto arg = new Callee::Argument;
        auto begin_location = lexer->getNextToken()->getLocation();
        if(lexer->predict(Token::byval_)){
            lexer->match(Token::byval_);
            arg->pass_kind = ast::expr::Callee::Argument::byval;
        }
        else if(lexer->predict(Token::byref_)){
            lexer->match(Token::byref_);
            arg->pass_kind = ast::expr::Callee::Argument::byref;
        }
        arg->expr = parseLogic(follows);
        arg->location = new Location(begin_location,lexer->getToken()->getLocation());
        return arg;
    }

    ast::expr::Digit *Parser::parseDigit(const set<Token::Enum>& follows){
        if(lexer->predict(Token::DIGIT)){
            lexer->match(Token::DIGIT);
            auto digit = new expr::Digit;
            digit->location = lexer->getToken()->getLocation();
            digit->value = stoi(lexer->getToken()->getLemexe());
            return digit;
        }
        else{
            Logger::error(lexer->getToken()->getLocation(),"missing digit here");
            return nullptr;
        }
    }

    ast::expr::Decimal *Parser::parseDecimal(const set<Token::Enum>& follows){
        lexer->match(Token::DECIMAL);
        auto decimal = new expr::Decimal;
        decimal->location = lexer->getToken()->getLocation();
        decimal->value = stod(lexer->getToken()->getLemexe());
        return decimal;
    }

    ast::expr::String *Parser::parseString(const set<Token::Enum>& follows){
        lexer->match(Token::STRING);
        auto str = new expr::String;
        str->location = lexer->getToken()->getLocation();
        auto &lexeme = lexer->getToken()->getLemexe();
        str->value = lexeme.substr(1,lexeme.size()-2);
        return str;
    }

    ast::expr::Char *Parser::parseChar(const set<Token::Enum>& follows){
        lexer->match(Token::CHAR);
        auto ch = new expr::Char;
        ch->location = lexer->getToken()->getLocation();
        auto &lexeme = lexer->getToken()->getLemexe();
        ch->value = lexer->getToken()->getLemexe().front();
        return ch;
    }

    ast::expr::Boolean *Parser::parseBoolean(const set<Token::Enum>& follows){
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

    ast::expr::Expression *Parser::parseParentheses(const set<Token::Enum>& follows) {
        lexer->match(Token::LP);
        auto location_begin = lexer->getToken()->getLocation();
        auto parentheses = new ast::expr::Parentheses;
        parentheses->expr = parseLogic(follows);
        parentheses->location = new Location(location_begin,lexer->getNextToken()->getLocation());
        lexer->match(Token::RP, follows, "Expected token ')'");
        return parentheses;
    }

    ast::stmt::ExprStmt *Parser::parseExprStmt(const set<Token::Enum>& follows) {
        auto tmp = new ExprStmt;
        tmp->expr = parseLogic(follows);
        return tmp;
    }

    ast::Annotation *Parser::parseAnnotation(const set<Token::Enum> &follows) {
        auto annotation = new Annotation;
        auto begin_location = lexer->getToken()->getLocation();
        annotation->unit_list.push_back(parseAnnotationUnit(follows));
        while(lexer->predict(Token::DOT)){
            lexer->match(Token::DOT);
            annotation->unit_list.push_back(parseAnnotationUnit(follows));
        }
        if(lexer->predict(Token::LB)){
            lexer->match(Token::LB);
            annotation->array_size = parseDigit(combine(follows,{Token::RB}));
            lexer->match(Token::RB);
        }
        annotation->location = new Location(begin_location,lexer->getToken()->getLocation());
        return annotation;
    }

    ast::AnnotationUnit *Parser::parseAnnotationUnit(const set<Token::Enum> &follows) {
        auto unit = new AnnotationUnit;
        auto begin_location = lexer->getNextToken()->getLocation();
        unit->name = parseID(follows);
        unit->location = new Location(begin_location,lexer->getToken()->getLocation());
        return unit;
    }


    ast::Annotation *constructAnnotationAST(std::string code) {
        auto source = new StringSource(std::move(code));
        Lexer lexer(source);
        Parser parser(&lexer);
        auto ast = parser.parseAnnotation({Token::EOF_});
        return ast;
    }
}