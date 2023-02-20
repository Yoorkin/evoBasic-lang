//
// Created by yorkin on 8/13/21.
//

#ifndef EVOVM_FORMATOR_H
#define EVOVM_FORMATOR_H
#include <sstream>
#include "unicode.h"

class Format{
public:
    std::stringstream stream;
    template<typename T>
    Format & operator<<(T t){
        stream<<t;
        return *this;
    }
    operator evoBasic::unicode::Utf8String(){
        return evoBasic::unicode::Utf8String(stream.str().c_str());
    }
};

#endif //EVOVM_FORMATOR_H
