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
#include <utils/unicode.h>

namespace evoBasic{
    class Position;
    class Token;
	class Location;

    enum class Channel{
        Tokens,ParseTrees,AbstractTrees,SymbolTable,
        IL,InheritTopologicalOrder,ByteLengthTopologicalOrder,
        OperandStack};

    class Logger {
        static std::ostream* out;
        static std::vector<bool> channels;
    public:
        using PrintAction = std::function<unicode::Utf8String()>;
        Logger()=delete;
        static void redirect(std::ostream* stream);
        static int errorCount,warningCount;

        static void error(unicode::Utf8String message);
        static void warning(unicode::Utf8String message);

        static void print(Channel channel,unicode::Utf8String message);
        static void lazy_print(Channel channel,PrintAction action);
        static void enable(Channel channel);
        static void disable(Channel channel);

        static void code(const Location *location,bool is_error = false);
        static void error(const Location *location,const unicode::Utf8String& message);
        static void error(std::initializer_list<const Location*> locations,const unicode::Utf8String& message);
        static void warning(const Location *location,const unicode::Utf8String& message);
        static void panic(const std::list<std::pair<unicode::Utf8String,Location*>>& callstack,const Location *location,const unicode::Utf8String& message);
    };


}

#endif //EVOBASIC_LOGGER_H
