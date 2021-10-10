//
// Created by yorkin on 7/13/21.
//

#include "Parser.h"
#include "Exception.h"
#define stmtFollows Token::if_,Token::while_,Token::select_,Token::let_,Token::ID,\
Token::return_,Token::exit_,Token::continue_,Token::for_,Token::not_,Token::MINUS,\
Token::ADD,Token::DIGIT,Token::DECIMAL,Token::STRING,Token::CHAR,Token::LB,Token::ID

namespace evoBasic{
    set<Token::Enum> combine(set<Token::Enum> a,set<Token::Enum> b){
        a.insert(b.begin(),b.end());
        return move(a);
    }


    shared_ptr<Node> Parser::globalDecl(){
        set<Token::Enum> follow = {Token::import,Token::public_,Token::private_,Token::friend_,Token::virtual_,Token::override_,
                                   Token::function_,Token::sub_,Token::type_,Token::enum_,Token::module_,Token::class_,
                                   Token::let_,Token::EOF_};
        shared_ptr<Node> ret(new Node(Tag::Global));
        while(lexer.getNextToken().kind != Token::EOF_){
            auto flag = accessFlag();
            auto member = moduleMember(follow);
            member->set(Attr::AccessFlag,flag);
            if(member->tag==Tag::Function)member->set(Attr::MethodFlag,MethodFlag::Normal);
            ret->child.push_back(member);
        }
        lexer.match(Token::EOF_);
        return ret;
    }

    shared_ptr<Node> Parser::moduleDecl(set<Token::Enum> follow){
        set<Token::Enum> memberFollow =  {Token::public_,Token::private_,Token::friend_,Token::virtual_,Token::override_,
                                          Token::function_,Token::sub_,Token::type_,Token::enum_,Token::module_,Token::class_,
                                          Token::let_};
        auto addition(follow);
        addition.insert(memberFollow.begin(),memberFollow.end());
        addition.insert(Token::end_module);

        lexer.match(Token::module_);
        shared_ptr<Node> ret(new Node(Tag::Module));
        ret->child.push_back(ID());
        ret->set(Attr::Position,ret->child[0]->pos());
        while(memberFollow.contains(lexer.getNextToken().kind)){
            auto flag = accessFlag();
            auto member = moduleMember(addition);
            member->set(Attr::AccessFlag,flag);
            if(member->tag==Tag::Function)member->set(Attr::MethodFlag,MethodFlag::Normal);
            ret->child.push_back(member);
        }
        lexer.match(Token::end_module,follow,"模块缺少结束符'End Module'");
        return ret;
    }

    shared_ptr<Node> Parser::classDecl(set<Token::Enum> follow){
        set<Token::Enum> memberFollow = {Token::public_,Token::private_,Token::friend_,Token::virtual_,Token::override_,
                                         Token::function_,Token::sub_,Token::type_,Token::enum_,
                                         Token::let_};
        auto addition(follow);
        addition.insert(memberFollow.begin(),memberFollow.end());
        addition.insert(Token::end_class);

        lexer.match(Token::class_);
        shared_ptr<Node> ret(new Node(Tag::Class));
        ret->child.push_back(ID());
        ret->set(Attr::Position,ret->child[0]->pos());
        while(memberFollow.contains(lexer.getNextToken().kind)){
            auto access = accessFlag();
            auto method = methodFlag();
            auto member = classMember(addition);
            member->set(Attr::AccessFlag,access);
            if(member->tag==Tag::Function)member->set(Attr::MethodFlag,method);
            ret->child.push_back(member);
        }
        lexer.match(Token::end_class,follow,"类缺少结束符'End Class'");
        return ret;
    }

    shared_ptr<Node> Parser::classMember(const set<Token::Enum>& follow){
        switch (lexer.getNextToken().kind) {
            case Token::function_: return functionDecl(follow);
            case Token::sub_:      return subDecl(follow);
            case Token::type_:     return typeDecl(follow);
            case Token::enum_:     return enumDecl(follow);
            case Token::let_:      return variableDecl(follow);
            default:
                logger->error(lexer.getNextToken().pos,"无法识别的声明");
                lexer.skipUntilFollow(follow);
                return Node::Error;
        }
    }

    shared_ptr<Node> Parser::moduleMember(const set<Token::Enum>& follow){
        switch (lexer.getNextToken().kind) {
            case Token::function_: return functionDecl(follow);
            case Token::sub_:      return subDecl(follow);
            case Token::type_:     return typeDecl(follow);
            case Token::enum_:     return enumDecl(follow);
            case Token::let_:      return variableDecl(follow);
            case Token::class_:    return classDecl(follow);
            case Token::module_:   return moduleDecl(follow);
            case Token::import:    return importDecl(follow);
            case Token::declare:   return declareDecl(follow);
            default:
                logger->error(lexer.getNextToken().pos,"无法识别的声明");
                lexer.skipUntilFollow(follow);
                return Node::Error;
        }
    }

    AccessFlag Parser::accessFlag(){
        auto token = lexer.getNextToken();
        switch (token.kind) {
            case Token::public_:
                lexer.match(Token::public_);
                return AccessFlag::Public;
            case Token::private_:
                lexer.match(Token::private_);
                return AccessFlag::Private;
            case Token::friend_:
                lexer.match(Token::friend_);
                return AccessFlag::Friend;
            default:
                return AccessFlag::Private;
        }
    }

    MethodFlag Parser::methodFlag(){
        auto token = lexer.getNextToken();
        switch(token.kind){
            case Token::static_:
                lexer.match(Token::static_);
                return MethodFlag::Static;
            case Token::virtual_:
                lexer.match(Token::virtual_);
                return MethodFlag::Virtual;
            case Token::override_:
                lexer.match(Token::override_);
                return MethodFlag::Override;
            default:
                return MethodFlag::Normal;
        }
    }

    shared_ptr<Node> Parser::typeDecl(set<Token::Enum> follow){
        auto addition(follow);
        addition.insert({Token::end_type,Token::ID});

        lexer.match(Token::type_);
        shared_ptr<Node> ret(new Node(Tag::Type));
        ret->child.push_back(ID());
        ret->set(Attr::Position, ret->child[0]->pos());
        while(lexer.getNextToken().kind==Token::ID){
            shared_ptr<Node> field(new Node(Tag::TypeField));
            field->child.push_back(ID(addition));
            field->set(Attr::Position, field->child[0]->pos());
            lexer.match(Token::as_,addition,"缺少关键字'As'");
            auto loca = locating(addition);
            field->child.push_back(make_node(Tag::Annotation,{loca}));
            ret->child.push_back(field);
        }
        lexer.match(Token::end_type,follow,"缺少'end type'结束符");
        return ret;
    }

    shared_ptr<Node> Parser::enumDecl(set<Token::Enum> follows){
        auto addition(follows);
        addition.insert({Token::ID,Token::end_enum});
        lexer.match(Token::enum_);
        shared_ptr<Node> ret(new Node(Tag::Enum));
        ret->child.push_back(ID(addition));
        ret->set(Attr::Position,ret->child[0]->get<Position>(Attr::Position));

        while(lexer.getNextToken().kind == Token::ID){
            shared_ptr<Node> member(new Node(Tag::EnumMember));
            member->child.push_back(ID(addition));
            member->set(Attr::Position, member->child[0]->pos());
            if(lexer.getNextToken().kind == Token::ASSIGN){
                lexer.match(Token::ASSIGN);
                member->child.push_back(DIGIT(addition));
            }
            ret->child.push_back(member);
        }
        lexer.match(Token::end_enum,follows,"缺少结束符号'end enum'");
        return ret;
    }

    shared_ptr<Node> Parser::functionDecl(set<Token::Enum> follows){
        lexer.match(Token::function_);
        auto id = ID(combine(follows,{Token::LB,Token::end_function}));
        auto params = parameterList(combine(follows,{Token::as_,Token::end_function}));
        lexer.match(Token::as_,combine(follows,{stmtFollows}),"缺少返回类型标记");
        auto retAnno = make_node(Tag::Annotation,{locating(combine(follows,{stmtFollows}))});
        auto stmts = stmtSet(combine(follows,{Token::end_function}));
        lexer.match(Token::end_function,follows,"函数缺少'End Function'标记");
        return make_node(Tag::Function, {{Attr::Position, id->pos()}}, {id, params, retAnno, stmts});
    }

    shared_ptr<Node> Parser::subDecl(set<Token::Enum> follows){
        auto addition(follows);
        addition.insert(Token::end_sub);

        lexer.match(Token::sub_);
        auto id = ID();
        auto params = parameterList(combine(follows,{stmtFollows}));
        auto stmts = stmtSet(addition);
        lexer.match(Token::end_sub,follows,"您完全不写'end sub'结束符号是吗？");
        return make_node(Tag::Function, {{Attr::Position, id->pos()}}, {id, params,Node::Empty, stmts});
    }

    shared_ptr<Node> Parser::parameterList(set<Token::Enum> follows){
        auto addition = combine(follows,{Token::COMMA,Token::RB});
        set<Token::Enum> memberFollow = {Token::byval_,Token::byref_,Token::ID,Token::COMMA,Token::as_};

        shared_ptr<Node> ret(new Node(Tag::ParameterList));
        lexer.match(Token::LB);
        while(memberFollow.contains(lexer.getNextToken().kind)){
            shared_ptr<Node> param(new Node(Tag::Parameter));
            if(lexer.getNextToken().kind == Token::optional_){
                lexer.match(Token::optional_);
                param->set(Attr::IsOptional,true);
            } else param->set(Attr::IsOptional,false);

            if(lexer.getNextToken().kind == Token::byref_){
                lexer.match(Token::byref_);
                param->set(Attr::IsByval,false);
            }
            else if(lexer.getNextToken().kind == Token::byval_){
                lexer.match(Token::byval_);
                param->set(Attr::IsByval,true);
            }
            else param->set(Attr::IsByval,false);

            param->child.push_back(ID(addition));

            if(lexer.match(Token::as_,addition,"缺少类型标记")){
                param->child.push_back(make_node(Tag::Annotation,{locating(addition)}));
            }
            else{
                param->child.push_back(Node::Error);
            }

            ret->child.push_back(param);

            if(lexer.getNextToken().kind!=Token::RB) {
                try{
                    lexer.match(Token::COMMA);
                }
                catch(SyntaxException& e){
                    logger->error(e.token.pos,"此处应有逗号，但却遇到了多余的符号");
                    lexer.skipUntilFollow({Token::RB, Token::COMMA});
                    if(lexer.getNextToken().kind==Token::COMMA)lexer.forward();
                }
            }
        }
        try{
            lexer.match(Token::RB);
        }
        catch(SyntaxException& e){
            logger->error(e.token.pos,Format()<<"需要右括号，但是遇到了'"<<e.token.lexeme<<"'");
        }

        return ret;
    }

    shared_ptr<Node> Parser::ID(set<Token::Enum> follows){
        try{
            lexer.match(Token::ID);
            return make_node(Tag::ID,{{Attr::Lexeme,lexer.getToken().lexeme},{Attr::Position,lexer.getToken().pos}});
        }
        catch (SyntaxException& e) {
            logger->error(e.token.pos,Format()<<"需要一个ID,但是遇到了"<<e.token.lexeme);
            lexer.skipUntilFollow(follows);
            return Node::Error;
        }
    }

    shared_ptr<Node> Parser::DIGIT(set<Token::Enum> follows){
      try{
          lexer.match(Token::DIGIT);
          int value = stoi(lexer.getToken().lexeme);
          return make_node(Tag::Digit,{{Attr::Value,value},{Attr::Position,lexer.getToken().pos}});
      }
      catch (SyntaxException& e) {
          logger->error(e.token.pos,Format()<<"需要一个整数,但是遇到了"<<e.token.lexeme);
          lexer.skipUntilFollow(follows);
          return Node::Error;
      }
    }

    shared_ptr<Node> Parser::DEMICAL(){
        lexer.match(Token::DECIMAL);
        double value = stod(lexer.getToken().lexeme);
        return make_node(Tag::Decimal,{{Attr::Value,value},{Attr::Position,lexer.getToken().pos}});
    }

    shared_ptr<Node> Parser::STRING(){
        lexer.match(Token::STRING);
        auto& lexeme = lexer.getToken().lexeme;
        auto& pos = lexer.getToken().pos;
        string str = lexeme.substr(1,lexeme.size()-2);
        return make_node(Tag::String,{{Attr::Value,str},{Attr::Position,pos}});
    }

    shared_ptr<Node> Parser::CHAR(){
        lexer.match(Token::CHAR);
        auto& lexeme = lexer.getToken().lexeme;
        auto& pos = lexer.getToken().pos;
        string str = lexeme.substr(1,lexeme.size()-2);
        return make_node(Tag::Char,{{Attr::Value,str},{Attr::Position,pos}});
    }

    shared_ptr<Node> Parser::variableDecl(set<Token::Enum> follow){
        follow.insert(Token::COMMA);
        lexer.match(Token::let_);
        shared_ptr<Node> ret(new Node(Tag::Let));
        while(true){
            shared_ptr<Node> var(new Node(Tag::Variable));
            var->child.push_back(ID());
            if(lexer.getNextToken().kind == Token::as_){
                lexer.match(Token::as_);
                var->child.push_back(make_node(Tag::Annotation,{locating()}));
            } else var->child.push_back(Node::Empty);

            if(lexer.getNextToken().kind == Token::ASSIGN){
                lexer.match(Token::ASSIGN);
                var->child.push_back(expr(follow));
            } else var->child.push_back(Node::Empty);

            ret->child.push_back(var);
            if(lexer.getNextToken().kind != Token::COMMA)break;
            lexer.match(Token::COMMA);
        }
        return ret;
    }

    shared_ptr<Node> Parser::expr(set<Token::Enum> follows){
        auto ret = make_node(Tag::Exprssion,{logic(follows)});
        if(follows.size()>0 && !follows.contains(lexer.getNextToken().kind)){
            for(auto a:follows)cout<<Token::reserved[(int)a]<<' ';cout<<endl;
            logger->error(lexer.getNextToken().pos,"缺少运算符或左括号");
            lexer.skipUntilFollow(follows);
        }
        return ret;
    }

    shared_ptr<Node> Parser::logic(set<Token::Enum> follows){
        auto addition = {Token::and_,Token::or_,Token::xor_};
        follows.insert(addition.begin(),addition.end());
        auto lhs = cmp(follows);
        shared_ptr<Node> rhs;
        while(true){
            auto op = lexer.getNextToken();
            switch(op.kind){
                case Token::and_:
                    lexer.match(Token::and_);
                    rhs = cmp(follows);
                    lhs = make_node(Tag::And,{{Attr::Position,op.pos}},{lhs,rhs});
                    break;
                case Token::or_:
                    lexer.match(Token::or_);
                    rhs = cmp(follows);
                    lhs = make_node(Tag::Or,{{Attr::Position,op.pos}},{lhs,rhs});
                    break;
                case Token::xor_:
                    lexer.match(Token::xor_);
                    rhs = cmp(follows);
                    lhs = make_node(Tag::Xor,{{Attr::Position,op.pos}},{lhs,rhs});
                    break;
                default:
                    return lhs;
            }
        }
    }


    shared_ptr<Node> Parser::cmp(set<Token::Enum> follows){
        auto addition = {Token::EQ,Token::NE,Token::LT,Token::GT,Token::LE,Token::GE};
        follows.insert(addition.begin(),addition.end());
        shared_ptr<Node> rhs,lhs = add(follows);
        while(true){
            auto op = lexer.getNextToken();
            switch(op.kind){
                case Token::EQ:
                    lexer.match(Token::EQ);
                    rhs = add(follows);
                    lhs = make_node(Tag::EQ,{{Attr::Position,op.pos}},{lhs,rhs});
                    break;
                case Token::NE:
                    lexer.match(Token::NE);
                    rhs = add(follows);
                    lhs = make_node(Tag::NE,{{Attr::Position,op.pos}},{lhs,rhs});
                    break;
                case Token::LT:
                    lexer.match(Token::LT);
                    rhs = add(follows);
                    lhs = make_node(Tag::LT,{{Attr::Position,op.pos}},{lhs,rhs});
                    break;
                case Token::GT:
                    lexer.match(Token::GT);
                    rhs = add(follows);
                    lhs = make_node(Tag::GT,{{Attr::Position,op.pos}},{lhs,rhs});
                    break;
                case Token::LE:
                    lexer.match(Token::LE);
                    rhs = add(follows);
                    lhs = make_node(Tag::LE,{{Attr::Position,op.pos}},{lhs,rhs});
                    break;
                case Token::GE:
                    lexer.match(Token::GE);
                    rhs = add(follows);
                    lhs = make_node(Tag::GE,{{Attr::Position,op.pos}},{lhs,rhs});
                    break;
                default:return lhs;
            }
        }
    }

    shared_ptr<Node> Parser::add(set<Token::Enum> follows){
        auto addition = {Token::ADD,Token::MINUS};
        follows.insert(addition.begin(),addition.end());
        shared_ptr<Node> rhs,lhs = term(follows);
        while(true){
            auto op = lexer.getNextToken();
            switch(op.kind){
                case Token::ADD:
                    lexer.match(Token::ADD);
                    rhs = term(follows);
                    lhs = make_node(Tag::Add,{{Attr::Position,op.pos}},{lhs,rhs});
                    break;
                case Token::MINUS:
                    lexer.match(Token::MINUS);
                    rhs = term(follows);
                    lhs = make_node(Tag::Minus,{{Attr::Position,op.pos}},{lhs,rhs});
                    break;
                default:return lhs;
            }
        }

    }

    shared_ptr<Node> Parser::term(set<Token::Enum> follows){
        auto addition = {Token::MUL,Token::FDIV,Token::DIV};
        follows.insert(addition.begin(),addition.end());
        shared_ptr<Node> rhs,lhs = factor(follows);
        while(true){
            auto op = lexer.getNextToken();
            switch (op.kind) {
                case Token::MUL:
                    lexer.match(Token::MUL);
                    rhs = factor(follows);
                    lhs = make_node(Tag::Mul,{{Attr::Position,op.pos}},{lhs,rhs});
                    break;
                case Token::FDIV:
                    lexer.match(Token::FDIV);
                    rhs = factor(follows);
                    lhs = make_node(Tag::FDiv,{{Attr::Position,op.pos}},{lhs,rhs});
                    break;
                case Token::DIV:
                    lexer.match(Token::DIV);
                    rhs = factor(follows);
                    lhs = make_node(Tag::Div,{{Attr::Position,op.pos}},{lhs,rhs});
                    break;
                default: return lhs;
            }
        }

    }

    shared_ptr<Node> Parser::factor(set<Token::Enum> follows){
        auto addition = {Token::MUL,Token::FDIV,Token::DIV};
        follows.insert(addition.begin(),addition.end());
        shared_ptr<Node> rhs,lhs = unit(follows);
        while(true){
            auto op = lexer.getNextToken();
            if(op.kind == Token::ASSIGN){
                lexer.match(Token::ASSIGN);
                rhs = unit(follows);
                lhs = make_node(Tag::Assign,{{Attr::Position,op.pos}},{lhs,rhs});
            }
            else if(op.kind == Token::as_){
                lexer.match(Token::as_);
                rhs = make_node(Tag::Annotation,{locating()});
                lhs = make_node(Tag::Cast,{{Attr::Position,op.pos}},{lhs,rhs});
            }
            else return lhs;
        }
    }

    shared_ptr<Node> Parser::unit(set<Token::Enum> follows){
        follows.insert(Token::DOT);
        shared_ptr<Node> rhs,lhs;

        int minusCount=0,addCount=0;
        Token *alertMinus=nullptr,*alertAdd=nullptr;
        while(lexer.getNextToken().kind == Token::MINUS || lexer.getNextToken().kind == Token::ADD){
            if(lexer.getNextToken().kind == Token::MINUS){
                lexer.match(Token::MINUS);
                minusCount++;
                if(minusCount>1)alertMinus= const_cast<Token *>(&lexer.getToken());
            }
            else{
                lexer.match(Token::ADD);
                addCount++;
                alertAdd= const_cast<Token *>(&lexer.getToken());
            }
        } //TODO 去掉这些消除“多余”正负号的代码

        if(alertMinus!=nullptr)logger->warning(alertMinus->pos,"傻逼吗 写了那么多负号干什么");
        if(alertAdd!=nullptr)logger->warning(alertAdd->pos,"傻逼吗 多写正号干什么");

        if(minusCount%2==1)lhs = make_node(Tag::SelfNeg,{{Attr::Position,lexer.getToken().pos}},{terminal(follows)});
        else lhs = terminal(follows);

        while(true){
            auto op = lexer.getNextToken();
            if(op.kind == Token::DOT){
                lexer.match(Token::DOT);
                rhs = terminal(follows);
                lhs = make_node(Tag::Dot,{{Attr::Position,op.pos}},{lhs,rhs});
            }
            else return lhs;
        }
    }

    shared_ptr<Node> Parser::terminal(set<Token::Enum> follows){
        shared_ptr<Node> tmp;
        string name;
        switch (lexer.getNextToken().kind) {
            case Token::DIGIT:
                return DIGIT();
            case Token::DECIMAL:
                return DEMICAL();
            case Token::STRING:
                return STRING();
            case Token::CHAR:
                return CHAR();
            case Token::ID:
                lexer.match(Token::ID);
                name = lexer.getToken().lexeme;
                if(lexer.getNextToken().kind == Token::LB){
                    follows.insert(Token::RB);
                    tmp = argsBody(follows);
                    return make_node(Tag::Callee,{{Attr::Lexeme,name},{Attr::Position,lexer.getToken().pos}},{tmp});
                }
                else
                    return make_node(Tag::ID,{{Attr::Lexeme,name},{Attr::Position,lexer.getToken().pos}});
            case Token::LB:
                lexer.match(Token::LB);
                follows.insert(Token::RB);
                tmp = logic(follows);
                lexer.match(Token::RB,follows,"缺少右括号");
                return tmp;
            default:
                logger->error(lexer.getToken().pos,"缺少表达式");
                printFollows(follows,"234");
                lexer.skipUntilFollow(follows);
                return Node::Error;
                //throw SyntaxException(&lexer.getNextToken());
        }
    }

    shared_ptr<Node> Parser::argsBody(set<Token::Enum> follows){
        auto nextFollow(follows);
        nextFollow.insert(Token::COMMA);
        lexer.match(Token::LB,nextFollow,"缺少左括号");
        auto ret = make_node(Tag::ParameterArg,{{Attr::Position,lexer.getToken().pos}});
        if(lexer.getNextToken().kind != Token::RB){
            ret->child.push_back(expr(nextFollow));
            while(lexer.getNextToken().kind == Token::COMMA){
                lexer.match(Token::COMMA);
                ret->child.push_back(expr(nextFollow));
            }
        }
        lexer.match(Token::RB,follows,"缺少右括号");
        return ret;
    }

    shared_ptr<Node> Parser::importDecl(set<Token::Enum> follow) {
        lexer.match(Token::import);
        auto pos = lexer.getToken().pos;
        auto loca = locating(follow);
        return make_node(Tag::Import,{{Attr::Position,pos}},{loca});
    }

    shared_ptr<Node> Parser::declareDecl(set<Token::Enum> follow){
        lexer.match(Token::declare);
        bool isfunc;
        try{
            if(lexer.getNextToken().kind==Token::function_){
                isfunc = true;
                lexer.match(Token::function_);
            }
            else if(lexer.getNextToken().kind==Token::sub_){
                isfunc = false;
                lexer.match(Token::sub_);
            }
        }
        catch(SyntaxException& e){
            logger->error(e.token.pos,"'Declare'符号后只能为'Function'、'Sub'关键字");
        }

        auto name = ID(combine(follow,{Token::lib,Token::alias,Token::LB}));
        auto pos = name->pos();
        auto ret = make_node(Tag::ExternalFunction,{{Attr::Position,pos}},{name});
        if(lexer.getNextToken().kind==Token::lib){
            lexer.match(Token::lib);
            ret->child.push_back(STRING());
        }
        else ret->child.push_back(Node::Empty);

        if(lexer.getNextToken().kind==Token::alias){
            lexer.match(Token::alias);
            ret->child.push_back(STRING());
        }
        else ret->child.push_back(Node::Empty);

        auto params = parameterList(follow);
        ret->child.push_back(params);

        if(isfunc){
            lexer.match(Token::as_,follow,"缺少返回类型标记");
            auto loca = locating(follow);
            ret->child.push_back(loca);
        }
        else ret->child.push_back(Node::Empty);

        return ret;
    }

    shared_ptr<Node> Parser::locating(set<Token::Enum> follow){
        auto addition(follow);
        addition.insert(Token::DOT);
        shared_ptr<Node> rhs,lhs = ID(addition);
        while(lexer.getNextToken().kind == Token::DOT){
            lexer.match(Token::DOT);
            rhs = ID(addition);
            lhs = make_node(Tag::Dot,{lhs,rhs});
        }
        return make_node(Tag::Path,{lhs});
    }

    shared_ptr<Node> Parser::ifStmt(set<Token::Enum> follow){
        set<Token::Enum> addition = {Token::end_if,Token::elseif_,Token::else_,Token::then_};
        addition.insert(follow.begin(),follow.end());
        auto ifNode = make_node(Tag::If);
        auto headPos = lexer.getNextToken().pos;
        shared_ptr<Node> cond,stmts;
        lexer.match(Token::if_);
        cond = expr(addition);
        try{
            lexer.match(Token::then_);
            stmts = stmtSet(addition);
            ifNode->child.push_back(make_node(Tag::ElseIf,{{Attr::Position,headPos}},{cond,stmts}));
        }
        catch(SyntaxException& e){
            ifNode->tag=Tag::Error;
            logger->error(lexer.getNextToken().pos,Format()<<"期望关键字'then'但是遇到了"<<Token::reserved[(int)e.token.kind]);
            lexer.skipUntilFollow(addition);
        }

        bool loop = true;
        while(loop){
            switch(lexer.getNextToken().kind){
                case Token::elseif_:
                    lexer.match(Token::elseif_);
                    headPos = lexer.getToken().pos;
                    try{
                        cond = expr(addition);
                        lexer.match(Token::then_);
                        stmts = stmtSet(addition);
                        ifNode->child.push_back(make_node(Tag::ElseIf,{{Attr::Position,headPos}},{cond,stmts}));
                    }
                    catch(SyntaxException& e){
                        logger->error(lexer.getNextToken().pos,Format()<<"期望关键字'then'但是遇到了'"<<Token::reserved[(int)e.token.kind]<<"'");
                        lexer.skipUntilFollow(addition);
                    }
                    break;
                case Token::else_:
                    lexer.match(Token::else_);
                    headPos = lexer.getToken().pos;
                    stmts = stmtSet(addition);
                    ifNode->child.push_back(make_node(Tag::Else,{{Attr::Position,headPos}},{stmts}));
                    break;
                default: loop = false;
            }
        }

        try{
            lexer.match(Token::end_if);
        }
        catch(SyntaxException& e){
            logger->error(e.token.pos,Format()<<"期望关键字'"<<Token::reserved[(int)e.expected]
                                                                        <<"'但是遇到了'"<<Token::reserved[(int)e.token.kind]<<"'");
            lexer.skipUntilFollow(follow);
        }
        return ifNode;
    }

    shared_ptr<Node> Parser::loopStmt(set<Token::Enum> follow){
        follow.insert(Token::wend_);
        lexer.match(Token::while_);
        auto headPos = lexer.getToken().pos;
        auto cond = expr(follow);
        auto stmts = stmtSet(follow);
        lexer.match(Token::wend_,follow,"循环体缺少结束关键字'Wend'");
        return make_node(Tag::Loop,{{Attr::Position,headPos}},{cond,stmts});
    }

    shared_ptr<Node> Parser::forStmt(set<Token::Enum> follow){
        set<Token::Enum> addition = {Token::to_,Token::step_,Token::next_};
        addition.insert(follow.begin(),follow.end());

        lexer.match(Token::for_);
        auto headPos = lexer.getToken().pos;
        auto flag = locating({Token::ASSIGN,Token::to_});

        lexer.match(Token::ASSIGN,addition,"缺少符号'='");
        auto begin = expr(addition);
        addition.erase(Token::to_);

        lexer.match(Token::to_,addition,"缺少符号'To'");
        auto end = expr(addition);

        shared_ptr<Node> step;
        if(lexer.getNextToken().kind == Token::step_){
            lexer.match(Token::step_);
            step = expr(addition);
        }

        auto stmts = stmtSet(addition);
        lexer.match(Token::next_,follow,"For循环缺少'Next'结束符号");
        return make_node(Tag::For,{{Attr::Position,headPos}},{flag,begin,end,step,stmts});
    }

    shared_ptr<Node> Parser::controlStmt(set<Token::Enum> follow){
        shared_ptr<Node> tmp;
        switch(lexer.getNextToken().kind){
            case Token::return_:
                lexer.match(Token::return_);
                tmp = expr(follow);
                return make_node(Tag::Return,{{Attr::Position,lexer.getToken().pos}},{tmp});
            case Token::continue_:
                lexer.match(Token::continue_);
                return make_node(Tag::Continue,{{Attr::Position,lexer.getToken().pos}});
            case Token::exit_:
                lexer.match(Token::exit_);
                switch (lexer.getNextToken().kind) {
                case Token::for_:
                    lexer.match(Token::for_);
                    return make_node(Tag::ExitFor,{{Attr::Position,lexer.getToken().pos}});
                case Token::while_:
                    lexer.match(Token::while_);
                    return make_node(Tag::ExitLoop,{{Attr::Position,lexer.getToken().pos}});
                case Token::sub_:
                    lexer.match(Token::sub_);
                    return make_node(Tag::ExitSub,{{Attr::Position,lexer.getToken().pos}});
                default:
                    logger->error(lexer.getNextToken().pos,"'Exit'后的关键字只能是'For'、'While'、'Sub'");
                    lexer.skipUntilFollow(follow);
                    return Node::Error;
                }
        }
    }

    shared_ptr<Node> Parser::selectStmt(set<Token::Enum> follow){
        set<Token::Enum> addition = {Token::case_,Token::end_select};
        addition.insert(follow.begin(),follow.end());

        lexer.match(Token::select_);
        auto headPos = lexer.getToken().pos;
        lexer.match(Token::case_);
        auto cond = expr(addition);
        auto ret = make_node(Tag::Select,{{Attr::Position,headPos}},{cond});

        while(!follow.contains(lexer.getNextToken().kind)){
            lexer.match(Token::case_,addition,"缺少case关键字");
            if(lexer.getNextToken().kind == Token::else_){
                lexer.match(Token::else_,addition,"缺少else关键字");
                auto stmts = stmtSet(addition);
                ret->child.push_back(make_node(Tag::DefaultCase,{stmts}));
            }
            else{
                auto exp = expr(addition);
                auto stmts = stmtSet(addition);
                ret->child.push_back(make_node(Tag::Case,{exp,stmts}));
            }
        }

        lexer.match(Token::end_select,follow,"Select语句缺少'end select'结束符");
        return ret;
    }

    shared_ptr<Node> Parser::stmtSet(set<Token::Enum> follow){
        auto ret = make_node(Tag::Statements);
        set<Token::Enum> stmtfollow = {stmtFollows};
        auto stmtfollowAndsetfollow(follow);
        stmtfollowAndsetfollow.insert(stmtfollow.begin(),stmtfollow.end());
        while(true){
            auto next = lexer.getNextToken().kind;
            if(stmtfollow.contains(next)){
                ret->child.push_back(stmt(stmtfollowAndsetfollow));
            }
            else if(!follow.empty() && !follow.contains(next)){
                logger->error(lexer.getNextToken().pos,"无法识别的语句");
                lexer.skipUntilFollow(stmtfollowAndsetfollow);
            }
            else break;
        }
        return ret;
    }

    shared_ptr<Node> Parser::stmt(set<Token::Enum> follow){
         switch(lexer.getNextToken().kind) {
            case Token::if_: return ifStmt(follow);
            case Token::while_: return loopStmt(follow);
            case Token::let_: return variableDecl(follow);
            case Token::select_: return selectStmt(follow);
            case Token::for_: return forStmt(follow);
            case Token::continue_:
            case Token::return_:
            case Token::exit_:
                    return controlStmt(follow);
            case Token::not_:
            case Token::MINUS:
            case Token::ADD:
            case Token::DIGIT:
            case Token::DECIMAL:
            case Token::STRING:
            case Token::CHAR:
            case Token::LB:
            case Token::ID:
                    return expr(follow);
            default:
              throw "unimplement";
//                if(!follow.empty() && !follow.contains(lexer.getNextToken().kind)){
//                    logger.error(lexer.getNextToken().pos,"无法识别的语句");
//                    lexer.skipUntilFollow(follow);
//                }
        }
    }

    AST Parser::parse(){
        return AST(logger,globalDecl());
    }

    Parser::Parser(Lexer &lexer,shared_ptr<Logger> logger)
        :lexer(lexer),logger(logger){}

}