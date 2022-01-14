//
// Created by yorkin on 11/7/21.
//

#ifndef EVOBASIC2_INVOKE_H
#define EVOBASIC2_INVOKE_H
// #include<ffi.h>
#include<string>
#include<vector>
#include"stack.h"
#include"utils/data.h"
#include"loader/bytecode.h"

namespace evoBasic::vm{

    class Library;

    class Invoke {
    public:
        static Library *loadLibrary(std::string lib);
    };

    class ForeignFunction{
        std::vector<vm::Data> args_type;
        vm::Data ret_type;
    public:
        virtual void invoke(Stack *operand);
    };

    class Library{
    public:
        virtual ForeignFunction *loadFunction(std::string name,std::vector<vm::Data> args_type,vm::Data ret_type);
    };

}


#endif //EVOBASIC2_INVOKE_H
