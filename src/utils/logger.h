//
// Created by yorkin on 7/15/21.
//

#ifndef EVOBASIC_LOGGER_H
#define EVOBASIC_LOGGER_H
#include <iostream>
#include <string>
#include <initializer_list>
#include <vector>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <list>
#include <memory>
#include <iostream>
#include <functional>
#include "token.h"

namespace evoBasic{
    class Position;
    class Token;

    enum class Channel{Tokens,ParseTrees,AbstractTrees,SymbolTable,IL,InheritTopologicalOrder,ByteLengthTopologicalOrder};

    class Logger {
        static std::ostream* out;
        static std::vector<bool> channels;
    public:
        using PrintAction = std::function<std::string()>;
        Logger()=delete;
        static void redirect(std::ostream* stream);
        static int errorCount,warningCount;

        static void error(std::string message);
        static void warning(std::string message);

        static void print(Channel channel,std::string message);
        static void lazy_print(Channel channel,PrintAction action);
        static void enable(Channel channel);
        static void disable(Channel channel);

        static void code(const Location *location,bool is_error = false);
        static void error(const Location *location,const std::string& message);
        static void error(std::initializer_list<const Location*> locations,const std::string& message);
        static void warning(const Location *location,const std::string& message);
        static void panic(const std::list<std::pair<std::string,Location*>>& callstack,const Location *location,const std::string& message);
    };


}

#endif //EVOBASIC_LOGGER_H
