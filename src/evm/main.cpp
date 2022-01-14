//
// Created by yorkin on 11/19/21.
//
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <fstream>
#include <filesystem>
#include <functional>
#include <map>
#include <stack>
#include <utils/cmd.h>
#include <utils/logger.h>
#include <utils/format.h>
#include "stack.h"
#include <loader/bytecode.h>
#include <utils/config.h>
#include <loader/loader.h>
#include <loader/il.h>
#include "operation.h"
#include "utils/data.h"

using namespace std;
using namespace evoBasic;
namespace fs = std::filesystem;

void printHelp(){
    //todo: print help message
}

using ExecutionInfo = std::pair<il::FunctionDefine*,PC>;

Loader *loader = nullptr;
vm::Stack *operand = nullptr;
vm::Stack *frame = nullptr;

void loadTarget(string path){
    if(fs::exists(path)){
        fs::path package_path(path);
        if(package_path.extension() == evoBasic::extensions::package){
            loader->addToWaitingDeque(path);
        }
        else Logger::error(Format() << "'" << path << "' is not a .bkg file");
    }
    else Logger::error(Format() << "cannot find file '" << path << "'");
}

void runProgram(il::SFtn *entrance){
    std::stack<ExecutionInfo> execution_stack;
    PC pc = entrance->getBlocksMemory();
    il::FunctionDefine *current = entrance;
    while(true){
        switch ((Bytecode)*pc) {
            case Bytecode::Nop:
                pc+=1;
                break;
            case Bytecode::Ret:
                pc+=1;
                current = execution_stack.top().first;
                pc = execution_stack.top().second;
                execution_stack.pop();
                break;
            case Bytecode::Call:
            case Bytecode::CallVirt:
            case Bytecode::Callstatic:
                pc+=1;
                execution_stack.push({current,pc});
                current = operand->pop<il::FunctionDefine*>();
                pc = current->getBlocksMemory();
//                for(auto local : current->getLocals()){
//                    local->
//                }
                break;
            case Bytecode::Ldnull:
                pc+=1;
                break;
            case Bytecode::And:
                pc+=1;
                operand->push<data::boolean>(operand->pop<data::boolean>() && operand->pop<data::boolean>());
                break;
            case Bytecode::Or:
                operand->push<data::boolean>(operand->pop<data::boolean>() || operand->pop<data::boolean>());
                pc+=1;
                break;
            case Bytecode::Xor:
                operand->push<data::boolean>(operand->pop<data::boolean>() xor operand->pop<data::boolean>()); 
                pc+=1;
                break;
            case Bytecode::Ldloca:
                pc+=1;
                break;
            case Bytecode::Ldarga:
                pc+=1;
                break;
            case Bytecode::Ldelema:
                pc+=1;
                break;
            case Bytecode::Not:
				operand->push<data::boolean>(!operand->pop<data::boolean>());
                pc+=1;
                break;
            case Bytecode::Ldvftn:
                PANIC;
            case Bytecode::Ldsftn:
            case Bytecode::Ldftn:{
                pc+=2;
                auto target_id = *((il::TokenDef::ID*)pc);
				pc+=sizeof(il::TokenDef::ID);
                auto ftn = (il::FunctionDefine*)current->getDocument()->findTokenDef(target_id)->getTarget();
			    operand->push<il::FunctionDefine*>(ftn);
                break;
            }
            case Bytecode::Ldc:
                pc+=1;
                PANIC;
                break;
            case Bytecode::Newobj:
                break;
            case Bytecode::Invoke:
                break;
            case Bytecode::Ldflda:
                break;
            case Bytecode::Ldsflda:
                break;
            case Bytecode::Ldelem:
                break;
            case Bytecode::Stelem:
                break;
            case Bytecode::Stelema:
                break;
            case Bytecode::Ldarg:
                break;
            case Bytecode::Starg:
                break;
            case Bytecode::Ldloc:
                break;
            case Bytecode::Stloc:
                break;
            case Bytecode::Add:
                forEachType<OpAdd>(operand, pc);
                break;
            case Bytecode::Sub:
                forEachType<OpSub>(operand, pc);
                break;
            case Bytecode::Mul:
                forEachType<OpMul>(operand, pc);
                break;
            case Bytecode::Div:
                forEachType<OpDiv>(operand, pc);
                break;
            case Bytecode::FDiv:
                forEachType<OpFDiv>(operand, pc);
                break;
            case Bytecode::EQ:
                forEachType<OpEQ>(operand, pc);
                break;
            case Bytecode::NE:
                forEachType<OpNE>(operand, pc);
                break;
            case Bytecode::LT:
                forEachType<OpLT>(operand, pc);
                break;
            case Bytecode::GT:
                forEachType<OpGT>(operand, pc);
                break;
            case Bytecode::LE:
                forEachType<OpLE>(operand, pc);
                break;
            case Bytecode::GE:
                forEachType<OpGE>(operand, pc);
                break;
            case Bytecode::Neg:
                forEachType<OpNeg>(operand, pc);
                break;
            case Bytecode::Pop:
                forEachType<OpPop>(operand, pc);
                break;
            case Bytecode::Dup:
                forEachType<OpDup>(operand, pc);
                break;
            case Bytecode::Ldfld:
                break;
            case Bytecode::Ldsfld:
                break;
            case Bytecode::Stfld:
                break;
            case Bytecode::Stsfld:
                break;
            case Bytecode::Jif:
                break;
            case Bytecode::Br:
                break;
            case Bytecode::Push:
                break;
            case Bytecode::CastCls:
                break;
            case Bytecode::Conv:
                break;
        }
    }
}


int main(int argc,char *argv[]){
    loader = new Loader;

    CmdDistributor distributor;
    distributor.others(loadTarget);
    for(int i=1;i<argc;i++){
        distributor.distribute(argv[i]);
    }

    operand = new vm::Stack(1024);
    frame = new vm::Stack(2048);

    if(loader->getPackages().empty()){
        Logger::error("nothing to run.");
        printHelp();
    }
    else{
        loader->loadPackages();
        auto entrance = loader->getPackages().front()->getEntrance();
        if(!entrance){
            Logger::error("Sub Main not found.");
        }
        else{
            runProgram(entrance);
        }
    }

    delete loader;
}

