//
// Created by yorkin on 11/14/21.
//

#include "utils.h"
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

}
