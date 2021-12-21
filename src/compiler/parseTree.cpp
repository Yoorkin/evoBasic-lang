//
// Created by yorkin on 10/26/21.
//

#include "parseTree.h"
#include "nullSafe.h"
#include <ostream>
#include <sstream>
#include <string>
#include "ast.h"

using namespace std;
namespace evoBasic::parseTree{
    std::vector<std::string> AccessFlagToString={
        "Public","Private","Friend","Protected"
    };

    const std::string prefix_unit = "\033[32m│  \033[0m";


    void debugTree(ostream &stream,DebugInfo *info,string prefix,bool is_last){
        const string indent_unit = "\t";
        stream<<prefix;
        if(is_last)stream<<"└─";else stream<<"├─";
        stream<<info->text<<endl;
        prefix += is_last ? " " : "│";
        for(int i=0;i<info->text.size()/2+1;i++)prefix+=' ';

        for(auto iter:info->childs){
            debugTree(stream,iter,prefix,(iter == info->childs.back()));
        }
    }

    string debugParseTree(parseTree::Node *ast){
        stringstream stream;
        auto info = ast->debug();
        debugTree(stream,info,"",true);
        delete info;
        return stream.str();
    }

    DebugInfo *Global::debug() {
        auto ret = new DebugInfo{"Global"};
        auto iter = member;
        while(iter){
            ret->childs.push_back(iter->debug());
            iter = iter->next_sibling;
        }
        return ret;
    }

    DebugInfo *Class::debug() {
        string text = Format() << "Class"
                               << " " << AccessFlagToString[(int)access];
        auto ret = new DebugInfo{text,{name->debug()}};
        if(extend)ret->childs.push_back(extend->debug());
        auto iter = member;
        while(iter){
            ret->childs.push_back(iter->debug());
            iter = iter->next_sibling;
        }
        return ret;
    }

    DebugInfo *Module::debug() {
        string text = Format() << "Module "
                               << " " << AccessFlagToString[(int)access];
        auto ret = new DebugInfo{text,{name->debug()}};
        auto iter = member;
        while(iter){
            ret->childs.push_back(iter->debug());
            iter = iter->next_sibling;
        }
        return ret;
    }


    DebugInfo *Interface::debug() {
        string text = Format() << "Interface "
                               << " " << AccessFlagToString[(int)access];
        auto ret = new DebugInfo{text,{name->debug()}};
        Member *iter = function;
        while(iter){
            ret->childs.push_back(iter->debug());
            iter = iter->next_sibling;
        }
        return ret;
    }

    DebugInfo *Import::debug() {
        return new DebugInfo{"Import",{annotation->debug()}};
    }

    DebugInfo *Dim::debug() {
        string text = Format() << "Dim"
                               << " " << AccessFlagToString[(int)access]
                               << (is_const?" Const":"")
                               << (is_static?" Static":"");
        auto ret = new DebugInfo{text};
        auto iter = variable;
        while(iter){
            ret->childs.push_back(iter->debug());
            iter = iter->next_sibling;
        }
        return ret;
    }

    DebugInfo *Variable::debug() {
        auto ret = new DebugInfo{"Variable",{name->debug()}};
        if(annotation)ret->childs.push_back(annotation->debug());
        if(initial)ret->childs.push_back(initial->debug());
        return ret;
    }

    DebugInfo *Function::debug() {
        vector flag = {" Virtual"," Override",""};
        string text = Format() << "Function"
                               << " " << AccessFlagToString[(int)access]
                               << flag[(int)method_flag]
                               << (is_static?" Static":"");
        auto ret = new DebugInfo{text};
        if(name)ret->childs.push_back(name->debug());
        auto iter = parameter;
        while(iter){
            ret->childs.push_back(iter->debug());
            iter = iter->next_sibling;
        }

        if(return_annotation)
            ret->childs.push_back(return_annotation->debug());

        auto stmt = statement;
        while(stmt){
            ret->childs.push_back(stmt->debug());
            stmt = stmt->next_sibling;
        }
        return ret;
    }

    DebugInfo *External::debug() {
        auto ret = new DebugInfo{"External",{name->debug()}};
        if(lib)ret->childs.push_back(lib->debug());
        if(alias)ret->childs.push_back(alias->debug());
        auto iter = parameter;
        while(iter){
            ret->childs.push_back(iter->debug());
            iter = iter->next_sibling;
        }
        ret->childs.push_back(return_annotation->debug());
        return ret;
    }

    DebugInfo *Enum::debug() {
        string text = Format() << "Enum"
                               << " " << AccessFlagToString[(int)access];
        auto ret = new DebugInfo{text,{name->debug()}};
        auto iter = member;
        while(iter){
            ret->childs.push_back(iter->debug());
            iter = iter->next_sibling;
        }
        return ret;
    }

    DebugInfo *Type::debug() {
        string text = Format() << "Type"
                               << " " << AccessFlagToString[(int)access];
        auto ret = new DebugInfo{text,{name->debug()}};
        auto iter = member;
        while(iter){
            ret->childs.push_back(iter->debug());
            iter = iter->next_sibling;
        }
        return ret;
    }

    DebugInfo *Parameter::debug() {
        string text = "Parameter";

        if(is_byval)text += " ByVal";
        else text += " ByRef";

        if(is_optional)text += " Optional";

        auto ret = new DebugInfo{text,{name->debug(),annotation->debug()}};
        if(initial)ret->childs.push_back(initial->debug());
        return ret;
    }

    DebugInfo *stmt::Let::debug() {
        auto ret = new DebugInfo{"Let"};
        auto iter = variable;
        while(iter){
            ret->childs.push_back(iter->debug());
            iter = iter->next_sibling;
        }
        return ret;
    }

    DebugInfo *stmt::Select::debug() {
        auto ret = new DebugInfo{"Select",{condition->debug()}};
        auto iter = case_;
        while(iter){
            ret->childs.push_back(iter->debug());
            iter = iter->next_sibling;
        }
        return ret;
    }

    DebugInfo *stmt::Loop::debug() {
        auto ret = new DebugInfo{"While",{condition->debug()}};
        auto iter = statement;
        while(iter){
            ret->childs.push_back(iter->debug());
            iter = iter->next_sibling;
        }
        return ret;
    }

    DebugInfo *stmt::If::debug() {
        auto ret = new DebugInfo{"If"};
        auto iter = case_;
        while(iter){
            ret->childs.push_back(iter->debug());
            iter = iter->next_sibling;
        }
        return ret;
    }

    DebugInfo *stmt::For::debug() {
        auto ret = new DebugInfo{"For",{begin->debug(),end->debug()}};
        if(step)ret->childs.push_back(step->debug());
        auto iter = statement;
        while(iter){
            ret->childs.push_back(iter->debug());
            iter = iter->next_sibling;
        }
        return ret;
    }

    DebugInfo *stmt::Return::debug() {
        return new DebugInfo{"Return",{expr->debug()}};
    }

    DebugInfo *stmt::Exit::debug() {
        vector<string> flag = {"For","While","Sub"};
        return new DebugInfo{Format() << "Exit " << flag[exit_flag]};
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
        auto iter = statement;
        while(iter){
            ret->childs.push_back(iter->debug());
            iter = iter->next_sibling;
        }
        return ret;
    }

    DebugInfo *AnnotationUnit::debug() {
        return new DebugInfo{"AnnotationUnit",{name->debug()}};
    }

    DebugInfo *Annotation::debug() {
        auto ret = new DebugInfo{"Annotation"};
        auto iter = unit;
        while(iter){
            ret->childs.push_back(iter->debug());
            iter = iter->next_sibling;
        }
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
        return new DebugInfo{Format() << "Unary " << OpStr[op], {terminal->debug()}};
    }

    DebugInfo *expr::Parentheses::debug() {
        return new DebugInfo{"Parentheses",{expr->debug()}};
    }

    DebugInfo *expr::ID::debug() {
        return new DebugInfo{Format() << "ID '" << lexeme << "'"};
    }

    DebugInfo *expr::Argument::debug() {
        vector<string> PassKindStr{"Undefined","ByRef","ByVal"};
        return new DebugInfo{Format() << "Parameter " << PassKindStr[pass_kind], {expr->debug()}};
    }

    DebugInfo *expr::Callee::debug() {
        auto ret = new DebugInfo{"Callee"};
        ret->childs.push_back(name->debug());
        auto iter = argument;
        while(iter){
            ret->childs.push_back(iter->debug());
            iter = iter->next_sibling;
        }
        return ret;
    }

    DebugInfo *expr::Digit::debug() {
        return new DebugInfo{Format() << "Digit " << value};
    }

    DebugInfo *expr::Decimal::debug() {
        return new DebugInfo{Format() << "Decimal " << value};
    }

    DebugInfo *expr::String::debug() {
        return new DebugInfo{Format() << "String \"" << value << "\""};
    }

    DebugInfo *expr::Char::debug() {
        return new DebugInfo{Format() << "Char '" << value << "'"};
    }

    DebugInfo *expr::Boolean::debug() {
        return new DebugInfo{Format() << "Boolean " << (value ? "true" : "false")};
    }

    DebugInfo *EnumMember::debug() {
        return new DebugInfo{"EnumMember"};
    }

    DebugInfo *Implement::debug() {
        return new DebugInfo{"Implement"};
    }

    DebugInfo *expr::New::debug() {
        auto ret = new DebugInfo{"New",{annotation->debug()}};
        auto iter = argument;
        while(iter){
            ret->childs.push_back(iter->debug());
            iter = iter->next_sibling;
        }
        return ret;
    }

    DebugInfo *expr::Index::debug() {
        return new DebugInfo{"Index",{target->debug(),value->debug()}};
    }

    DebugInfo *expr::Dot::debug() {
        return new DebugInfo{"Dot",{lhs->debug(),rhs->debug()}};
    }

    DebugInfo *expr::Assign::debug() {
        return new DebugInfo{"'='",{lhs->debug(),rhs->debug()}};
    }

    DebugInfo *expr::Colon::debug() {
        return new DebugInfo{"Colon",{lhs->debug(),rhs->debug()}};
    }

    DebugInfo *Constructor::debug() {
        auto ret = new DebugInfo{"Constructor"};
        auto iter = parameter;
        while(iter){
            ret->childs.push_back(iter->debug());
            iter = iter->next_sibling;
        }
        return ret;
    }
}

