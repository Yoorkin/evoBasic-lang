//
// Created by yorkin on 1/14/22.
//

#ifndef EVOBASIC_PROCESSOR_H
#define EVOBASIC_PROCESSOR_H
#include "runtime.h"
#include <type_traits>
#include <loader/bytecode.h>
#include <stack>
namespace evoBasic::vm{
    using PC = data::Byte*;
    class ExecutionEnv{
        Function *function;
        PC pc;
        Memory frame;
        Memory locals_frame;
    public:
        Function *getFunction(){
            return function;
        }

        Memory &getFrame(){
            return frame;
        }

        Memory &getLocalFrame(){
            return locals_frame;
        }

        explicit ExecutionEnv(Function *function,Memory frame);

        template<typename T>
        T consume(){
            T t = *((T*)pc);
            pc += sizeof(T);
            return t;
        }

        template<Bytecode>
        Bytecode consume(){
            auto code = (Bytecode)(*((data::Byte*)pc));
            pc += 1;
            return code;
        }

        void jump(data::u64 dst){
            pc = function->getBlock() + dst;
        }
    };

    class Processor {
        RuntimeContext *context = nullptr;
        std::stack<ExecutionEnv> execution_stack;
        ExecutionEnv &getCurrentEnv();
        Stack operand;
        Memory frame;
        data::u64 frame_top = 0;
    public:
        explicit Processor(RuntimeContext *context,Function *function,size_t operand_size,size_t frame_size);
        void run();

        void copyArgs(Function *function);

        void initLocals(ExecutionEnv &env);
    };

}

#endif //EVOBASIC_PROCESSOR_H
