//
// Created by yorkin on 10/26/21.
//

#include "ast.h"
#include "nullSafe.h"
#include <ostream>
#include <sstream>
#include <string>

using namespace std;
namespace evoBasic::ast{
    std::vector<std::string> AccessFlagToString={
        "Public","Private","Friend","Protected"
    };

    const std::string prefix_unit = "\033[32m│  \033[0m";


    void debugAST(ostream &stream,DebugInfo *info,string prefix,bool is_last){
        const string indent_unit = "\t";
        stream<<prefix;
        if(is_last)stream<<"└─";else stream<<"├─";
        stream<<info->text<<endl;
        prefix += is_last ? " " : "│";
        for(int i=0;i<info->text.size()/2+1;i++)prefix+=' ';
        for(auto child:info->childs){
            debugAST(stream,child,prefix,(child == info->childs.back()));
        }
    }

    string debugAST(Node *ast){
        stringstream stream;
        auto info = ast->debug();
        debugAST(stream,info,"",true);
        delete info;
        return stream.str();
    }


    DebugInfo *Global::debug() {
        auto ret = new DebugInfo{"Global"};
        for(auto m:member_list)ret->childs.push_back(m->debug());
        return ret;
    }

    DebugInfo *Class::debug() {
        auto ret = new DebugInfo{"Class",{name->debug()}};
        for(auto m:member_list)ret->childs.push_back(m->debug());
        return ret;
    }

    DebugInfo *Module::debug() {
        auto ret = new DebugInfo{"Module",{name->debug()}};
        for(auto m:member_list)ret->childs.push_back(m->debug());
        return ret;
    }

    DebugInfo *Import::debug() {
        return new DebugInfo{"Import",{annotation->debug()}};
    }

    DebugInfo *Dim::debug() {
        auto ret = new DebugInfo{format()<<"Dim "<<(is_const?"const":"")};
        for(auto var:variable_list)ret->childs.push_back(var->debug());
        return ret;
    }

    DebugInfo *Variable::debug() {
        auto ret = new DebugInfo{"Variable",{name->debug()}};
        if(annotation)ret->childs.push_back(annotation->debug());
        if(initial)ret->childs.push_back(initial->debug());
        return ret;
    }

    DebugInfo *Function::debug() {
        vector flag = {"Static","Virtual","Override","None"};
        auto ret = new DebugInfo{format()<<"Function "<<flag[(int)method_flag],{name->debug()}};
        for(auto p:parameter_list)ret->childs.push_back(p->debug());
        ret->childs.push_back(return_type->debug());
        for(auto stmt:statement_list)ret->childs.push_back(stmt->debug());
        return ret;
    }

    DebugInfo *External::debug() {
        auto ret = new DebugInfo{"External",{name->debug()}};
        if(lib)ret->childs.push_back(lib->debug());
        if(alias)ret->childs.push_back(alias->debug());
        for(auto p:parameter_list)ret->childs.push_back(p->debug());
        ret->childs.push_back(return_annotation->debug());
        return ret;
    }

    DebugInfo *Init::debug() {
        return nullptr;
    }

    DebugInfo *Operator::debug() {
        return nullptr;
    }

    DebugInfo *Enum::debug() {
        auto ret = new DebugInfo{"Enum",{name->debug()}};
        for(auto em:this->member_list){
            ret->childs.push_back(em.first->debug());
            ret->childs.push_back(em.second->debug());
        }
        return ret;
    }

    DebugInfo *Type::debug() {
        auto ret = new DebugInfo{"Type",{name->debug()}};
        for(auto var:member_list)
            ret->childs.push_back(var->debug());
        return ret;
    }

    DebugInfo *Parameter::debug() {
        string text = "Parameter";

        if(is_byval)text += " ByVal";
        else text += " ByRef";

        if(is_optional)text += " Optional";

        return new DebugInfo{text,{name->debug(),annotation->debug()}};
    }

    DebugInfo *stmt::Let::debug() {
        auto ret = new DebugInfo{"Let"};
        for(auto var:variable_list)
            ret->childs.push_back(var->debug());
        return ret;
    }

    DebugInfo *stmt::Select::debug() {
        auto ret = new DebugInfo{"Select",{condition->debug()}};
        for(auto case_:case_list)
            ret->childs.push_back(case_->debug());
        return ret;
    }

    DebugInfo *stmt::Loop::debug() {
        auto ret = new DebugInfo{"While",{condition->debug()}};
        for(auto stmt : statement_list)
            ret->childs.push_back(stmt->debug());
        return ret;
    }

    DebugInfo *stmt::If::debug() {
        auto ret = new DebugInfo{"If"};
        for(auto case_:case_list)
            ret->childs.push_back(case_->debug());
        return ret;
    }

    DebugInfo *stmt::For::debug() {
        auto ret = new DebugInfo{"For",{begin->debug(),end->debug()}};
        if(step)ret->childs.push_back(step->debug());
        for(auto stmt : statement_list)
            ret->childs.push_back(stmt->debug());
        return ret;
    }

    DebugInfo *stmt::Return::debug() {
        return new DebugInfo{"Return",{expr->debug()}};
    }

    DebugInfo *stmt::Exit::debug() {
        vector<string> flag = {"For","While","Sub"};
        return new DebugInfo{format()<<"Exit "<<flag[exit_flag]};
    }

    DebugInfo *stmt::Continue::debug() {
        return new DebugInfo{"Continue"};
    }

    DebugInfo *stmt::ExprStmt::debug() {
        return new DebugInfo{"ExprStmt",{expr->debug()}};
    }

    DebugInfo *Case::debug() {
        auto ret = new DebugInfo{"Case"};
        if(condition)ret->childs.push_back(condition->debug());
        for(auto stmt:statement_list)
            ret->childs.push_back(stmt->debug());
        return ret;
    }

    DebugInfo *AnnotationUnit::debug() {
        return new DebugInfo{"AnnotationUnit",{name->debug()}};
    }

    DebugInfo *Annotation::debug() {
        auto ret = new DebugInfo{"Annotation"};
        for(auto unit:unit_list)
            ret->childs.push_back(unit->debug());
        if(array_size)
            ret->childs.push_back(array_size->debug());
        return ret;
    }

    DebugInfo *expr::Expression::debug() {
        return new DebugInfo{"< Expression Error >"};
    }

    DebugInfo *expr::Binary::debug() {
        vector<string> OpStr = {"Empty","And","Or","Xor","Not","'=='","'<>'","'>='","'<='","'>='","'<'",
                        "'+'","'-'","'*'","'/'","'\\'","'='","'.'","Index"};
        return new DebugInfo{OpStr[op],{lhs->debug(),rhs->debug()}};
    }

    DebugInfo *expr::Cast::debug() {
        return new DebugInfo{"Cast",{expr->debug(),annotation->debug()}};
    }

    DebugInfo *expr::Unary::debug() {
        vector<string> OpStr = {"Empty","'-'","'+'"};
        return new DebugInfo{format()<<"Unary "<<OpStr[op],{terminal->debug()}};
    }

    DebugInfo *expr::Parentheses::debug() {
        return new DebugInfo{"Parentheses",{expr->debug()}};
    }

    DebugInfo *expr::ID::debug() {
        return new DebugInfo{format()<<"ID '"<<lexeme<<"'"};
    }

    DebugInfo *expr::Callee::Argument::debug() {
        vector<string> PassKindStr{"Undefined","ByRef","ByVal"};
        return new DebugInfo{format()<<"Argument "<<PassKindStr[pass_kind],{expr->debug()}};
    }

    DebugInfo *expr::Callee::debug() {
        auto ret = new DebugInfo{"Callee"};
        ret->childs.push_back(name->debug());
        for(auto arg:arg_list)
            ret->childs.push_back(arg->debug());
        return ret;
    }

    DebugInfo *expr::Digit::debug() {
        return new DebugInfo{format()<<"Digit "<<value};
    }

    DebugInfo *expr::Decimal::debug() {
        return new DebugInfo{format()<<"Decimal "<<value};
    }

    DebugInfo *expr::String::debug() {
        return new DebugInfo{format()<<"String \""<<value<<"\""};
    }

    DebugInfo *expr::Char::debug() {
        return new DebugInfo{format()<<"Char '"<<value<<"'"};
    }

    DebugInfo *expr::Boolean::debug() {
        return new DebugInfo{format()<<"Boolean "<<(value?"true":"false")};
    }
}

