//
// Created by yorkin on 11/14/21.
//

#include <algorithm>
#include "utils.h"
#include "logger.h"
#include "i18n.h"

using namespace std;
namespace evoBasic{
    string getID(parseTree::expr::ID *id) {
        NotNull(id);
        string ret = id->lexeme;
        transform(ret.begin(),ret.end(),ret.begin(),[](char c){
            return tolower(c);
        });
        return ret;
    }

    int getDigit(parseTree::expr::Digit *digit){
        NotNull(digit);
        return digit->value;
    }

    string getString(parseTree::expr::String *str){
        return str->value;
    }


    bool is_name_valid(const string& name,Location *location, type::Domain *domain){
        NotNull(location);
        if(domain->findInDomainOnly(name)){
            Logger::error(location,i18n::lang->msgNameConflicit());
            return false;
        }
        return true;
    }


}
