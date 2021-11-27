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

    class SymbolNotFound : std::exception{
    public:
        type::Symbol *search_domain = nullptr;
        Location *location = nullptr;
        std::string search_name;
        SymbolNotFound(Location *location,type::Symbol *search_domain,std::string search_name)
            : search_domain(search_domain),search_name(search_name),location(location){}
    };
}



#endif //EVOBASIC_UTILS_H
