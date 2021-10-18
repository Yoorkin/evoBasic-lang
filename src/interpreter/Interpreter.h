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
        std::shared_ptr<Type::UserFunction> current_function;
        std::map<std::string,std::shared_ptr<Type::Value>> local_variables;
        Position prv_pos;
    public:
        explicit StackFrame(const std::shared_ptr<Type::UserFunction>& function,Position previous);
        std::shared_ptr<Type::Value> lookUp(const std::string& name);
        void add(std::string name,std::shared_ptr<Type::Value> variable);
        std::shared_ptr<Type::UserFunction> getFunction();
        const Position& getReturnPosition();
    };

    class RuntimeException : std::exception{
        Position pos;
        std::string msg;
    public:
        RuntimeException(Position position,std::string message);
        [[nodiscard]] const char * what() const noexcept override;
        const Position& getPosition()const {return pos;}
    };

    class Interpreter {
        std::stack<StackFrame> call_stack;
        std::shared_ptr<Type::Domain> current_domain;
        std::shared_ptr<Type::UserFunction> entrance;
    public:
        explicit Interpreter(const SymbolTable& table);
        void execute();
        std::shared_ptr<Type::Value> call(std::shared_ptr<Type::Function> function,Position previous);
        std::shared_ptr<Type::Value> calculateExpression(std::shared_ptr<Node> expression);
    };



}


#endif //EVOBASIC_INTERPRETER_H
