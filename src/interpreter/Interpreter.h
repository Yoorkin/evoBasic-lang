//
// Created by yorkin on 10/11/21.
//

#ifndef EVOBASIC_INTERPRETER_H
#define EVOBASIC_INTERPRETER_H
#include<memory>
#include<stack>
#include<list>
#include<string>
#include<exception>
#include "../frontend/Type.h"
#include "../frontend/Semantic.h"

namespace evoBasic{
    class StackFrame{
        std::weak_ptr<Type::Domain> function_parent_domain;
        std::map<std::string,std::shared_ptr<Type::Variable>> local_variables;
    public:
        explicit StackFrame(const std::shared_ptr<Type::UserFunction>& function);
        std::shared_ptr<Type::Value> lookUp(const std::string& name);
        void add(std::string name,std::shared_ptr<Type::Variable> variable);
    };

    class RuntimeException : std::exception{
        Position pos;
        std::string msg;
    public:
        RuntimeException(Position position,std::string message);
        [[nodiscard]] const char * what() const noexcept override;
    };

    class Interpreter {
        std::stack<StackFrame> call_stack;
    public:
        void execute(const SymbolTable& table);
        std::shared_ptr<Type::Value> call(const std::shared_ptr<Type::Function> function);

    };



}


#endif //EVOBASIC_INTERPRETER_H
