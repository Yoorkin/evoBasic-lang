//
// Created by yorkin on 10/26/21.
//

#include "ast.h"
#include "nullSafe.h"
using namespace std;
namespace evoBasic::ast{
    std::vector<std::string> AccessFlagToString={
            "Public","Private","Friend","Protected"
    };

    const std::string prefix_unit = "\033[32m│  \033[0m";

    void Global::debug(std::ostream &stream, std::string prefix) {
        stream<<prefix<<"================== AST Begin ==================";
        if(!member_list.empty())stream<<'\n';
        for(auto& m:member_list){
            m->debug(stream,prefix);
        }
        stream<<prefix<<"==================  AST End  ==================\n";
    }

    void Class::debug(std::ostream &stream, std::string prefix) {
        name->debug(stream,prefix);
        stream<<" : Class{\n";
        if(!member_list.empty())stream<<'\n';
        for(auto& m:member_list){
            m->debug(stream,prefix + prefix_unit);
        }
        stream<<prefix<<"}\n";
    }

    void Module::debug(std::ostream &stream, std::string prefix) {
        name->debug(stream,prefix);
        stream<<" : Module{";
        if(!member_list.empty())stream<<'\n';
        for(auto& m:member_list){
            m->debug(stream,prefix + prefix_unit);
        }
        stream<<prefix<<"}\n";
    }

    void Import::debug(std::ostream &stream, std::string prefix) {
        stream<<prefix<<"Import{\n";
        annotation->debug(stream,prefix);
        stream<<prefix<<"}\n";
    }

    void Dim::debug(std::ostream &stream, std::string prefix) {
        stream<<prefix<<"Dim{";
        if(!variable_list.empty())stream<<'\n';
        for(auto& v:variable_list){
            v->debug(stream,prefix + prefix_unit);
            stream<<'\n';
        }
        stream<<prefix<<"}\n";
    }

    void Variable::debug(std::ostream &stream, std::string prefix) {
        name->debug(stream,prefix);
        stream<<" : ";
        if(annotation) annotation->debug(stream,"");
        else stream<<" ? ";
        if(initial) {
            stream<<" = {\n";
            initial->debug(stream,prefix_unit + prefix);
            stream<<prefix<<"}";
        }
    }

    void Function::debug(std::ostream &stream, std::string prefix) {
        name->debug(stream,prefix);
        stream<<" : Function(";
        for(auto& p:parameter_list){
            auto str = (&p == &parameter_list.front()) ? "" : ".";
            p->debug(stream,str);
        }
        stream<<')';
        if(return_type){
            return_type->debug(stream," As ");
        }
        stream<<"{\n";
        for(auto& s:statement_list){
            s->debug(stream,prefix + prefix_unit);
        }
        stream<<prefix<<"}\n";
    }

    void External::debug(std::ostream &stream, std::string prefix) {
        name->debug(stream,prefix);
        stream<<" : Function(";
        for(auto& p:parameter_list){
            auto str = (&p == &parameter_list.front()) ? "" : ".";
            p->debug(stream,str);
        }
        stream<<')';
        if(return_annotation){
            return_annotation->debug(stream," As ");
        }
    }

    void Init::debug(std::ostream &stream, std::string prefix) {
        stream<<prefix<<"Init(";
        for(auto& p:parameter_list){
            auto str = (&p == &parameter_list.front()) ? "" : ".";
            p->debug(stream,str);
        }
        stream<<"){\n";
        for(auto& s:statement_list){
            s->debug(stream,prefix + prefix_unit);
        }
        stream<<prefix<<"}\n";
    }

    void Operator::debug(std::ostream &stream, std::string prefix) {
        name->debug(stream,prefix);
        stream<<" : Function(";
        for(auto& p:parameter_list){
            auto str = (&p == &parameter_list.front()) ? "" : ".";
            p->debug(stream,str);
        }
        stream<<')';
        if(return_annotation){
            return_annotation->debug(stream," As ");
        }
        stream<<"{\n";
        for(auto& s:statement_list){
            s->debug(stream,prefix + prefix_unit);
        }
        stream<<prefix<<"}\n";
    }

    void Enum::debug(std::ostream &stream, std::string prefix) {
        name->debug(stream,prefix);
        stream<<" : Enum{";
        if(member_list.empty())stream<<" }\n";
        else {
            stream<<'\n';
            for(auto& m:member_list){
                m.first->debug(stream,prefix + prefix_unit);
                if(m.second!=nullptr){
                    m.second->debug(stream," : ");
                }
                stream<<'\n';
            }
            stream<<prefix<<"}\n";
        }
    }

    void Type::debug(std::ostream &stream, std::string prefix) {
        name->debug(stream,prefix);
        stream<<" : type{";
        if(member_list.empty())stream<<" }\n";
        else {
            stream<<'\n';
            for(auto& m:member_list){
                m->debug(stream,"");
            }
            stream<<prefix<<"}\n";
        }
    }

    void Parameter::debug(std::ostream &stream, std::string prefix) {
        stream<<prefix;
        if(is_optional)stream<<"Optional ";
        if(is_byval)stream<<"Byval ";
        name->debug(stream,"");
        annotation->debug(stream," : ");
    }

    void expr::ID::debug(std::ostream &stream, std::string prefix) {
        stream<<prefix<<lexeme;
    }


    void stmt::Let::debug(std::ostream &stream, std::string prefix) {
        stream<<prefix<<"Let{\n";
        for(auto& v:variable_list){
            v->debug(stream,prefix + prefix_unit);
            stream<<'\n';
        }
        stream<<prefix<<"}\n";
    }

    void stmt::Select::debug(std::ostream &stream, std::string prefix) {
        stream<<prefix<<"Select{\n"
            <<prefix<<prefix_unit<<"Condition{\n";
        condition->debug(stream,prefix + prefix_unit + prefix_unit);
        stream<<prefix<<prefix_unit<<"}\n";
        for(auto &c:case_list){
            c->debug(stream,prefix + prefix_unit);
        }
    }

    void stmt::Loop::debug(std::ostream &stream, std::string prefix) {
        stream<<prefix<<"Loop{\n"
            <<prefix<<prefix_unit<<"Condition{\n";
        condition->debug(stream,prefix + prefix_unit + prefix_unit);
        stream<<prefix<<prefix_unit<<"}\n";
        for(auto &s:statement_list){
            s->debug(stream,prefix + prefix_unit);
        }
        stream<<prefix<<"}\n";
    }

    void stmt::If::debug(std::ostream &stream, std::string prefix) {
        stream<<prefix<<"If{\n";
        for(auto &c:case_list){
            c->debug(stream,prefix + prefix_unit);
        }
        stream<<prefix<<"}\n";
    }

    void stmt::For::debug(std::ostream &stream, std::string prefix) {
        stream<<prefix<<"For{\n";
        stream<<prefix<<prefix_unit<<"Begin{\n";
        begin->debug(stream,prefix + prefix_unit + prefix_unit);
        stream<<prefix<<prefix_unit<<"}\n";

        stream<<prefix<<prefix_unit<<"End{\n";
        end->debug(stream,prefix + prefix_unit + prefix_unit);
        stream<<prefix<<prefix_unit<<"}\n";

        if(step){
            stream<<prefix<<prefix_unit<<"Step{\n";
            step->debug(stream,prefix + prefix_unit + prefix_unit);
            stream<<prefix_unit<<prefix<<"}\n";
        }

        for(auto &s:statement_list){
            s->debug(stream,prefix + prefix_unit);
        }

        stream<<prefix<<"}\n";
    }

    void stmt::Return::debug(std::ostream &stream, std::string prefix) {
        stream<<prefix<<"Return{\n";
        this->expr->debug(stream,prefix + prefix_unit);
        stream<<prefix<<"}\n";
    }

    void stmt::Exit::debug(std::ostream &stream, std::string prefix) {
        stream<<prefix<<"Exit ";
        switch(exit_flag){
            case For:
                stream<<"For";
                break;
            case While:
                stream<<"While";
                break;
            case Sub:
                stream<<"Sub";
                break;
        }
        stream<<'\n';
    }

    void stmt::Continue::debug(std::ostream &stream, std::string prefix) {
        stream<<prefix<<"Continue"<<'\n';
    }

    void stmt::ExprStmt::debug(std::ostream &stream, std::string prefix) {
        this->expr->debug(stream,move(prefix));
    }

    void Case::debug(std::ostream &stream, std::string prefix) {
        stream<<prefix<<"Case{\n";
        if(condition){
            stream<<prefix<<prefix_unit<<"Condition{\n";
            condition->debug(stream,prefix + prefix_unit + prefix_unit);
            stream<<prefix<<prefix_unit<<"}\n";
        }
        for(auto &s:statement_list){
            s->debug(stream,prefix + prefix_unit);
        }
        stream<<prefix<<"}\n";
    }

    void expr::Binary::debug(std::ostream &stream, std::string prefix) {
        stream<<prefix;
        switch (op) {
            case And: stream<<"And"; break;
            case Or:  stream<<"Or";  break;
            case Xor: stream<<"Xor"; break;
            case Not: stream<<"Not"; break;
            case EQ: stream<<"=="; break;
            case NE: stream<<"!="; break;
            case GE: stream<<">="; break;
            case LE: stream<<"<="; break;
            case GT: stream<<">";  break;
            case LT: stream<<"<";  break;
            case ADD: stream<<"+ "; break;
            case MINUS: stream<<"- "; break;
            case MUL: stream<<"* "; break;
            case DIV: stream<<"/ "; break;
            case FDIV:stream<<"\\ "; break;
        }
        stream<<"{\n";
        lhs->debug(stream,prefix + prefix_unit);
        rhs->debug(stream,prefix + prefix_unit);
        stream<<prefix<<"}\n";
    }

    void expr::Unary::debug(std::ostream &stream, std::string prefix) {
        stream<<prefix;
        switch (op) {
            case MINUS: stream<<"-"; break;
            case ADD:   stream<<"+"; break;
        }
        stream<<"{\n";
        terminal->debug(stream,prefix_unit + prefix);
        stream<<prefix<<"}\n";
    }

    void expr::Callee::debug(std::ostream &stream, std::string prefix) {
        name->debug(stream,prefix);
        stream<<prefix<<" ArgsList(\n";
        for(auto &arg:arg_list)
            arg->debug(stream,prefix+prefix_unit);
        stream<<prefix<<")";
    }


    void expr::Digit::debug(std::ostream &stream, std::string prefix) {
        stream<<prefix<<to_string(value);
    }

    void expr::Decimal::debug(std::ostream &stream, std::string prefix) {
        stream<<prefix<<to_string(value);
    }

    void expr::String::debug(std::ostream &stream, std::string prefix) {
        stream<<prefix<<'"'<<value<<'"';
    }

    void expr::Char::debug(std::ostream &stream, std::string prefix) {
        stream<<prefix<<'\''<<value<<'\'';
    }

    void expr::Boolean::debug(std::ostream &stream,std::string prefix) {
        stream<<prefix<<(value ? "true" : "false");
    }

    void expr::Expression::debug(ostream &stream, std::string prefix) {
        stream<<"< Exp Error >\n";
    }

    void expr::Parentheses::debug(ostream &stream, std::string prefix) {
        stream<<prefix<<"Parentheses(\n";
        this->expr->debug(stream,prefix + prefix_unit);
        stream<<prefix<<")";
    }

    void expr::Callee::Argument::debug(ostream &stream, std::string prefix) {
        switch (pass_kind) {
            case byval: stream<<"Byval "; break;
            case byref: stream<<"Byref";  break;
        }
        expr->debug(stream,prefix + prefix_unit);
    }

    void AnnotationUnit::debug(ostream &stream, std::string prefix) {

    }

    void Annotation::debug(ostream &stream, std::string prefix) {

    }

    void expr::Cast::debug(ostream &stream, std::string prefix) {
        Expression::debug(stream, prefix);
    }
}

