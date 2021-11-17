//
// Created by yorkin on 11/14/21.
//

#ifndef EVOBASIC_UTILS_H
#define EVOBASIC_UTILS_H
#include <string>
#include "ast.h"
namespace evoBasic{
    std::string getID(ast::expr::ID *id);

    int getDigit(ast::expr::Digit *digit);

    std::string getString(ast::expr::String *str);

// helper type for the visitor
    template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
// explicit deduction guide (not needed as of C++20)
    template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;
}



#endif //EVOBASIC_UTILS_H
