//
// Created by yorkin on 8/13/21.
//

#ifndef EVOVM_FORMATOR_H
#define EVOVM_FORMATOR_H
#include<sstream>
#include<string>

class Format{
public:
    std::stringstream stream;
    template<typename T>
    Format & operator<<(T t){
        stream<<t;
        return *this;
    }
    operator std::string(){
        return move(stream.str());
    }
};

#endif //EVOVM_FORMATOR_H
