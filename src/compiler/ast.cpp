//
// Created by yorkin on 12/18/21.
//

#include "ast.h"
#include <string>
#include <vector>
namespace evoBasic::ast{
    using namespace std;

    Expression *Expression::error = new Expression;

    void debugTree(ostream &stream,DebugInfo *info,string prefix,bool is_last){
        const string indent_unit = "\t";
        stream<<prefix;
        if(is_last)stream<<"└─";else stream<<"├─";
        stream<<info->text<<endl;
        prefix += is_last ? " " : "│";
        auto pos = info->text.size()/2+1;
        if(pos>5)pos = 5;
        for(int i=0;i<pos;i++)prefix+=' ';

        for(auto iter:info->childs){
            debugTree(stream,iter,prefix,(iter == info->childs.back()));
        }
    }

    string debugAST(ast::Node *ast){
        stringstream stream;
        auto info = ast->debug();
        debugTree(stream,info,"",true);
        delete info;
        return stream.str();
    }


    DebugInfo *Expression::debug() {
        return new DebugInfo{"<Error>"};
    }

    evoBasic::DebugInfo *Global::debug() {
        auto ret = new DebugInfo{"<Global>"};
        FOR_EACH(iter,member){
            ret->childs.push_back(iter->debug());
        }
        return ret;
    }

    DebugInfo *Class::debug() {
        auto ret = new DebugInfo{Format() << "<Class> " << class_symbol->getName()};
        FOR_EACH(iter,member){
            ret->childs.push_back(iter->debug());
        }
        return ret;
    }

    DebugInfo *Module::debug() {
        auto ret = new DebugInfo{Format() << "<Module> " << module_symbol->getName()};
        FOR_EACH(iter,member){
            ret->childs.push_back(iter->debug());
        }
        return ret;
    }

    DebugInfo *Variable::debug() {
        auto ret = new DebugInfo{Format() << "<Variable> " << variable_symbol->getName()};
        if(this->initial)ret->childs.push_back(initial->debug());
        return ret;
    }

    DebugInfo *Function::debug() {
        auto ret = new DebugInfo{Format() << "<Function> " << function_symbol->getName()};
        for(auto &option : function_symbol->getArgsOptions()){
            ret->childs.push_back(
                    new DebugInfo{Format() << "<Optional> " << option->getName(),{option->getInitial()->debug()}}
            );
        }
        FOR_EACH(iter,statement){
            ret->childs.push_back(iter->debug());
        }
        return ret;
    }

    DebugInfo *Constructor::debug() {
        auto ret = new DebugInfo{Format() << "<Constructor> " << constructor_symbol->getName()};
        FOR_EACH(iter,statement){
            ret->childs.push_back(iter->debug());
        }
        return ret;
    }

    DebugInfo *External::debug() {
        return new DebugInfo{Format() << "<External> " << function_symbol->getName()};
    }

    DebugInfo *Interface::debug() {
        return new DebugInfo{Format() << "<Interface> " << interface_symbol->getName()};
    }

    DebugInfo *Enum::debug() {
        return new DebugInfo{Format() << "<Enum> " << enum_symbol->getName()};
    }

    DebugInfo *Type::debug() {
        return new DebugInfo{Format() << "<Type> " << type_symbol->getName()};
    }

    DebugInfo *Let::debug() {
        auto ret = new DebugInfo{"<Let>"};
        FOR_EACH(iter,variable){
            ret->childs.push_back(iter->debug());
        }
        return ret;
    }

    DebugInfo *Select::debug() {
        auto ret = new DebugInfo{"<Select>",{condition->debug()}};
        FOR_EACH(iter,case_){
            ret->childs.push_back(iter->debug());
        }
        return ret;
    }

    DebugInfo *Loop::debug() {
        auto ret = new DebugInfo{"<Loop>",{condition->debug()}};
        FOR_EACH(iter,statement){
            ret->childs.push_back(iter->debug());
        }
        return ret;
    }

    DebugInfo *If::debug() {
        auto ret = new DebugInfo{"<If>"};
        FOR_EACH(iter,case_){
            ret->childs.push_back(iter->debug());
        }
        return ret;
    }

    DebugInfo *For::debug() {
        auto ret = new DebugInfo{"<For>",{
            iterator->debug(),
            begin->debug(),
            end->debug(),
            step->debug()
        }};
        FOR_EACH(iter,statement){
            ret->childs.push_back(iter->debug());
        }
        return ret;
    }

    DebugInfo *Return::debug() {
        return new DebugInfo{"<Return>",{expr->debug()}};
    }

    DebugInfo *Exit::debug() {
        vector<string> flag = {"For","While","Sub"};
        return new DebugInfo{Format() << "<Exit> " << flag[exit_flag]};
    }

    DebugInfo *Continue::debug() {
        return new DebugInfo{"<Continue>"};
    }

    DebugInfo *ExprStmt::debug() {
        return new DebugInfo{"<ExprStmt>",{expr->debug()}};
    }

    DebugInfo *Case::debug() {
        auto ret = new DebugInfo{"<Case>"};
        if(condition)ret->childs.push_back(condition->debug());
        auto iter = statement;
        while(iter){
            ret->childs.push_back(iter->debug());
            iter = iter->next_sibling;
        }
        return ret;
    }

    DebugInfo *Unary::debug() {
        vector<string> OpStr = {"Empty","'-'","'+'"};
        return new DebugInfo{Format() << "Unary " << OpStr[op], {terminal->debug()}};
    }

    DebugInfo *Binary::debug() {
        vector<string> OpStr = {"Empty","And","Or","Xor","Not","'=='","'<>'","'>='","'<='","'>='","'<'",
                                "'+'","'-'","'*'","'/'","'\\'","'='","'.'","Index"};
        return new DebugInfo{OpStr[op],{lhs->debug(),rhs->debug()}};
    }

    DebugInfo *Cast::debug() {
        return new DebugInfo{Format() << "<Cast> " << target->getName(), {expr->debug()}};
    }

    DebugInfo *Assign::debug() {
        return new DebugInfo{"'='",{lhs->debug(),rhs->debug()}};
    }

    DebugInfo *Argument::debug() {
        return new DebugInfo{Format() << "<Argument>", {expr->debug()}};
    }

    DebugInfo *New::debug() {
        auto ret = new DebugInfo{Format() << "<New> " << target->mangling('.')};
        auto iter = argument;
        while(iter){
            ret->childs.push_back(iter->debug());
            iter = iter->next_sibling;
        }
        return ret;
    }

    DebugInfo *Parentheses::debug() {
        return new DebugInfo{"<Parentheses>",{expr->debug()}};
    }

    DebugInfo *ArrayElement::debug() {
        return new DebugInfo{"<Array-Element> ",{
            array->debug(),
            offset->debug()
        }};
    }


    DebugInfo *FtnCall::debug() {
        auto ret = new DebugInfo{Format() << "<FtnCall> " << function->mangling('.'), {ref->debug()}};
        FOR_EACH(iter,argument){
            ret->childs.push_back(iter->debug());
        }
        return ret;
    }

    DebugInfo *VFtnCall::debug() {
        auto ret = new DebugInfo{Format() << "<VFtnCall> " << function->mangling('.'), {ref->debug()}};
        FOR_EACH(iter,argument){
            ret->childs.push_back(iter->debug());
        }
        return ret;
    }

    DebugInfo *SFtnCall::debug() {
        auto ret = new DebugInfo{Format() << "<SFtnCall> " << function->mangling('.')};
        FOR_EACH(iter,argument){
            ret->childs.push_back(iter->debug());
        }
        return ret;
    }

    DebugInfo *ExtCall::debug() {
        auto ret = new DebugInfo{Format() << "<ExtCall> " << function->mangling('.')};
        FOR_EACH(iter,argument){
            ret->childs.push_back(iter->debug());
        }
        return ret;
    }

    DebugInfo *Local::debug() {
        return new DebugInfo{Format() << "<Local> " << variable->mangling('.') << '@' << variable->getLayoutIndex()};
    }

    DebugInfo *Arg::debug() {
        return new DebugInfo{Format() << "<ArgUse> " << variable->mangling('.') << '@' << variable->getLayoutIndex()};
    }

    DebugInfo *Fld::debug() {
        return new DebugInfo{Format() << "<Fld> " << variable->mangling('.'), {ref->debug()}};
    }

    DebugInfo *SFld::debug() {
        return new DebugInfo{Format() << "<SFld> " << variable->mangling('.')};
    }

    DebugInfo *Digit::debug() {
        return new DebugInfo{Format() << "<Digit> " << value};
    }

    DebugInfo *Decimal::debug() {
        return new DebugInfo{Format() << "<Decimal> " << value};
    }

    DebugInfo *String::debug() {
        return new DebugInfo{Format() << "<String> " << value << "\""};
    }

    DebugInfo *Char::debug() {
        return new DebugInfo{Format() << "<Char> '" << value << "'"};
    }

    DebugInfo *Boolean::debug() {
        return new DebugInfo{Format() << "<Boolean> " << (value ? "true" : "false")};
    }

    DebugInfo *Dim::debug() {
        auto ret = new DebugInfo{"<Dim>"};
        FOR_EACH(iter,variable){
            ret->childs.push_back(iter->debug());
        }
        return ret;
    }

    DebugInfo *Delegate::debug() {
        auto ret = new DebugInfo{Format() << "<Delegate> " << target->mangling('.')};
        if(ref)ret->childs.push_back(ref->debug());
        return ret;
    }
};
