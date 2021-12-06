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
#include "token.h"

namespace evoBasic{
    class Position;
    class Token;
    class Logger {
        static std::ostream* out;
    public:
        Logger()=delete;
        static void redirect(std::ostream* stream);
        static bool debugMode;
        static int errorCount,warningCount;

        static void error(std::string message);
        static void warning(std::string message);
        static void dev(std::string message);

        static void code(const Location *location,bool is_error = false);
        static void error(const Location *location,const std::string& message);
        static void error(std::initializer_list<const Location*> locations,const std::string& message);
        static void warning(const Location *location,const std::string& message);
        static void panic(const std::list<std::pair<std::string,Location*>>& callstack,const Location *location,const std::string& message);
    };
}

#endif //EVOBASIC_LOGGER_H
