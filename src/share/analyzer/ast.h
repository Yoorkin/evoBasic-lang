//
// Created by yorkin on 12/18/21.
//

#ifndef EVOBASIC_AST_H
#define EVOBASIC_AST_H

#include <analyzer/type.h>
#include <parser/parseTree.h>


namespace evoBasic::ast{

    struct Node;
    struct Member;
    struct Global;
    struct Class;
    struct Module;
    struct Implement;
    struct Import;
    struct Dim;
    struct Variable;
    struct Function;
    struct Constructor;
    struct External;
    struct Interface;
    struct EnumMember;
    struct Enum;
    struct Type;
    struct Statement;
    struct Let;
    struct Select;
    struct Loop;
    struct If;
    struct For;
    struct Return;
    struct Exit;
    struct Continue;
    struct ExprStmt;
    struct Expression;
    struct Case;

    using AST = Global;

    std::string debugAST(ast::Node *ast);

    struct Node{
        virtual DebugInfo *debug()=0;
    };

    struct Member : Node{
        AccessFlag access = AccessFlag::Private;
        bool is_static = false;
        enum MemberKind{
            error,function_,class_,module_,type_,enum_,dim_,
            import_,external_,interface_,constructor_
        }member_kind = error;

        Member *next_sibling = nullptr,*prv_sibling = nullptr;
    };

    struct Global : Node{
        Member *member = nullptr;
        type::Domain *global_symbol = nullptr;
        DebugInfo *debug()override;
    };

    struct Class : Member{
        Class(){member_kind = MemberKind::class_;}
        Member *member = nullptr;
        type::Class *class_symbol = nullptr;
        DebugInfo *debug()override;
    };

    struct Module : Member{
        Module(){member_kind = MemberKind::module_;}
        Member *member = nullptr;
        type::Domain *module_symbol = nullptr;
        DebugInfo *debug()override;
    };

    struct Import : Member{
        Import(){member_kind = MemberKind::import_;}
        type::Module *target = nullptr;
        DebugInfo *debug()override;
    };

    struct Dim : Member{
        Dim(){member_kind = MemberKind::dim_;}
        bool is_const = false;
        Variable* variable = nullptr;
        DebugInfo *debug()override;
    };

    struct Variable : Node{
        Variable(type::Variable *variable,Expression *initial){
            this->variable_symbol = variable;
            this->initial = initial;
        }
        Variable(){}
        type::Variable *variable_symbol = nullptr;
        Expression *initial = nullptr;
        Variable *next_sibling = nullptr,*prv_sibling = nullptr;
        DebugInfo *debug()override;
    };


    struct Function : Member{
        Function(){member_kind = MemberKind::function_;}
        Statement* statement = nullptr;
        type::Function *function_symbol = nullptr;
        DebugInfo *debug()override;
    };

    struct Constructor : Member{
        Constructor(){member_kind = MemberKind::constructor_;}
        Statement* statement = nullptr;
        type::Constructor *constructor_symbol = nullptr;
        DebugInfo *debug()override;
    };

    struct External : Member{
        External(type::ExternalFunction *symbol){
            member_kind = MemberKind::external_;
            this->function_symbol = symbol;
        }
        type::ExternalFunction *function_symbol = nullptr;
        DebugInfo *debug()override;
    };

    struct Interface : Member {
        Interface(type::Interface *symbol){
            member_kind = interface_;
            this->interface_symbol = symbol;
        }
        type::Interface *interface_symbol = nullptr;
        DebugInfo *debug()override;
    };

    struct Enum : Member{
        Enum(type::Enumeration *symbol){
            member_kind = MemberKind::enum_;
            this->enum_symbol = symbol;
        }
        type::Enumeration *enum_symbol = nullptr;
        DebugInfo *debug()override;
    };

    struct Type : Member{
        Type(type::Record *symbol){
            member_kind = MemberKind::type_;
            this->type_symbol = symbol;
        }
        type::Record *type_symbol = nullptr;
        DebugInfo *debug()override;
    };

    struct Statement : Node{
        enum Enum{
            error,let_,loop_,if_,for_,select_,return_,continue_,exit_,expr_
        } stmt_flag=error;
        Statement *next_sibling = nullptr,*prv_sibling = nullptr;
    };

    struct Let : Statement{
        Let(){stmt_flag = let_;}
        Variable* variable = nullptr;
        DebugInfo *debug()override;
    };

    struct Select : Statement{
        Select(){stmt_flag = select_;}
        Expression *condition = nullptr;
        Case *case_ = nullptr;
        DebugInfo *debug()override;
    };

    struct Loop : Statement{
        Loop(){stmt_flag = loop_;}
        Expression *condition = nullptr;
        Statement* statement = nullptr;
        DebugInfo *debug()override;
    };

    struct If : Statement{
        If(){stmt_flag = if_;}
        Case *case_ = nullptr;
        DebugInfo *debug()override;
    };

    struct For : Statement{
        For(){stmt_flag = for_;}
        Expression *iterator = nullptr;
        bool iterator_has_let = false;
        Expression *begin = nullptr,*end = nullptr,*step = nullptr;
        type::Variable *iterator_variable = nullptr,
                       *begin_variable = nullptr,
                       *end_variable = nullptr,
                       *step_variable = nullptr;
        Statement* statement = nullptr;
        DebugInfo *debug()override;
    };

    struct Return : Statement{
        Return(){stmt_flag = return_;}
        Expression *expr = nullptr;
        DebugInfo *debug()override;
    };

    struct Exit : Statement{
        enum Kind{For,While,Sub}exit_flag;
        Exit(Kind kind){
            stmt_flag = exit_;
            exit_flag = kind;
        }
        DebugInfo *debug()override;
    };

    struct Continue : Statement{
        Continue(){stmt_flag = continue_;}
        DebugInfo *debug()override;
    };

    struct ExprStmt : Statement{
        ExprStmt(Expression *expr){
            stmt_flag = expr_;
            this->expr = expr;
        }
        Expression *expr;
        DebugInfo *debug()override;
    };

    struct Case : Node{
        Expression *condition = nullptr;
        Statement *statement = nullptr;
        Case *next_sibling = nullptr,*prv_sibling = nullptr;
        DebugInfo *debug()override;
    };

    struct Expression : Node{
        enum ExpressionKind{
            Element,Ftn,VFtn,SFtn,Local,ArgUse,Fld,Assign,
            SFld,Digit,Decimal,String,Boolean,Char,Unary,Binary,
            Cast,New,Parentheses,Empty,Argument,TmpPath,Ext,Delegate,EnumMember
        }expression_kind = Empty;
        ExpressionType *type = nullptr;
        static Expression *error;
        DebugInfo *debug()override;
    };

    struct EnumMember : Expression{
        EnumMember(){expression_kind = ExpressionKind::EnumMember;}
        type::EnumMember *member = nullptr;
        explicit EnumMember(type::EnumMember *member, ExpressionType *type):EnumMember(){
            this->member = member;
            this->type = type;
        }
        DebugInfo *debug()override;
    };

    struct TmpPath : Expression{
        TmpPath(ExpressionType *type){
            expression_kind = Expression::TmpPath;
            this->type = type;
        }
    };

    struct Unary : Expression{
        Unary(){expression_kind = ExpressionKind::Unary;}
        using Op = parseTree::Unary::Enum;
        Op op = Op::Empty;
        Expression *terminal = nullptr;
        Unary(parseTree::Unary::Enum op,Expression *terminal):Unary(){
            this->op = op;
            this->terminal = terminal;
        }
        DebugInfo *debug()override;
    };

    struct Binary : Expression{
        Binary(){expression_kind = ExpressionKind::Binary;}
        Binary(Expression *lhs,parseTree::Binary::Enum op,Expression *rhs):Binary(){
            NotNull(lhs);
            NotNull(rhs);
            this->lhs = lhs;
            this->rhs = rhs;
            this->op = op;
        }
        Expression *lhs = nullptr;
        using Op = parseTree::Binary::Enum;
        Op op = Op::Empty;
        Expression *rhs = nullptr;
        type::Variable *temp_address = nullptr;
        DebugInfo *debug()override;
    };

    struct Cast : Expression{
        Cast(Expression *expression,type::Prototype *target){
            expression_kind = Expression::Cast;
            this->expr = expression;
            this->target = target;
        }
        Expression *expr = nullptr;
        type::Prototype *target = nullptr;
        DebugInfo *debug()override;
    };

    struct Assign : Expression{
        Assign(Expression *lhs,Expression *rhs){
            expression_kind = Expression::Assign;
            this->lhs = lhs;
            this->rhs = rhs;
        }
        Expression *lhs = nullptr;
        Expression *rhs = nullptr;
        DebugInfo *debug()override;
    };

    struct Argument : Expression{
        Argument(){
            expression_kind = Expression::Argument;
        }
        bool byval = false;
        type::Parameter *parameter = nullptr;
        Expression *expr = nullptr;
        Argument *next_sibling = nullptr,*prv_sibling = nullptr;
        DebugInfo *debug()override;
    };

    struct Parentheses : Expression{
        Parentheses(){
            expression_kind = ExpressionKind::Parentheses;
        }
        Expression *expr = nullptr;
        DebugInfo *debug()override;
    };

    struct ArrayElement : Expression{
        ArrayElement(Expression *array,Expression *offset){
            expression_kind = Expression::Element;
            this->array = array;
            this->offset = offset;
        }
        Expression *array = nullptr;
        Expression *offset = nullptr;
        DebugInfo *debug()override;
    };

    struct Delegate : Expression{
        Delegate(type::Function *target,bool is_static,ExpressionType *type){
            expression_kind = Expression::Delegate;
            this->target = target;
            this->type = type;
            this->is_static = is_static;
        }
        bool is_static;
        Expression *ref = nullptr;
        type::Function *target = nullptr;
        DebugInfo *debug()override;
    };

    struct Call : Expression{
        type::Function *function = nullptr;
        struct Argument *argument = nullptr;
    };

    struct New : Call{
        New(type::Class *target,type::Function *ctor){
            expression_kind = Expression::New;
            this->target = target;
            this->function = ctor;
        }
        type::Class *target = nullptr;
        DebugInfo * debug() override;
    };


    struct FtnCall : Call{
        FtnCall(){
            expression_kind = Expression::Ftn;
        }
        Expression *ref = nullptr;
        DebugInfo *debug()override;
    };

    struct VFtnCall : Call{
        VFtnCall(){
            expression_kind = Expression::VFtn;
        }
        Expression *ref = nullptr;
        DebugInfo *debug()override;
    };

    struct SFtnCall : Call{
        SFtnCall(){
            expression_kind = Expression::SFtn;
        }
        DebugInfo *debug()override;
    };

    struct ExtCall : Call{
        ExtCall(){
            expression_kind = Expression::Ext;
        }
        DebugInfo *debug()override;
    };

    struct Local : Expression{
        Local(type::Variable *variable,ExpressionType *type){
            expression_kind = Expression::Local;
            this->variable = variable;
            this->type = type;
        }
        type::Variable *variable = nullptr;
        DebugInfo *debug()override;
    };

    struct Arg : Expression{
        Arg(type::Variable *variable,ExpressionType *type){
            expression_kind = Expression::ArgUse;
            this->variable = variable;
            this->is_ref = !variable->as<type::Parameter*>()->isByval();
            this->type = type;
        }
        bool is_ref = false;
        type::Variable *variable = nullptr;
        DebugInfo *debug()override;
    };

    struct Fld : Expression{
        Fld(type::Variable *variable,ExpressionType *type){
            expression_kind = Expression::Fld;
            this->variable = variable;
            this->type = type;
        }
        Expression *ref = nullptr;
        type::Variable *variable = nullptr;
        DebugInfo *debug()override;
    };

    struct SFld : Expression{
        SFld(type::Variable *variable,ExpressionType *type){
            expression_kind = Expression::SFld;
            this->variable = variable;
            this->type = type;
        }
        type::Variable *variable = nullptr;
        DebugInfo *debug()override;
    };

    struct Digit : Expression{
        Digit(){
            expression_kind = ExpressionKind::Digit;
        }
        data::i32 value = 0;
        DebugInfo *debug()override;
    };

    struct Decimal : Expression{
        Decimal(){
            expression_kind = ExpressionKind::Decimal;
        }
        data::f64 value = 0;
        DebugInfo *debug()override;
    };

    struct String : Expression{
        String(){
            expression_kind = ExpressionKind::String;
        }
        std::string value;
        DebugInfo *debug()override;
    };

    struct Char : Expression{
        Char(){
            expression_kind = ExpressionKind::String;
        }
        unicode::Utf8Char value;
        DebugInfo *debug()override;
    };

    struct Boolean : Expression{
        Boolean(){
            expression_kind = ExpressionKind::Boolean;
        }
        data::boolean value = false;
        DebugInfo *debug()override;
    };

}

#endif //EVOBASIC_AST_H
