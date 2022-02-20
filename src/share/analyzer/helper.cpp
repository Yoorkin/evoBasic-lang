#include "helper.h"
#include "type.h"

#include <utils/logger.h>
#include <utils/i18n.h>
#include <utils/nullSafe.h>

using namespace std;
using namespace evoBasic::unicode;
namespace evoBasic{

	Utf8String getID(parseTree::expr::ID *id) {
        NotNull(id);
        Utf8String ret;
        for(auto c : id->lexeme){
            ret.push_back(toLowerCase(c));
        }
        return ret;
    }

    int getDigit(parseTree::expr::Digit *digit){
        NotNull(digit);
        return digit->value;
    }

    unicode::Utf8String getString(parseTree::expr::String *str){
        return str->value;
    }


    bool is_name_valid(unicode::Utf8String name, Location *location, type::Domain *domain){
        NotNull(location);
        if(domain->findInDomainOnly(name)){
            Logger::error(location,i18n::lang->msgNameConflicit());
            return false;
        }
        return true;
    }

}

