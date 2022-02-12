#ifndef EVO_ANALYZER_HELPER
#define EVO_ANALYZER_HELPER
#include <string>
#include <parser/parseTree.h>

namespace evoBasic{

	std::string getID(parseTree::expr::ID *id);

    int getDigit(parseTree::expr::Digit *digit);

    std::string getString(parseTree::expr::String *str);

    bool is_name_valid(const std::string& name, Location *location, type::Domain *domain);

}
#endif
