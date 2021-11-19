#include <iostream>
#include "nullSafe.h"
#include "token.h"
#include "lexer.h"
#include "logger.h"
#include "parser.h"
#include "semantic.h"
#include "context.h"
#include "codeGen.h"
using namespace evoBasic;
using namespace std;
int main() {
    Logger::debugMode = true;
    auto source = new FileSource("test");
    Lexer lexer(source);
    Parser parser(&lexer);
    auto ast = parser.parseGlobal();
    ast->debug(cout,"");

    auto context = make_shared<Context>();
    Semantic::collectSymbol(ast,context);
    cout<<context->getGlobal()->debug(0)<<endl;

    Semantic::collectDetail(ast,context);
    cout<<context->getGlobal()->debug(0)<<endl;

    Semantic::typeCheck(ast,context);
    cout<<context->getGlobal()->debug(0)<<endl;

    if(Logger::errorCount == 0){
        Semantic::solveByteLengthDependencies(context);

        IRGen gen;
        auto ir = gen.gen(ast,context);
        ir->toString(cout);
        fstream file("out.evo",ios::binary | ios::out);
        ir->toHex(file);
    }
    else{
        Logger::error("Build failed");
    }

    return 0;
}
