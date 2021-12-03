//
// Created by yorkin on 11/14/21.
//

#ifndef EVOBASIC_UTILS_H
#define EVOBASIC_UTILS_H
#include <string>
#include <exception>
#include "ast.h"
#include "type.h"
namespace evoBasic{
    std::string getID(ast::expr::ID *id);

    int getDigit(ast::expr::Digit *digit);

    std::string getString(ast::expr::String *str);

    bool is_name_valid(const std::string& name, Location *location, type::Domain *domain);
}



#endif //EVOBASIC_UTILS_H
