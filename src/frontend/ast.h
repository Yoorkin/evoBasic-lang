#ifndef EB_AST
#define EB_AST
#include "token.h"
#include "data.h"
#include "nullSafe.h"
#include <list>
#include <memory>

namespace evoBasic{
    enum class AccessFlag {Public,Private,Friend,Protected};
    enum class MethodFlag {Static,Virtual,Override,NonMethod};
    struct ExpressionType;
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
    struct ID;
    struct Annotation;
    struct AnnotationUnit;
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
        struct GenericArgs;
        struct ArgsList;
        namespace literal{
            struct Digit;
            struct Decimal;
            struct String;
            struct Char;
        }
    }



    using Expr = expr::Expression;

    struct Node{
        Location *location = nullptr;
        virtual void debug(std::ostream &stream,std::string prefix)=0;
    };

    struct Global : Node{
        std::list<Member*> member_list{};
        void debug(std::ostream &stream,std::string prefix)override;
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
        ID *name = nullptr;
        Annotation *extend = nullptr;
        std::list<Annotation*> impl_list{};
        std::list<Member*> member_list{};
        void debug(std::ostream &stream,std::string prefix)override;
    };

    struct Module : Member{
        Module(){member_kind = MemberKind::module_;}
        ID *name = nullptr;
        std::list<Member*> member_list;
        void debug(std::ostream &stream,std::string prefix)override;
    };


    struct Import : Member{
        Import(){member_kind = MemberKind::import_;}
        Annotation *annotation = nullptr;
        void debug(std::ostream &stream,std::string prefix)override;
    };

    struct Dim : Member{
        Dim(){member_kind = MemberKind::dim_;}
        bool is_const = false;
        std::list<Variable*> variable_list{};
        void debug(std::ostream &stream,std::string prefix)override;
    };

    struct Variable : Node{
        ID *name = nullptr;
        Annotation *annotation = nullptr;
        Expr *initial = nullptr;
        void debug(std::ostream &stream,std::string prefix)override;
    };

    struct Function : Member{
        Function(){member_kind = MemberKind::function_;}
        MethodFlag method_flag;
        ID *name = nullptr;
        std::list<Parameter*> parameter_list{};
        Annotation *return_type = nullptr;
        std::list<stmt::Statement*> statement_list{};
        void debug(std::ostream &stream,std::string prefix)override;
    };

    struct External : Member{
        External(){member_kind = MemberKind::external_;}
        ID *name = nullptr;
        expr::literal::String *lib = nullptr,*alias = nullptr;
        std::list<Parameter*> parameter_list{};
        Annotation *return_annotation = nullptr;
        void debug(std::ostream &stream,std::string prefix)override;
    };

    struct Init : Member{
        Init(){member_kind = MemberKind::init_;}
        std::list<Parameter*> parameter_list;
        std::list<stmt::Statement*> statement_list{};
        void debug(std::ostream &stream,std::string prefix)override;
    };

    struct Operator : Member{
        Operator(){member_kind = MemberKind::operator_;}
        ID *name = nullptr;
        std::list<Parameter*> parameter_list{};
        Annotation *return_annotation;
        std::list<stmt::Statement*> statement_list{};
        void debug(std::ostream &stream,std::string prefix)override;
    };

    using EnumMember = std::pair<ID*,expr::literal::Digit*>;
    struct Enum : Member{
        Enum(){member_kind = MemberKind::enum_;}
        ID *name;
        std::list<EnumMember> member_list{};
        void debug(std::ostream &stream,std::string prefix)override;
    };

    using TypeMember = std::pair<ID*,Annotation*>;
    struct Type : Member{
        Type(){member_kind = MemberKind::type_;}
        ID *name;
        std::list<TypeMember> member_list{};
        void debug(std::ostream &stream,std::string prefix)override;
    };


    struct Parameter : Node{
        bool is_byval = false;
        bool is_optional = false;
        ID *name = nullptr;
        Annotation *annotation = nullptr;
        void debug(std::ostream &stream,std::string prefix)override;
    };

    struct ID : Node{
        std::string lexeme;
        void debug(std::ostream &stream,std::string prefix)override;
    };

    struct Annotation : Node{
        std::list<AnnotationUnit*> unit_list{};
        bool is_array = false;
        void debug(std::ostream &stream,std::string prefix)override;
    };

    struct AnnotationUnit : Node{
        ID *name = nullptr;
        expr::GenericArgs *generic_args;
        void debug(std::ostream &stream,std::string prefix)override;
    };

    struct Generic : Node{
        std::list<ID*> generic_list{};
        void debug(std::ostream &stream,std::string prefix)override;
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
            void debug(std::ostream &stream,std::string prefix)override;
        };

        struct Select : Statement{
            Select(){stmt_flag = select_;}
            Expr *condition = nullptr;
            std::list<Case*> case_list{};
            void debug(std::ostream &stream,std::string prefix)override;
        };

        struct Loop : Statement{
            Loop(){stmt_flag = loop_;}
            Expr *condition = nullptr;
            std::list<Statement*> statement_list{};
            void debug(std::ostream &stream,std::string prefix)override;
        };

        struct If : Statement{
            If(){stmt_flag = if_;}
            std::list<Case*> case_list{};
            void debug(std::ostream &stream,std::string prefix)override;
        };

        struct For : Statement{
            For(){stmt_flag = for_;}
            expr::Expression *iterator = nullptr;
            bool iterator_has_let = false;
            Expr *begin = nullptr,*end = nullptr,*step = nullptr;
            std::list<Statement*> statement_list{};
            void debug(std::ostream &stream,std::string prefix)override;
        };

        struct Return : Statement{
            Return(){stmt_flag = return_;}
            Expr *expr = nullptr;
            void debug(std::ostream &stream,std::string prefix)override;
        };

        struct Exit : Statement{
            Exit(){stmt_flag = exit_;}
            enum {For,While,Sub}exit_flag;
            void debug(std::ostream &stream,std::string prefix)override;
        };

        struct Continue : Statement{
            Continue(){stmt_flag = continue_;}
            void debug(std::ostream &stream,std::string prefix)override;
        };

        struct ExprStmt : Statement{
            ExprStmt(){stmt_flag = expr_;}
            Expr *expr;
            void debug(std::ostream &stream,std::string prefix)override;
        };
    }


    struct Case : Node{
        Expr *condition = nullptr;
        std::list<stmt::Statement*> statement_list{};
        void debug(std::ostream &stream,std::string prefix)override;
    };

    namespace expr{

        struct Expression : Node{
            enum ExpressionKind{
                error_ = 0,binary_,cast_,unary_,link_,terminal_
            }expression_kind = error_;
            void debug(std::ostream &stream,std::string prefix)override;
            ExpressionType *type = nullptr;
        };

        struct Binary : Expression{
            Binary(){expression_kind = ExpressionKind::binary_;}
            enum Enum{
                Empty,And,Or,Xor,Not,EQ,NE,GE,LE,GT,LT,
                ADD,MINUS,MUL,DIV,FDIV,ASSIGN
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
            void debug(std::ostream &stream,std::string prefix)override;
        };

        struct Cast : Expression{
            Cast()=delete;
            Cast(Expression *src,Annotation *dst){
                expression_kind = ExpressionKind::cast_;
                this->src = src;
                this->dst = dst;
                this->location = new Location(src->location,dst->location);
            }
            Expression *src = nullptr;
            Annotation *dst = nullptr;
            void debug(std::ostream &stream,std::string prefix)override;
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
            void debug(std::ostream &stream,std::string prefix)override;
        };

        struct Link : Expression{
            Link(){expression_kind = ExpressionKind::link_;}
            std::list<Terminal*> terminal_list;
            void debug(std::ostream &stream,std::string prefix)override;
        };

        struct Terminal : Expression{
            enum TerminalKind{
                error = 0,parentheses_,digit_,decimal_,callee_,string_,char_,boolean_
            }terminal_kind = error;
            void debug(std::ostream &stream,std::string prefix)override;
        };

        struct Parentheses : Terminal{
            Parentheses(){
                expression_kind = ExpressionKind::terminal_;
                terminal_kind = TerminalKind::parentheses_;
            }
            Expression *expr = nullptr;
            void debug(std::ostream &stream,std::string prefix)override;
        };

        struct Callee : Terminal{
            Callee(){
                expression_kind = ExpressionKind::terminal_;
                terminal_kind = TerminalKind::callee_;
            }
            ID *name = nullptr;
            ArgsList *args = nullptr;
            GenericArgs *generic_args = nullptr;
            Expr *index_arg = nullptr;
            void debug(std::ostream &stream,std::string prefix)override;
        };

        struct GenericArgs : Node{
            std::list<Annotation*> annotation_list;
            void debug(std::ostream &stream,std::string prefix)override;
        };


        struct Arg : Node{
            enum PassKind{undefined,byref,byval}pass_kind = undefined;
            expr::Expression *expr = nullptr;
            void debug(std::ostream &stream,std::string prefix)override;
        };

        struct ArgsList : Node{
            std::list<Arg*> arg_list;
            void debug(std::ostream &stream,std::string prefix)override;
        };


        namespace literal{
            struct Digit : Terminal{
                Digit(){
                    expression_kind = ExpressionKind::terminal_;
                    terminal_kind = TerminalKind::digit_;
                }
                data::i32 value = 0;
                void debug(std::ostream &stream,std::string prefix)override;
            };

            struct Decimal : Terminal{
                Decimal(){
                    expression_kind = ExpressionKind::terminal_;
                    terminal_kind = TerminalKind::decimal_;
                }
                data::f64 value = 0;
                void debug(std::ostream &stream,std::string prefix)override;
            };

            struct String : Terminal{
                String(){
                    expression_kind = ExpressionKind::terminal_;
                    terminal_kind = TerminalKind::string_;
                }
                std::string value;
                void debug(std::ostream &stream,std::string prefix)override;
            };

            struct Char : Terminal{
                Char(){
                    expression_kind = ExpressionKind::terminal_;
                    terminal_kind = TerminalKind::char_;
                }
                data::i8 value = 0;
                void debug(std::ostream &stream,std::string prefix)override;
            };

            struct Boolean : Terminal{
                Boolean(){
                    expression_kind = ExpressionKind::terminal_;
                    terminal_kind = TerminalKind::boolean_;
                }
                data::boolean value = false;
                void debug(std::ostream &stream,std::string prefix)override;
            };
        }
    }

}



#endif