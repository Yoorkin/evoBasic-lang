//
// Created by yorkin on 12/18/21.
//

#include "ast.h"
namespace evoBasic::ast{
    Expression *Expression::error = new Expression;

    DebugInfo *Expression::debug() {
        return nullptr;
    }

    evoBasic::DebugInfo *Global::debug() {
        return nullptr;
    }

    DebugInfo *Class::debug() {
        return nullptr;
    }

    DebugInfo *Module::debug() {
        return nullptr;
    }

    DebugInfo *Variable::debug() {
        return nullptr;
    }

    DebugInfo *Function::debug() {
        return nullptr;
    }

    DebugInfo *Constructor::debug() {
        return nullptr;
    }

    DebugInfo *External::debug() {
        return nullptr;
    }

    DebugInfo *Interface::debug() {
        return nullptr;
    }

    DebugInfo *Enum::debug() {
        return nullptr;
    }

    DebugInfo *Type::debug() {
        return nullptr;
    }

    DebugInfo *Let::debug() {
        return nullptr;
    }

    DebugInfo *Select::debug() {
        return nullptr;
    }

    DebugInfo *Loop::debug() {
        return nullptr;
    }

    DebugInfo *If::debug() {
        return nullptr;
    }

    DebugInfo *For::debug() {
        return nullptr;
    }

    DebugInfo *Return::debug() {
        return nullptr;
    }

    DebugInfo *Exit::debug() {
        return nullptr;
    }

    DebugInfo *Continue::debug() {
        return nullptr;
    }

    DebugInfo *ExprStmt::debug() {
        return nullptr;
    }

    DebugInfo *Case::debug() {
        return nullptr;
    }

    DebugInfo *Unary::debug() {
        return nullptr;
    }

    DebugInfo *Binary::debug() {
        return nullptr;
    }

    DebugInfo *Cast::debug() {
        return nullptr;
    }

    DebugInfo *Assign::debug() {
        return nullptr;
    }

    DebugInfo *Argument::debug() {
        return nullptr;
    }

    DebugInfo *New::debug() {
        return nullptr;
    }

    DebugInfo *Parentheses::debug() {
        return nullptr;
    }

    DebugInfo *ArrayElement::debug() {
        return nullptr;
    }

    DebugInfo *RecordVector::debug() {
        return nullptr;
    }

    DebugInfo *FtnCall::debug() {
        return nullptr;
    }

    DebugInfo *VFtnCall::debug() {
        return nullptr;
    }

    DebugInfo *SFtnCall::debug() {
        return nullptr;
    }

    DebugInfo *Local::debug() {
        return nullptr;
    }

    DebugInfo *Arg::debug() {
        return nullptr;
    }

    DebugInfo *Fld::debug() {
        return nullptr;
    }

    DebugInfo *SFld::debug() {
        return nullptr;
    }

    DebugInfo *Digit::debug() {
        return nullptr;
    }

    DebugInfo *Decimal::debug() {
        return nullptr;
    }

    DebugInfo *String::debug() {
        return nullptr;
    }

    DebugInfo *Char::debug() {
        return nullptr;
    }

    DebugInfo *Boolean::debug() {
        return nullptr;
    }
};
