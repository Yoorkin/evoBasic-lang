//
// Created by yorkin on 1/7/22.
//

#ifndef EVOBASIC_LOADER_H
#define EVOBASIC_LOADER_H
#include <list>
#include <string>
#include <utils/unicode.h>

namespace evoBasic{

    namespace il{
        class Document;
    }

    class CompileTimeContext;

    class Loader {
        std::list<il::Document*> packages;
        std::list<unicode::Utf8String> waiting_package;
    public:
        void addToWaitingDeque(std::string package_path);
        void loadPackages();
        void loadPackagesSymbols(CompileTimeContext *context);
        std::list<il::Document*> &getPackages();
        ~Loader();
    };

}

#endif //EVOBASIC_LOADER_H
