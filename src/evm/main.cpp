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
#include "memory.h"
#include <loader/bytecode.h>
#include <utils/config.h>
#include <loader/loader.h>
#include <loader/il.h>
#include "utils/data.h"
#include "runtime.h"
#include "processor.h"

using namespace std;
using namespace evoBasic;
using namespace evoBasic::vm;
namespace fs = std::filesystem;

void printHelp(){
    //todo: print help message
}


Loader *loader = nullptr;

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


int main(int argc,char *argv[]){
    loader = new Loader;

    CmdDistributor distributor;
    distributor.others(loadTarget);
    for(int i=1;i<argc;i++){
        distributor.distribute(argv[i]);
    }

    loader->loadPackages();

    RuntimeContext context(loader->getPackages());

    if(loader->getPackages().empty()){
        Logger::error("nothing to run.");
        printHelp();
    }
    else{
        loader->loadPackages();
        auto entrance = dynamic_cast<Function*>(context.find("main"));
        if(!entrance){
            Logger::error("Sub Main not found.");
        }
        else{
            Processor processor(&context,entrance,1024,2048);
            processor.run();
        }
    }

    delete loader;
}

