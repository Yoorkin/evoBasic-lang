//
// Created by yorkin on 1/7/22.
//

#include "loader.h"
#include "il.h"
#include <utils/logger.h>
#include <filesystem>
#include <utils/config.h>

namespace fs = std::filesystem;
using namespace std;
namespace evoBasic{

    void Loader::addToWaitingDeque(std::string package_path) {
        waiting_package.push_back(package_path);
    }

    void Loader::loadPackages() {
        while(!waiting_package.empty()){
            auto path = waiting_package.front();
            waiting_package.pop_front();

            if(!fs::exists(path)){
                Logger::error(Format()<<"missing package: '"<<path<<"' not found");
            }
            else{
                fstream package_file(path,ios::binary | ios::in);
                if(!package_file)PANIC;

                fs::path package_path(path);
                auto package = new il::Document(package_path.stem().string(), package_file);
                auto sub_dependencies = package->getDependenciesPath();
                for(auto sub_dep : sub_dependencies){
                    waiting_package.push_back(sub_dep + extensions::package);
                }
                packages.push_back(package);
                Logger::lazy_print(Channel::IL, [&](){
                    return package->toString();
                });
                package_file.close();
            }
        }
    }

    void Loader::loadPackagesSymbols(CompileTimeContext *context) {
        for(auto package : packages){
            package->pushSymbolsInto(context);
        }

        for(auto package : packages){
            package->fillSymbolsDetail(context);
        }
    }

    std::list<il::Document*> &Loader::getPackages() {
        return packages;
    }

    Loader::~Loader() {
        for(auto package : packages){
            delete package;
        }
    }


}