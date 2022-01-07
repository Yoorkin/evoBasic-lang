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
#include "cmd.h"
#include "logger.h"
#include "format.h"
#include "stack.h"
#include "bytecode.h"
#include "config.h"
#include "loader.h"
#include "il.h"
#include "operation.h"

using namespace std;
using namespace evoBasic;
namespace fs = std::filesystem;

void printHelp(){
    //todo: print help message
}

using ExecutionInfo = std::pair<il::FunctionDefine*,PC>;

using FtnRef = il::FunctionDefine*;


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
    while(true){
        switch ((Bytecode)*pc) {
            case Bytecode::Nop:
                pc+=1;
                break;
            case Bytecode::Ret:
                pc+=1;
                break;
            case Bytecode::CallVirt:
                pc+=1;
                break;
            case Bytecode::Callstatic:
                pc+=1;
                break;
            case Bytecode::Call:
                pc+=1;
                break;
            case Bytecode::Ldnull:
                pc+=1;
                break;
            case Bytecode::And:
                pc+=1;
                break;
            case Bytecode::Or:
                pc+=1;
                break;
            case Bytecode::Xor:
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
                pc+=1;
                break;
            case Bytecode::Ldftn:
                pc+=1;
                break;
            case Bytecode::Ldsftn:
                pc+=1;
                break;
            case Bytecode::Ldvftn:
                pc+=1;
                break;
            case Bytecode::Ldc:
                pc+=1;
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
                forEachType<OpAdd>(*(pc + 1), operand, pc);
                break;
            case Bytecode::Sub:
                break;
            case Bytecode::Mul:
                break;
            case Bytecode::Div:
                break;
            case Bytecode::FDiv:
                break;
            case Bytecode::EQ:
                break;
            case Bytecode::NE:
                break;
            case Bytecode::LT:
                break;
            case Bytecode::GT:
                break;
            case Bytecode::LE:
                break;
            case Bytecode::GE:
                break;
            case Bytecode::Neg:
                break;
            case Bytecode::Pop:
                break;
            case Bytecode::Dup:
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

            case Bytecode::i8:
                break;
            case Bytecode::i16:
                break;
            case Bytecode::i32:
                break;
            case Bytecode::i64:
                break;
            case Bytecode::u8:
                break;
            case Bytecode::u16:
                break;
            case Bytecode::u32:
                break;
            case Bytecode::u64:
                break;
            case Bytecode::f32:
                break;
            case Bytecode::f64:
                break;
            case Bytecode::ref:
                break;
            case Bytecode::ftn:
                break;
            case Bytecode::vftn:
                break;
            case Bytecode::sftn:
                break;
            case Bytecode::record:
                break;
            case Bytecode::array:
                break;
            case Bytecode::boolean:
                break;
            case Bytecode::character:
                break;
            case Bytecode::delegate:
                break;

            case Bytecode::EndMark:
            case Bytecode::DependDef:
            case Bytecode::TextTokenDef:
            case Bytecode::ConstructedDef:
            case Bytecode::TokenRef:
            case Bytecode::PtdAcsDef:
            case Bytecode::PriAcsDef:
            case Bytecode::PubAcsDef:
            case Bytecode::ExtendDef:
            case Bytecode::ImplDef:
            case Bytecode::LibDef:
            case Bytecode::ExtAliasDef:
            case Bytecode::DocumentDef:
            case Bytecode::ClassDef:
            case Bytecode::ModuleDef:
            case Bytecode::InterfaceDef:
            case Bytecode::EnumDef:
            case Bytecode::RecordDef:
            case Bytecode::FtnDef:
            case Bytecode::VFtnDef:
            case Bytecode::SFtnDef:
            case Bytecode::CtorDef:
            case Bytecode::ExtDef:
            case Bytecode::ItfFtnDef:
            case Bytecode::FldDef:
            case Bytecode::SFldDef:
            case Bytecode::PairDef:
            case Bytecode::RegDef:
            case Bytecode::OptDef:
            case Bytecode::InfDef:
            case Bytecode::LocalDef:
            case Bytecode::ResultDef:
            case Bytecode::Byref:
            case Bytecode::Byval:
            case Bytecode::InstBeg:
                PANIC;
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
    operand = new vm::Stack(2048);

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

