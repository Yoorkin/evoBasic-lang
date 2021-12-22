#include <iostream>
#include <filesystem>
#include "nullSafe.h"
#include "token.h"
#include "lexer.h"
#include "logger.h"
#include "parser.h"
#include "semantic/semantic.h"
#include "context.h"
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
     --output-name=<File Name>
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

void unmatched(string str){
    Logger::error(Format() << "unsupported option '" << str << "'");
    enable_compile = false;
}

int main(int argc,char *argv[]) {

    CmdDistributor distributor;
    distributor.on("--dev-info",enableDevInfo)
            .on("--output-name=",setOutputName)
            .on("-o=",setOutputName)
            .on("--help",printHelpInfo)
            .on("-h",printHelpInfo)
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

    for(auto tree : trees){
        auto ast = Semantic::typeCheck(tree,context);
        Logger::dev(debugAST(ast));
    }

    Logger::dev(context->getGlobal()->debug(0));

//    if(Logger::errorCount == 0){
//        for(auto ast : trees){
//            Semantic::solveByteLengthDependencies(context);
//            IRGen gen;
//
//            auto ir = gen.gen(ast,context);
//            ir->toString(cout);
//            fstream file("out.evo",ios::binary | ios::out);
//            ir->toHex(file);
//        }
//    }

    cout<<endl<<Logger::errorCount<<" error(s),"
        <<Logger::warningCount<<" warning(s)."<<endl;

    return 0;
}
