#include <iostream>
#include <filesystem>
#include "nullSafe.h"
#include "token.h"
#include "lexer.h"
#include "logger.h"
#include "parser.h"
#include "semantic/semantic.h"
#include "context.h"
#include "codeGen.h"
#include "cmd.h"
namespace fs = std::filesystem;
using namespace evoBasic;
using namespace std;
list<Source*> sources;
bool enable_compile = true;

void enableDevInfo(string str){
    Logger::debugMode = true;
}

void addSources(string path){
    if(fs::exists(path)){
        sources.push_back(new FileSource(path));
    }
    else{
        Logger::error(format()<<"cannot find file '"<<path<<"'");
        enable_compile = false;
    }
}

void setOutputName(string name){

}

void printHelpInfo(string){
    cout<<"Usage:"<<endl
        <<"     ecc <Options> <Sources> -o=<Output File Name>"<<endl
        <<endl
        <<"Options:"<<endl
        <<"     --help-info "<<endl
        <<"     -h                          print help information"<<endl
        <<"     --output-name=<File Name>"<<endl
        <<"     --o=<File name>             set output file name"<<endl
        <<"     --library-import"<<endl
        <<"     -lib                        import dependencies"<<endl;
    enable_compile = false;
}

void printVersionInfo(string){
    cout<<"evoBasic Compiler"<<endl
        <<"Version 0.1.0"<<endl
        <<"Repo URL "<<endl;
    enable_compile = false;
}

void unmatched(string str){
    Logger::error(format()<<"unsupported option '"<<str<<"'");
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

    auto context = make_shared<Context>();
    list<ast::Global*> asts;

    if(!enable_compile)return 0;

    if(sources.empty()){
        Logger::error("no input files.");
        printHelpInfo("");
    }

    for(auto source : sources){
        Lexer lexer(source);
        Parser parser(&lexer);
        auto ast = parser.parseGlobal();
        asts.push_back(ast);

        stringstream stream;
        ast->debug(stream,"");
        Logger::dev(stream.str());

        Semantic::collectSymbol(ast,context);
        Semantic::collectDetail(ast,context);
        Semantic::typeCheck(ast,context);

        Logger::dev(context->getGlobal()->debug(0));
    }

    if(Logger::errorCount == 0){
        for(auto ast : asts){
            Semantic::solveByteLengthDependencies(context);

            IRGen gen;

            auto ir = gen.gen(ast,context);
            ir->toString(cout);
            fstream file("out.evo",ios::binary | ios::out);
            ir->toHex(file);
        }
    }

    cout<<endl<<Logger::errorCount<<" error(s),"
        <<Logger::warningCount<<" warning(s)."<<endl;

    return 0;
}
