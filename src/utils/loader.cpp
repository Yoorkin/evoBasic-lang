//
// Created by yorkin on 1/7/22.
//

#include "loader.h"
#include "logger.h"
#include "il.h"
#include <filesystem>

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

            fstream package_file(path,ios::binary | ios::in);
            fs::path package_path(path);
            auto package = new il::Document(package_path.stem(),package_file);
            auto sub_dependencies = package->getDependenciesPath();
            waiting_package.insert(waiting_package.end(),sub_dependencies.begin(),sub_dependencies.end());

            packages.push_back(package);
            Logger::lazy_print(Channel::IL, [&](){
                return package->toString();
            });
        }
    }

    void Loader::loadPackagesSymbols(Context *context) {
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
}