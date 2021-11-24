#ifndef EB_AST
#define EB_AST
#include "token.h"
#include "data.h"
#include "nullSafe.h"
#include <list>
#include <memory>

namespace evoBasic{
    enum class AccessFlag {Public,Private,Friend,Protected};
    enum class MethodFlag {Static,Virtual,Override,None};
    struct ExpressionType;
    namespace type{
        class Variable;
    }
}

namespace evoBasic::ast{

    struct Node;
    struct Global;
    struct Class;
    struct Module;
    struct Member;
    struct Import;
    struct Dim;
    struct Variable;
    struct Function;
    struct External;
    struct Init;
    struct Operator;
    struct Enum;
    struct Type;
    struct Parameter;
    struct Annotation;
    struct Generic;

    namespace stmt{
        struct Statement;
        struct Let;
        struct Select;
        struct Loop;
        struct If;
        struct For;
        struct Control;
        struct Return;
        struct Exit;
    }

    struct Case;

    namespace expr{
        struct Expression;
        struct Link;
        struct Binary;
        struct Unary;
        struct Terminal;
        struct Callee;
        struct ID;
        struct Digit;
        struct Decimal;
        struct String;
        struct Char;
    }


    std::string debugAST(Node *ast);

    using Expr = expr::Expression;

    struct DebugInfo{
        std::string text;
        std::list<DebugInfo*> childs;
        ~DebugInfo(){
            for(auto child:childs)delete child;
        }
    };

    struct Node{
        Location *location = nullptr;
        virtual DebugInfo *debug()=0;
    };

    struct Global : Node{
        std::list<Member*> member_list{};
        DebugInfo *debug()override;
    };


    struct Member : Node{
        AccessFlag access;
        enum MemberKind{
            error,function_,class_,module_,type_,enum_,dim_,operator_,init_,
            import_,external_
        }member_kind = error;
    };

    struct Class : Member{
        Class(){member_kind = MemberKind::class_;}
        expr::ID *name = nullptr;
        Annotation *extend = nullptr;
        std::list<Annotation*> impl_list{};
        std::list<Member*> member_list{};
        DebugInfo *debug()override;
    };

    struct Module : Member{
        Module(){member_kind = MemberKind::module_;}
        expr::ID *name = nullptr;
        std::list<Member*> member_list;
        DebugInfo *debug()override;
    };


    struct Import : Member{
        Import(){member_kind = MemberKind::import_;}
        Annotation *annotation = nullptr;
        DebugInfo *debug()override;
    };

    struct Dim : Member{
        Dim(){member_kind = MemberKind::dim_;}
        bool is_const = false;
        std::list<Variable*> variable_list{};
        DebugInfo *debug()override;
    };

    struct Variable : Node{
        expr::ID *name = nullptr;
        Annotation *annotation = nullptr;
        expr::Expression *initial = nullptr;
        DebugInfo *debug()override;
    };

    struct Function : Member{
        Function(){member_kind = MemberKind::function_;}
        MethodFlag method_flag;
        expr::ID *name = nullptr;
        std::list<Parameter*> parameter_list{};
        Annotation *return_type = nullptr;
        std::list<stmt::Statement*> statement_list{};
        DebugInfo *debug()override;
    };

    struct External : Member{
        External(){member_kind = MemberKind::external_;}
        expr::ID *name = nullptr;
        expr::String *lib = nullptr,*alias = nullptr;
        std::list<Parameter*> parameter_list;
        Annotation *return_annotation = nullptr;
        DebugInfo *debug()override;
    };

    struct Init : Member{
        Init(){member_kind = MemberKind::init_;}
        std::list<Parameter*> parameter_list;
        std::list<stmt::Statement*> statement_list{};
        DebugInfo *debug()override;
    };

    struct Operator : Member{
        Operator(){member_kind = MemberKind::operator_;}
        expr::ID *name = nullptr;
        std::list<Parameter*> parameter_list{};
        Annotation *return_annotation;
        std::list<stmt::Statement*> statement_list{};
        DebugInfo *debug()override;
    };

    using EnumMember = std::pair<expr::ID*,expr::Digit*>;
    struct Enum : Member{
        Enum(){member_kind = MemberKind::enum_;}
        expr::ID *name;
        std::list<EnumMember> member_list{};
        DebugInfo *debug()override;
    };

    struct Type : Member{
        Type(){member_kind = MemberKind::type_;}
        expr::ID *name;
        std::list<Variable*> member_list;
        DebugInfo *debug()override;
    };


    struct Parameter : Node{
        bool is_byval = false;
        bool is_optional = false;
        expr::ID *name = nullptr;
        Annotation *annotation = nullptr;
        DebugInfo *debug()override;
    };

    namespace stmt{

        struct Statement : Node{
            enum Enum{
                error,let_,loop_,if_,for_,select_,return_,continue_,exit_,expr_
            } stmt_flag=error;
        };

        struct Let : Statement{
            Let(){stmt_flag = let_;}
            bool is_const = false;
            std::list<Variable*> variable_list{};
            DebugInfo *debug()override;
        };

        struct Select : Statement{
            Select(){stmt_flag = select_;}
            Expr *condition = nullptr;
            std::list<Case*> case_list{};
            DebugInfo *debug()override;
        };

        struct Loop : Statement{
            Loop(){stmt_flag = loop_;}
            Expr *condition = nullptr;
            std::list<Statement*> statement_list{};
            DebugInfo *debug()override;
        };

        struct If : Statement{
            If(){stmt_flag = if_;}
            std::list<Case*> case_list;
            DebugInfo *debug()override;
        };

        struct For : Statement{
            For(){stmt_flag = for_;}
            expr::Expression *iterator = nullptr;
            bool iterator_has_let = false;
            Expr *begin = nullptr,*end = nullptr,*step = nullptr;
            std::list<Statement*> statement_list;
            DebugInfo *debug()override;
        };

        struct Return : Statement{
            Return(){stmt_flag = return_;}
            Expr *expr = nullptr;
            DebugInfo *debug()override;
        };

        struct Exit : Statement{
            Exit(){stmt_flag = exit_;}
            enum {For,While,Sub}exit_flag;
            DebugInfo *debug()override;
        };

        struct Continue : Statement{
            Continue(){stmt_flag = continue_;}
            DebugInfo *debug()override;
        };

        struct ExprStmt : Statement{
            ExprStmt(){stmt_flag = expr_;}
            Expr *expr;
            DebugInfo *debug()override;
        };
    }

    struct Case : Node{
        Expr *condition = nullptr;
        std::list<stmt::Statement*> statement_list;
        DebugInfo *debug()override;
    };

    struct AnnotationUnit : Node{
        expr::ID *name = nullptr;
        DebugInfo *debug()override;
    };

    struct Annotation : Node{
        std::list<AnnotationUnit*> unit_list;
        expr::Digit *array_size = nullptr;
        DebugInfo *debug()override;
    };

    namespace expr{

        struct Expression : Node{
            enum ExpressionKind{
                error_ = 0,binary_,unary_,parentheses_,ID_,cast_,
                digit_,decimal_,string_,char_,boolean_,callee_
            }expression_kind = error_;
            DebugInfo *debug()override;
            ExpressionType *type = nullptr;
        };

        struct Binary : Expression{
            Binary(){expression_kind = ExpressionKind::binary_;}
            enum Enum{
                Empty,And,Or,Xor,Not,EQ,NE,GE,LE,GT,LT,
                ADD,MINUS,MUL,DIV,FDIV,ASSIGN,Dot,Index
            };
            Binary(Expression *lhs,Enum op,Expression *rhs):Binary(){
                NotNull(lhs);
                NotNull(rhs);
                this->lhs = lhs;
                this->rhs = rhs;
                this->op = op;
                this->location = new Location(lhs->location,rhs->location);
            }
            Expression *lhs = nullptr;
            Enum op = Empty;
            Expression *rhs = nullptr;
            std::shared_ptr<type::Variable> temp_address = nullptr;
            DebugInfo *debug()override;
        };

        struct Cast : Expression{
            Cast(Expression *expression,Annotation *annotation){
                expression_kind = cast_;
                this->expr = expression;
                this->annotation = annotation;
            }
            Expression *expr = nullptr;
            Annotation *annotation = nullptr;
            DebugInfo *debug()override;
        };

        struct Unary : Expression{
            Unary(){expression_kind = ExpressionKind::unary_;}
            enum Enum{Empty,MINUS,ADD}op = Empty;
            Expression *terminal = nullptr;
            Unary(Enum op,Expression *terminal):Unary(){
                this->op = op;
                this->terminal = terminal;
                this->location = terminal->location;
            }
            DebugInfo *debug()override;
        };

        struct Parentheses : Expression{
            Parentheses(){
                expression_kind = ExpressionKind::parentheses_;
            }
            Expression *expr = nullptr;
            DebugInfo *debug()override;
        };

        struct ID : Expression{
            ID(){
                expression_kind = ExpressionKind::ID_;
            }
            std::string lexeme;
            DebugInfo *debug()override;
        };

        struct Callee : Expression{
            struct Argument : Node{
                enum PassKind{undefined,byref,byval}pass_kind = undefined;
                std::shared_ptr<type::Variable> temp_address = nullptr;
                expr::Expression *expr = nullptr;
                DebugInfo *debug()override;
            };

            Callee(){
                expression_kind = ExpressionKind::callee_;
            }
            ID *name = nullptr;
            std::vector<Argument*> arg_list;
            DebugInfo *debug()override;
        };

        struct Digit : Expression{
            Digit(){
                expression_kind = ExpressionKind::digit_;
            }
            data::i32 value = 0;
            DebugInfo *debug()override;
        };

        struct Decimal : Expression{
            Decimal(){
                expression_kind = ExpressionKind::decimal_;
            }
            data::f64 value = 0;
            DebugInfo *debug()override;
        };

        struct String : Expression{
            String(){
                expression_kind = ExpressionKind::string_;
            }
            std::string value;
            DebugInfo *debug()override;
        };

        struct Char : Expression{
            Char(){
                expression_kind = ExpressionKind::char_;
            }
            data::i8 value = 0;
            DebugInfo *debug()override;
        };

        struct Boolean : Expression{
            Boolean(){
                expression_kind = ExpressionKind::boolean_;
            }
            data::boolean value = false;
            DebugInfo *debug()override;
        };
    }

}



#endif