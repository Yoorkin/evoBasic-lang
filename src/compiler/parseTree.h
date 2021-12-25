#ifndef EB_AST
#define EB_AST
#include "token.h"
#include "data.h"
#include "nullSafe.h"
#include "enums.h"
#include <memory>
#include <list>

#define FOR_EACH(Iterator,Head) for(auto Iterator=Head; Iterator!=nullptr; Iterator=Iterator->next_sibling)

namespace evoBasic{
    struct ExpressionType;
    namespace type{
        class Variable;
        class Domain;
        class Class;
        class Function;
        class UserFunction;
        class ExternalFunction;
        class Constructor;
        class Enumeration;
        class EnumMember;
        class Interface;
        class Record;
        class Parameter;
    }

    struct DebugInfo{
    public:
        std::string text;
        std::list<DebugInfo*> childs;

        ~DebugInfo(){
            for(auto child:childs)delete child;
        }
    };
}

namespace evoBasic::parseTree{

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
    struct Constructor;
    struct Operator;
    struct Enum;
    struct Type;
    struct Parameter;
    struct Annotation;
    struct Generic;
    struct Implement;
    struct Interface;

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
        struct Binary;
        struct Unary;
        struct Terminal;
        struct Argument;
        struct Callee;
        struct ID;
        struct Digit;
        struct Decimal;
        struct String;
        struct Char;
    }

    using namespace stmt;
    using namespace expr;


    std::string debugParseTree(Node *ast);

    using Expr = expr::Expression;


    struct Node{
        Location *location = nullptr;
        bool has_error = false;
        virtual DebugInfo *debug()=0;
    };

    struct Global : Node{
        Member *member = nullptr;
        DebugInfo *debug()override;
        type::Domain *global_symbol = nullptr;
    };

    struct Member : Node{
        AccessFlag access;
        bool is_static = false;
        enum MemberKind{
            error,function_,class_,module_,type_,enum_,dim_,
            import_,external_,interface_,constructor_
        }member_kind = error;

        Member *next_sibling = nullptr,*prv_sibling = nullptr;
    };

    struct Class : Member{
        Class(){member_kind = MemberKind::class_;}
        expr::ID *name = nullptr;
        Annotation *extend = nullptr;
        Implement *impl = nullptr;

        Member *member = nullptr;
        DebugInfo *debug()override;
        type::Class *class_symbol = nullptr;
    };

    struct Module : Member{
        Module(){member_kind = MemberKind::module_;}
        expr::ID *name = nullptr;
        Member *member = nullptr;
        DebugInfo *debug()override;
        type::Domain *module_symbol = nullptr;
    };

    struct Implement : Node {
        Annotation* annotation = nullptr;
        Implement *next_sibling = nullptr,
                  *prv_sibling = nullptr;
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
        Variable* variable;
        DebugInfo *debug()override;
    };

    struct Variable : Node{
        expr::ID *name = nullptr;
        Annotation *annotation = nullptr;
        expr::Expression *initial = nullptr;
        DebugInfo *debug()override;
        Variable *next_sibling = nullptr,*prv_sibling = nullptr;
        type::Variable *variable_symbol = nullptr;
    };

    struct Function : Member{
        Function(){member_kind = MemberKind::function_;}
        MethodFlag method_flag;
        expr::ID *name = nullptr;
        Parameter *parameter = nullptr;
        Annotation *return_annotation = nullptr;
        stmt::Statement* statement = nullptr;
        DebugInfo *debug()override;
        type::Function *function_symbol = nullptr;
    };

    struct Constructor : Member{
        Constructor(){member_kind = MemberKind::constructor_;}
        Parameter *parameter = nullptr;
        stmt::Statement* statement = nullptr;
        type::Constructor *constructor_symbol = nullptr;
        DebugInfo *debug()override;
    };

    struct External : Member{
        External(){member_kind = MemberKind::external_;}
        expr::ID *name = nullptr;
        expr::String *lib = nullptr,*alias = nullptr;
        Parameter* parameter = nullptr;
        Annotation *return_annotation = nullptr;
        DebugInfo *debug()override;
        type::ExternalFunction *function_symbol = nullptr;
    };

    struct Interface : Member {
        Interface(){member_kind = interface_;}
        expr::ID *name = nullptr;
        Function *function = nullptr;
        type::Interface *interface_symbol = nullptr;
        DebugInfo *debug()override;
    };

    struct EnumMember : Node {
        expr::ID *name = nullptr;
        expr::Digit *value = nullptr;
        EnumMember *next_sibling = nullptr,*prv_sibling = nullptr;
        DebugInfo *debug()override;
        type::EnumMember *enum_member_symbol = nullptr;
    };

    struct Enum : Member{
        Enum(){member_kind = MemberKind::enum_;}
        expr::ID *name = nullptr;
        EnumMember *member = nullptr;
        DebugInfo *debug()override;
        type::Enumeration *enum_symbol = nullptr;
    };


    struct Type : Member{
        Type(){member_kind = MemberKind::type_;}
        expr::ID *name = nullptr;
        Variable *member = nullptr;
        DebugInfo *debug()override;
        type::Record *type_symbol = nullptr;
    };


    struct Parameter : Node{
        bool is_byval = false;
        bool is_optional = false;
        bool is_param_array = false;
        expr::ID *name = nullptr;
        Annotation *annotation = nullptr;
        expr::Expression *initial = nullptr;
        DebugInfo *debug()override;
        Parameter *next_sibling = nullptr,*prv_sibling = nullptr;
        type::Parameter *parameter_symbol = nullptr;
    };

    namespace stmt{

        struct Statement : Node{
            enum Enum{
                error,let_,loop_,if_,for_,select_,return_,continue_,exit_,expr_
            } stmt_flag=error;
            Statement *next_sibling = nullptr,*prv_sibling = nullptr;
        };

        struct Let : Statement{
            Let(){stmt_flag = let_;}
            bool is_const = false;
            Variable* variable = nullptr;
            DebugInfo *debug()override;
        };

        struct Select : Statement{
            Select(){stmt_flag = select_;}
            Expr *condition = nullptr;
            Case *case_ = nullptr;
            DebugInfo *debug()override;
        };

        struct Loop : Statement{
            Loop(){stmt_flag = loop_;}
            Expr *condition = nullptr;
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
            expr::Expression *iterator = nullptr;
            bool iterator_has_let = false;
            Expr *begin = nullptr,*end = nullptr,*step = nullptr;
            Statement* statement = nullptr;
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
        stmt::Statement *statement = nullptr;
        Case *next_sibling = nullptr,*prv_sibling = nullptr;
        DebugInfo *debug()override;
    };

    struct AnnotationUnit : Node{
        expr::ID *name = nullptr;
        DebugInfo *debug()override;
        AnnotationUnit *next_sibling = nullptr,*prv_sibling = nullptr;
    };

    struct Annotation : Node{
        AnnotationUnit* unit = nullptr;
        expr::Digit *array_size = nullptr;
        DebugInfo *debug()override;
    };

    namespace expr{

        struct Expression : Node{
            enum ExpressionKind{
                error_ = 0,binary_,unary_,parentheses_,ID_,cast_,
                digit_,decimal_,string_,char_,boolean_,callee_,new_,
                index_,dot_,assign_,colon_
            }expression_kind = error_;
            DebugInfo *debug()override;
        };

        struct Colon : Expression{
            Colon(Expression *lhs,Expression *rhs){
                expression_kind = colon_;
                this->location = new Location(lhs->location,rhs->location);
                this->lhs = lhs;
                this->rhs = rhs;
            }
            Expression *lhs = nullptr;
            Expression *rhs = nullptr;
            DebugInfo *debug()override;
        };

        struct Binary : Expression{
            Binary(){expression_kind = ExpressionKind::binary_;}
            enum Enum{
                Empty,And,Or,Xor,Not,EQ,NE,GE,LE,GT,LT,
                ADD,MINUS,MUL,DIV,FDIV
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
            type::Variable *temp_address = nullptr;
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

        struct Index : Expression{
            Index(Expression *target,Expression *value){
                expression_kind = index_;
                this->target = target;
                this->value = value;
            }
            Expression *target = nullptr,*value = nullptr;
            DebugInfo *debug()override;
        };

        struct Dot : Expression{
            Dot(Expression *lhs,Expression *rhs){
                expression_kind = dot_;
                this->lhs = lhs;
                this->rhs = rhs;
                this->location = new Location(lhs->location,rhs->location);
            }
            Expression *lhs = nullptr,*rhs = nullptr;
            DebugInfo *debug()override;
        };

        struct Assign : Expression{
            Assign(Expression *lhs,Expression *rhs){
                expression_kind = assign_;
                this->location = new Location(lhs->location,rhs->location);
                this->lhs = lhs;
                this->rhs = rhs;
            }
            Expression *lhs = nullptr;
            Expression *rhs = nullptr;
            DebugInfo *debug()override;
        };

        struct New : Expression{
            New(){
                expression_kind = new_;
            }
            Annotation *annotation = nullptr;
            Argument *argument = nullptr;
            DebugInfo * debug() override;
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

        struct Argument : Node{
            enum PassKind{undefined,byref,byval}pass_kind = undefined;
            expr::Expression *expr = nullptr;
            Argument *next_sibling = nullptr,*prv_sibling = nullptr;
            DebugInfo *debug()override;
        };

        struct Callee : Expression{
            Callee(){
                expression_kind = ExpressionKind::callee_;
            }
            ID *name = nullptr;
            Argument* argument = nullptr;
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
    using namespace expr;

}



#endif