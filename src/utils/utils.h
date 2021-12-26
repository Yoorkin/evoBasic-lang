//
// Created by yorkin on 11/14/21.
//

#ifndef EVOBASIC_UTILS_H
#define EVOBASIC_UTILS_H
#include <string>
#include <exception>
#include "parseTree.h"
#include "type.h"
namespace evoBasic{
    std::string getID(parseTree::expr::ID *id);

    int getDigit(parseTree::expr::Digit *digit);

    std::string getString(parseTree::expr::String *str);

    bool is_name_valid(const std::string& name, Location *location, type::Domain *domain);


    struct DebugInfo{
        std::string text;
        std::list<DebugInfo*> childs;

        ~DebugInfo(){
            for(auto child:childs)delete child;
        }
    };
}



#endif //EVOBASIC_UTILS_H
