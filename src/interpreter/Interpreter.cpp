//
// Created by yorkin on 10/11/21.
//

#include "Interpreter.h"
#include "../frontend/Semantic.h"
#include <utility>
using namespace std;
namespace evoBasic{
    void Interpreter::execute(){
        if(!entrance){
            Logger::error("找不到入口函数'Sub Main'");
            return;
        }

        try{
            call(entrance,Position::Empty);
        }
        catch (const RuntimeException& e){
            list<pair<string,Position>> callstack_info;
            while(!call_stack.empty()){
                auto call = call_stack.top();
                call_stack.pop();
                callstack_info.emplace_back(call.getFunction()->getName(),call.getReturnPosition());
            }
            Logger::panic(callstack_info,e.getPosition(),e.what());
        }
    }



    std::shared_ptr<Type::Value> Interpreter::call(const std::shared_ptr<Type::Function> function,Position previous) {
        auto user = dynamic_pointer_cast<Type::UserFunction>(function);
        auto external = dynamic_pointer_cast<Type::ExternalFunction>(function);

        if(user){
            call_stack.push(StackFrame(user,previous));

            for(const auto& stmt : entrance->getImplCodeTree()->child){
                auto tag = stmt->tag;
                if(tag == Tag::Let && !call_stack.empty()){

                    for(const auto& var : stmt->child){
                        auto name = var->child[0]->get<string>(Attr::Lexeme);
                        auto tup = SymbolTable::visitAnnotation(current_domain,var->child[1]);
                        auto prototype = dynamic_pointer_cast<Type::Prototype>(get<0>(tup));
                        if(!prototype)throw RuntimeException(get<1>(tup),Format()<<"'"<<name<<"'初始化失败");
                        shared_ptr<Type::Value> ptr;
                        if(prototype->getKind() == Type::DeclarationEnum::Primitive){
                            // primitive type. integer,boolean,long,Variant etc.
                            ptr = prototype->create();
                        }
                        else{
                            // ref type
                            ptr = make_shared<Type::Ref>();
                        }
                        call_stack.top().add(name,ptr);
                        if(var->child[2]->tag != Tag::Empty){
                            auto initial = calculateExpression(var->child[2]);
                        }
                    }
                    //call_stack.top().add();
                }
            }

            call_stack.pop();

            if(user->getRetSignature()){
                throw RuntimeException(previous,Format()<<"函数'"<<user->getName()<<"'未返回值");
            }
            else{ // void
                return make_shared<Type::VoidValue>();
            }

        }
    }



    StackFrame::StackFrame(const std::shared_ptr<Type::UserFunction>& function,Position previous)
        : function_parent_domain(function->getParent()),current_function(function),prv_pos(previous){

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

    std::shared_ptr<Type::UserFunction> StackFrame::getFunction() {
        return current_function;
    }

    const Position &StackFrame::getReturnPosition() {
        return prv_pos;
    }

    void StackFrame::add(std::string name,std::shared_ptr<Type::Value> variable) {
        local_variables.insert({std::move(name),std::move(variable)});
    }

    RuntimeException::RuntimeException(Position position, std::string message) : pos(std::move(position)),msg(std::move(message)) {}

    const char *RuntimeException::what() const noexcept {
        return msg.c_str();
    }


    Interpreter::Interpreter(const SymbolTable& table) : current_domain(table.global),entrance(table.entrance){

    }

    std::shared_ptr<Type::Value> Interpreter::calculateExpression(std::shared_ptr<Node> expression) {
        throw "unimplement";
        
    }


}

