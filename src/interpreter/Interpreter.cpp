//
// Created by yorkin on 10/11/21.
//

#include "Interpreter.h"

#include <utility>
using namespace std;
namespace evoBasic{
    void Interpreter::execute(const evoBasic::SymbolTable &table) {
        if(!table.entrance){
            Logger::error("找不到入口函数'Sub Main'");
            return;
        }

        try{
            for(const auto& stmt : table.entrance->getImplCodeTree()->child){
                auto tag = stmt->tag;
                if(tag == Tag::Let){

                }
            }
        }
        catch (const RuntimeException& e){

        }
    }

    StackFrame::StackFrame(const std::shared_ptr<Type::UserFunction>& function) : function_parent_domain(function->getParent()){

    }

    std::shared_ptr<Type::Value> StackFrame::lookUp(const string &name) {
        auto tmp = local_variables.find(name);
        if(tmp!=local_variables.end()){
            return tmp->second;
        }
        else{
             function_parent_domain.lock()->lookUp(name);
        }
    }

    RuntimeException::RuntimeException(Position position, std::string message) : pos(position),msg(std::move(message)) {}

    const char *RuntimeException::what() const noexcept {
        return msg.c_str();
    }


}

