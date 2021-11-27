//
// Created by yorkin on 11/14/21.
//

#include "utils.h"
#include "logger.h"

using namespace std;
namespace evoBasic{
    string getID(ast::expr::ID *id) {
        NotNull(id);
        string ret = id->lexeme;
        transform(ret.begin(),ret.end(),ret.begin(),[](char c){
            return tolower(c);
        });
        return ret;
    }

    int getDigit(ast::expr::Digit *digit){
        NotNull(digit);
        return digit->value;
    }

    string getString(ast::expr::String *str){
        return str->value;
    }


    bool is_name_valid(const string& name,Location *location, type::Domain *domain){
        NotNull(location);
        if(domain->findInDomainOnly(name)){
            Logger::error(location,"Naming conflict in current scope");
            return false;
        }
        return true;
    }


}
