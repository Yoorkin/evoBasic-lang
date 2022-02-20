#ifndef EVO_ANALYZER_HELPER
#define EVO_ANALYZER_HELPER
#include <string>
#include <parser/parseTree.h>
#include <utils/unicode.h>

namespace evoBasic{

	unicode::Utf8String getID(parseTree::expr::ID *id);

    int getDigit(parseTree::expr::Digit *digit);

    unicode::Utf8String getString(parseTree::expr::String *str);

    bool is_name_valid(unicode::Utf8String name, Location *location, type::Domain *domain);

}
#endif
