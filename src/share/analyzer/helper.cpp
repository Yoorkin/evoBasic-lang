#include "helper.h"
#include "type.h"

#include <utils/logger.h>
#include <utils/i18n.h>
#include <utils/nullSafe.h>

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

