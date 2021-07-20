//
// Created by yorkin on 7/13/21.
//

#include "Parser.h"

namespace evoBasic{
    Node *Parser::moduleDecl(){
        lexer.match(Token::module_);
        auto ret = new Node(Tag::Module);
        ret->child.push_back(ID());
        while(lexer.getNextToken().token!=Token::end_){
            auto accessflag = accessFlag();
            auto body = moduleMember();
            auto member = new Node(Tag::ModuleMember,{accessflag,body});
            ret->child.push_back(member);
        }
        lexer.match(Token::end_);
        lexer.match(Token::module_);
        return ret;
    }

    Node *Parser::classDecl(){
        lexer.match(Token::class_);
        auto ret = new Node(Tag::Class);
        ret->child.push_back(ID());
        while(lexer.getNextToken().token!=Token::end_){
            auto accessflag = accessFlag();
            auto methodflag = methodFlag();
            auto body = classMember();
            auto member = new Node(Tag::ClassMember,{accessflag,methodflag,body});
            ret->child.push_back(member);
        }
        lexer.match(Token::end_);
        lexer.match(Token::class_);
        return ret;
    }

    Node *Parser::classMember(){
        switch (lexer.getNextToken().token) {
            case Token::function_: return functionDecl();
            case Token::sub_:      return subDecl();
            case Token::type_:     return typeDecl();
            case Token::enum_:     return enumDecl();
            case Token::let_:      return variableDecl();
        }
    }

    Node *Parser::moduleMember(){
        switch (lexer.getNextToken().token) {
            case Token::function_: return functionDecl();
            case Token::sub_:      return subDecl();
            case Token::type_:     return typeDecl();
            case Token::enum_:     return enumDecl();
            case Token::let_:      return variableDecl();
            case Token::class_:    return classDecl();
            case Token::module_:   return moduleDecl();
        }
    }

    Node *Parser::accessFlag(){
        switch (lexer.getNextToken().token) {
            case Token::public_:
                lexer.match(Token::public_);
                return new Node(Tag::Public);
            case Token::private_:
                lexer.match(Token::private_);
                return new Node(Tag::Private);
            case Token::friend_:
                lexer.match(Token::friend_);
                return new Node(Tag::Friend);
            default: return nullptr;
        }
    }

    Node *Parser::methodFlag(){
        switch(lexer.getNextToken().token){
            case Token::static_:
                lexer.match(Token::static_);
                return new Node(Tag::Static);
            case Token::virtual_:
                lexer.match(Token::virtual_);
                return new Node(Tag::Virtual);
            case Token::override_:
                lexer.match(Token::override_);
                return new Node(Tag::Override);
            default: return nullptr;
        }
    }

    Node *Parser::typeDecl(){
        lexer.match(Token::type_);
        auto ret = new Node(Tag::Type);
        ret->child.push_back(ID());
        while(lexer.getNextToken().token!=Token::end_){
            auto field = new Node(Tag::TypeField);
            field->child.push_back(ID());
            lexer.match(Token::as_);
            field->child.push_back(locating());
            ret->child.push_back(field);
        }
        lexer.match(Token::end_);
        lexer.match(Token::type_);
        return ret;
    }

    Node *Parser::enumDecl(){
        lexer.match(Token::enum_);
        auto node = new Node(Tag::Enum);
        node->child.push_back(ID());
        while(lexer.getNextToken().token!=Token::end_){
            auto member = new Node(Tag::EnumMember);
            member->child.push_back(ID());
            if(lexer.getNextToken().token==Token::ASSIGN){
                lexer.match(Token::ASSIGN);
                member->child.push_back(DIGIT());
            }
            node->child.push_back(member);
        }
        lexer.match(Token::end_);
        lexer.match(Token::enum_);
        return node;
    }

    Node *Parser::functionDecl(){
        lexer.match(Token::function_);
        lexer.match(Token::ID);
        auto id = new Node(Tag::ID,{{Attr::Lexeme,lexer.getToken().lexeme}});
        auto params = parameterList();
        lexer.match(Token::as_);
        auto retloca = locating();
        auto stmts = stmtSet();
        lexer.match(Token::end_);
        lexer.match(Token::function_);
        return new Node(Tag::Function,{id,params,retloca,stmts});
    }

    Node *Parser::subDecl(){
        lexer.match(Token::sub_);
        lexer.match(Token::ID);
        auto id = new Node(Tag::ID,{{Attr::Lexeme,lexer.getToken().lexeme}});
        auto params = parameterList();
        auto stmts = stmtSet();
        lexer.match(Token::end_);
        lexer.match(Token::sub_);
        return new Node(Tag::Sub,{id,params,stmts});
    }

    Node *Parser::parameterList(){
        auto node = new Node(Tag::ParameterList);
        lexer.match(Token::LB);
        while(true){
            auto param = new Node(Tag::Parameter);
            if(lexer.getNextToken().token==Token::optional_){
                lexer.match(Token::optional_);
                param->set(Attr::IsOptional,true);
            } else param->set(Attr::IsOptional,false);

            if(lexer.getNextToken().token==Token::byref_){
                lexer.match(Token::byref_);
                param->set(Attr::IsByval,false);
            }
            if(lexer.getNextToken().token==Token::byval_){
                lexer.match(Token::byval_);
                param->set(Attr::IsByval,true);
            }

            param->child.push_back(ID());
            lexer.match(Token::as_);
            param->child.push_back(locating());
            node->child.push_back(param);

            if(lexer.getNextToken().token==Token::COMMA)
                lexer.match(Token::COMMA);
            else break;
        }
        lexer.match(Token::RB);
        return node;
    }

    Node *Parser::ID(){
        lexer.match(Token::ID);
        return new Node(Tag::ID,{{Attr::Lexeme,lexer.getToken().lexeme}});
    }

    Node *Parser::DIGIT(){
        lexer.match(Token::DIGIT);
        return new Node(Tag::Digit,{{Attr::Lexeme,lexer.getToken().lexeme}});
    }

    Node *Parser::variableDecl(){
        lexer.match(Token::let_);
        auto ret = new Node(Tag::Let);
        while(true){
            auto var = new Node(Tag::Variable);
            lexer.match(Token::ID);
            var->child.push_back(new Node(Tag::ID,{{Attr::Lexeme,lexer.getToken().lexeme}}));
            if(lexer.getNextToken().token==Token::as_){
                lexer.match(Token::as_);
                var->child.push_back(locating());
            }
            if(lexer.getNextToken().token==Token::ASSIGN){
                lexer.match(Token::ASSIGN);
                var->child.push_back(expr());
            }
            ret->child.push_back(var);
            if(lexer.getNextToken().token!=Token::COMMA)break;
            lexer.match(Token::COMMA);
        }
        return ret;
    }

    Node *Parser::expr(){
        auto lhs = cmp();
        Node *rhs;
        while(true){
            switch(lexer.getNextToken().token){
                case Token::and_:
                    lexer.match(Token::and_);
                    rhs = cmp();
                    lhs = new Node(Tag::And,{lhs,rhs});
                    break;
                case Token::or_:
                    lexer.match(Token::or_);
                    rhs = cmp();
                    lhs = new Node(Tag::Or,{lhs,rhs});
                    break;
                case Token::xor_:
                    lexer.match(Token::xor_);
                    rhs = cmp();
                    lhs = new Node(Tag::Xor,{lhs,rhs});
                    break;
                default: return lhs;
            }
        }
    }

    Node *Parser::cmp(){
        Node *rhs,*lhs = add();
        while(true){
            switch(lexer.getNextToken().token){
                case Token::EQ:
                    lexer.match(Token::EQ);
                    rhs = add();
                    lhs = new Node(Tag::EQ,{lhs,rhs});
                    break;
                case Token::NE:
                    lexer.match(Token::NE);
                    rhs = add();
                    lhs = new Node(Tag::NE,{lhs,rhs});
                    break;
                case Token::LT:
                    lexer.match(Token::LT);
                    rhs = add();
                    lhs = new Node(Tag::LT,{lhs,rhs});
                    break;
                case Token::GT:
                    lexer.match(Token::GT);
                    rhs = add();
                    lhs = new Node(Tag::GT,{lhs,rhs});
                    break;
                case Token::LE:
                    lexer.match(Token::LE);
                    rhs = add();
                    lhs = new Node(Tag::LE,{lhs,rhs});
                    break;
                case Token::GE:
                    lexer.match(Token::GE);
                    rhs = add();
                    lhs = new Node(Tag::GE,{lhs,rhs});
                    break;
                default:return lhs;
            }
        }
    }

    Node *Parser::add(){
        Node *rhs,*lhs = term();
        while(true){
            switch(lexer.getNextToken().token){
                case Token::ADD:
                    lexer.match(Token::ADD);
                    rhs = term();
                    lhs = new Node(Tag::Add,{lhs,rhs});
                    break;
                case Token::MINUS:
                    lexer.match(Token::MINUS);
                    rhs = term();
                    lhs = new Node(Tag::Minus,{lhs,rhs});
                    break;
                default:return lhs;
            }
        }

    }

    Node *Parser::term(){
        Node *rhs,*lhs = factor();
        while(true){
            switch (lexer.getNextToken().token) {
                case Token::MUL:
                    lexer.match(Token::MUL);
                    rhs = factor();
                    lhs = new Node(Tag::Mul,{lhs,rhs});
                    break;
                case Token::FDIV:
                    lexer.match(Token::FDIV);
                    rhs = factor();
                    lhs = new Node(Tag::FDiv,{lhs,rhs});
                    break;
                case Token::DIV:
                    lexer.match(Token::DIV);
                    rhs = factor();
                    lhs = new Node(Tag::Div,{lhs,rhs});
                    break;
                default: return lhs;
            }
        }

    }

    Node *Parser::factor(){
        Node *rhs,*lhs = unit();
        while(true){
            if(lexer.getNextToken().token==Token::ASSIGN){
                lexer.match(Token::ASSIGN);
                lhs = new Node(Tag::Assign,{lhs,rhs});
            }
            else return lhs;
        }
    }

    Node *Parser::unit(){
        Node *rhs,*lhs;

        int minusCount=0,addCount=0;
        Token *alertMinus=nullptr,*alertAdd=nullptr;
        while(lexer.getNextToken().token==Token::MINUS || lexer.getNextToken().token==Token::ADD){
            if(lexer.getNextToken().token==Token::MINUS){
                lexer.match(Token::MINUS);
                minusCount++;
                if(minusCount>1)alertMinus=&lexer.getToken();
            }
            else{
                lexer.match(Token::ADD);
                addCount++;
                alertAdd=&lexer.getToken();
            }
        }

        if(alertMinus!=nullptr)logger.warning(*alertMinus,"傻逼吗 写了那么多负号干什么");
        if(alertAdd!=nullptr)logger.warning(*alertMinus,"傻逼吗 多写正号干什么");

        if(minusCount%2==1)lhs = new Node(Tag::SelfNeg,{terminal()});
        else lhs = terminal();

        while(true){
            if(lexer.getNextToken().token==Token::DOT){
                lexer.match(Token::DOT);
                rhs = terminal();
                lhs = new Node(Tag::Dot,{lhs,rhs});
            }
            else return lhs;
        }
    }

    Node *Parser::terminal(){
        Node* tmp;
        string name;
        switch (lexer.getNextToken().token) {
            case Token::DIGIT:
                lexer.match(Token::DIGIT);
                return new Node(Tag::Digit,{{Attr::Lexeme,lexer.getToken().lexeme}});
            case Token::DECIMAL:
                lexer.match(Token::DECIMAL);
                return new Node(Tag::Decimal,{{Attr::Lexeme,lexer.getToken().lexeme}});
            case Token::STRING:
                lexer.match(Token::STRING);
                return new Node(Tag::String,{{Attr::Lexeme,lexer.getToken().lexeme}});
            case Token::CHAR:
                lexer.match(Token::CHAR);
                return new Node(Tag::Char,{{Attr::Lexeme,lexer.getToken().lexeme}});
            case Token::ID:
                lexer.match(Token::ID);
                name = lexer.getToken().lexeme;
                if(lexer.getNextToken().token==Token::LB){
                    tmp = argsBody();
                    return new Node(Tag::Callee,{{Attr::Lexeme,name}},{tmp});
                }
                else
                    return new Node(Tag::ID,{{Attr::Lexeme,name}});
            case Token::LB:
                lexer.match(Token::LB);
                tmp = expr();
                lexer.match(Token::RB);
                return tmp;
            default:
                throw "unimplement";//TODO report error
        }
    }

    Node *Parser::argsBody(){
        lexer.match(Token::LB);
        Node* ret = new Node(Tag::ParameterArg);
        if(lexer.getNextToken().token!=Token::RB){
            ret->child.push_back(expr());
            while(lexer.getNextToken().token==Token::COMMA){
                lexer.match(Token::COMMA);
                ret->child.push_back(expr());
            }
        }
        lexer.match(Token::RB);
        return ret;
    }

    Node *Parser::locating(){
        lexer.match(Token::ID);
        Node *rhs,*lhs = new Node(Tag::ID,{{Attr::Lexeme,lexer.getToken().lexeme}});
        while(lexer.getNextToken().token==Token::DOT){
            lexer.match(Token::DOT);
            rhs = new Node(Tag::ID,{{Attr::Lexeme,lexer.getToken().lexeme}});
            lhs = new Node(Tag::Dot,{lhs,rhs});
        }
        return lhs;
    }

    Node *Parser::ifStmt(){
        auto ifNode = new Node(Tag::If);
        lexer.match(Token::if_);
        auto cond = expr();
        lexer.match(Token::then_);
        auto stmts = stmtSet();
        ifNode->child.push_back(new Node(Tag::ElseIf,{cond,stmts}));
        while(lexer.getNextToken().token!=Token::end_){
            switch(lexer.getNextToken().token){
                case Token::elseif_:
                    lexer.match(Token::elseif_);
                    cond = expr();
                    lexer.match(Token::then_);
                    stmts = stmtSet();
                    ifNode->child.push_back(new Node(Tag::ElseIf,{cond,stmts}));
                    break;
                case Token::else_:
                    lexer.match(Token::else_);
                    stmts = stmtSet();
                    ifNode->child.push_back(new Node(Tag::Else,{stmts}));
                    break;
            }
        }
        lexer.match(Token::end_);
        lexer.match(Token::if_);
        return ifNode;
    }

    Node *Parser::loopStmt(){
        lexer.match(Token::while_);
        auto cond = expr();
        auto stmts = stmtSet();
        lexer.match(Token::wend_);
        return new Node(Tag::Loop,{cond,stmts});
    }

    Node *Parser::forStmt(){
        lexer.match(Token::for_);
        auto flag = locating();
        lexer.match(Token::ASSIGN);
        auto begin = expr();
        lexer.match(Token::to_);
        auto end = expr();
        Node* step = nullptr;
        if(lexer.getNextToken().token==Token::step_){
            lexer.match(Token::step_);
            step = expr();
        }
        auto stmts = stmtSet();
        lexer.match(Token::next_);
        return new Node(Tag::For,{flag,begin,end,step,stmts});
    }

    Node *Parser::controlStmt(){
        switch(lexer.getNextToken().token){
            case Token::return_:
                lexer.match(Token::return_);
                return new Node(Tag::Return,{expr()});
            case Token::continue_:
                lexer.match(Token::continue_);
                return new Node(Tag::Continue);
            case Token::exit_:
                lexer.match(Token::exit_);
                switch (lexer.getNextToken().token) {
                    case Token::for_:
                        lexer.match(Token::for_);
                        return new Node(Tag::ExitFor);
                    case Token::while_:
                        lexer.match(Token::while_);
                        return new Node(Tag::ExitLoop);
                    case Token::sub_:
                        lexer.match(Token::sub_);
                        return new Node(Tag::ExitSub);
                    default:
                        throw "unimplement";//TODO
                }
        }
    }

    Node *Parser::selectStmt(){
        lexer.match(Token::select_);
        lexer.match(Token::case_);
        auto cond = expr();
        auto ret = new Node(Tag::Select,{cond});
        while(lexer.getNextToken().token==Token::case_){
            lexer.match(Token::case_);
            if(lexer.getNextToken().token==Token::else_){
                lexer.match(Token::else_);
                auto stmts = stmtSet();
                ret->child.push_back(new Node(Tag::DefaultCase,{stmts}));
            }
            else{
                auto exp = expr();
                auto stmts = stmtSet();
                ret->child.push_back(new Node(Tag::Case,{exp,stmts}));
            }
        }
        lexer.match(Token::end_);
        lexer.match(Token::select_);
        return ret;
    }

    Node *Parser::stmtSet(){
        auto ret = new Node(Tag::Statements);
        auto t = lexer.getNextToken().token;
        set<Token::Enum> follow = {Token::if_,Token::while_,Token::select_,Token::let_,Token::ID,
                           Token::return_,Token::exit_,Token::continue_,Token::for_};
        while(follow.contains(lexer.getNextToken().token)){
            ret->child.push_back(stmt());
        }
        return ret;
    }

    Node *Parser::stmt(){
        switch (lexer.getNextToken().token) {
            case Token::if_: return ifStmt();
            case Token::while_: return loopStmt();
            case Token::let_: return variableDecl();
            case Token::select_: return selectStmt();
            case Token::for_: return forStmt();
            case Token::continue_:
            case Token::return_:
            case Token::exit_:
                    return controlStmt();
            default: return expr();
        }
    }

    Node *Parser::parse(){
        return moduleDecl();
    }

    Parser::Parser(Lexer &lexer, Logger &logger, Domain& domain)
        :lexer(lexer),logger(logger),global(domain){}
}