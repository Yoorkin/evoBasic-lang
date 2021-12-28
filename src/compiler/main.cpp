#include <iostream>
#include <filesystem>
#include "nullSafe.h"
#include "token.h"
#include "lexer.h"
#include "logger.h"
#include "parser.h"
#include "semantic/semantic.h"
#include "context.h"
#include "ilGen.h"
#include "cmd.h"
namespace fs = std::filesystem;
using namespace evoBasic;
using namespace std;
list<Source*> sources;
bool enable_compile = true;
string output_name = "out.evo";

void enableDevInfo(string str){
    Logger::debugMode = true;
}

void addSources(string path){
    if(fs::exists(path)){
        sources.push_back(new FileSource(path));
    }
    else{
        Logger::error(Format() << "cannot find file '" << path << "'");
        enable_compile = false;
    }
}

void setOutputName(string name){
    output_name = name + ".evo";
}

void printHelpInfo(string){
    cout<<
R"HELPTEXT(
Usage:
     ecc <Options> <Sources> -o=<Output File Name>

Options:
     --help-info
     -h                          print help information
     --output=<File Name>
     --o=<File name>             set output file name
     --library-import
     -lib                        import dependencies
     --dev-info                  print information about lexeme,abstract tree,symbol table and IR

)HELPTEXT"
    <<endl;
    enable_compile = false;
}

void printVersionInfo(string){
    cout<<"evoBasic Compiler"<<endl
        <<"Version 0.1.0"<<endl
        <<"Repo URL "<<endl;
    enable_compile = false;
}

std::list<string> depend_libs;

void addDependLib(string lib){
    depend_libs.push_back(lib);
}

void unmatched(string str){
    Logger::error(Format() << "unsupported option '" << str << "'");
    enable_compile = false;
}

int main(int argc,char *argv[]) {

    CmdDistributor distributor;
    distributor.on("--dev-info",enableDevInfo)
            .on("--output=",setOutputName)
            .on("-o=",setOutputName)
            .on("--help",printHelpInfo)
            .on("-h",printHelpInfo)
            .on("--depend-lib",addDependLib)
            .on("-l",addDependLib)
            .others(addSources)
            .unmatched(unmatched);

    for(int i=1;i<argc;i++){
        distributor.distribute(argv[i]);
    }

    auto context = new Context;
    list<parseTree::Global*> trees;

    if(!enable_compile)return 0;

    if(sources.empty()){
        Logger::error("no input files.");
        printHelpInfo("");
    }

    for(auto source : sources){
        Lexer lexer(source);
        Parser parser(&lexer);
        auto ast = parser.parseGlobal();
        trees.push_back(ast);

        Logger::dev(debugParseTree(ast));

        Semantic::collectSymbol(ast,context);
        Semantic::collectDetail(ast,context);
    }

    Semantic::solveInheritDependencies(context);

    list<ast::Global*> asts;

    for(auto tree : trees){
        auto ast = Semantic::typeCheck(tree,context);
        asts.push_back(ast);
        Logger::dev(debugAST(ast));
    }

    Logger::dev(type::debugSymbolTable(context->getGlobal()->debug()));

    if(Logger::errorCount == 0){
        ILGen gen;
        Semantic::solveByteLengthDependencies(context);
        for(auto ast : asts){
            gen.visitGlobal(ast);
        }
        auto ir = gen.getDocument();
        for(auto &lib : depend_libs){
            ir->addDependenceLibrary(lib);
        }
        Logger::dev(ir->toString());
        fstream file(output_name,ios::binary | ios::out);
        ir->toHex(file);
    }


    cout<<endl<<Logger::errorCount<<" error(s),"
        <<Logger::warningCount<<" warning(s)."<<endl;

    return 0;
}
