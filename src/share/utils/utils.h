//
// Created by yorkin on 11/14/21.
//

#ifndef EVOBASIC_UTILS_H
#define EVOBASIC_UTILS_H
#include <string>
#include <exception>
#include <list>
#include "unicode.h"
namespace evoBasic{

    struct DebugInfo{
        unicode::Utf8String text;
        std::list<DebugInfo*> childs;
        void add(DebugInfo *info){
            childs.push_back(info);
        }
        void add(unicode::Utf8String text){
            childs.push_back(new DebugInfo{text});
        }
        ~DebugInfo(){
            for(auto child:childs)delete child;
        }
    };
}



#endif //EVOBASIC_UTILS_H
