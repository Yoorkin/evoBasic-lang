//
// Created by yorkin on 8/13/21.
//

#ifndef EVOVM_FORMATOR_H
#define EVOVM_FORMATOR_H
#include<sstream>
#include<string>

class format{
public:
    std::stringstream stream;
    template<typename T>
    format & operator<<(T t){
        stream<<t;
        return *this;
    }
    operator std::string(){
        return move(stream.str());
    }
};

#endif //EVOVM_FORMATOR_H
