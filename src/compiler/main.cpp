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

namespace extensions{
    const string package = ".bkg";
    const string source = ".eb";
}

bool enable_compile = true;
list<Source*> sources;
list<string> packages;

string output_name = "out.bkg";


void addSourcesOrPackages(string str_path){
    if(fs::exists(str_path)){
        fs::path file_path(str_path);
        if(file_path.extension() == extensions::source){
            sources.push_back(new FileSource(str_path));
        }
        else if(file_path.extension() == extensions::package){
            packages.push_back(str_path);
        }
        else{
            Logger::error(Format() << "Unrecognized file extension "<< file_path.extension());
        }
    }
    else{
        Logger::error(Format() << "cannot find file '" << str_path << "'");
        enable_compile = false;
    }
}

void setOutputName(string name){
    output_name = name + ".bkg";
}

void printVersionInfo(){
    cout<<"evoBasic Compiler"<<endl
        <<"Version 0.1.0"<<endl
        <<"Repo URL "<<endl;
    enable_compile = false;
}

void unmatched(string str){
    Logger::error(Format() << "unsupported option '" << str << "'");
    enable_compile = false;
}


void printHelpInfo(){
    cout<<
        R"HELPTEXT(
Usage:
     ecc <Options> <Sources> <Packages> -o=<Output File Name>

Options:
     --help-info
     -h                          print help information
     --output=<File Name>
     --o=<File name>             set output package name
     --dev-info                  print information about lexeme,abstract tree,symbol table and IR

)HELPTEXT"
        <<endl;
    enable_compile = false;
}

void enableEmitAST(){
    Logger::enable(Channel::AbstractTrees);
}

void enableEmitTokens(){
    Logger::enable(Channel::Tokens);
}

void enableEmitParseTrees(){
    Logger::enable(Channel::ParseTrees);
}

void enableEmitSymbolTable(){
    Logger::enable(Channel::SymbolTable);
}

void enableEmitIL(){
    Logger::enable(Channel::IL);
}

void enableDevInfo(){
    enableEmitAST();
    enableEmitTokens();
    enableEmitParseTrees();
    enableEmitSymbolTable();
    enableEmitIL();
}

int main(int argc,char *argv[]) {

    CmdDistributor distributor;
    distributor.on("--dev-info",        enableDevInfo)
            .on("--output=",    "-o=",  setOutputName)
            .on("--help",       "-h",   printHelpInfo)
            .on("--emit-tokens",       enableEmitTokens)
            .on("--emit-parse-trees",  enableEmitParseTrees)
            .on("--emit-ast",          enableEmitAST)
            .on("--emit-symbol-table", enableEmitSymbolTable)
            .on("--emit-il",           enableEmitIL)
            .others(addSourcesOrPackages)
            .unmatched(unmatched);

    for(int i=1;i<argc;i++){
        distributor.distribute(argv[i]);
    }

    auto context = new Context;
    list<parseTree::Global*> trees;

    if(!enable_compile)return 0;

    if(sources.empty()){
        Logger::error("no input files.");
    }


    for(const auto& package_path : packages){
        fstream package_file(package_path,ios::binary | ios::in);
        auto package = new il::Document(package_file);
        Logger::print(Channel::IL, package->toString());
    }


    for(auto source : sources){
        Lexer lexer(source);
        Parser parser(&lexer);
        auto ast = parser.parseGlobal();
        trees.push_back(ast);

        Logger::print(Channel::ParseTrees,debugParseTree(ast));

        Semantic::collectSymbol(ast,context);
        Semantic::collectDetail(ast,context);
    }

    Semantic::solveInheritDependencies(context);

    list<ast::Global*> asts;

    for(auto tree : trees){
        auto ast = Semantic::typeCheck(tree,context);
        asts.push_back(ast);
        Logger::print(Channel::AbstractTrees,debugAST(ast));
    }

    Logger::print(Channel::SymbolTable,type::debugSymbolTable(context->getGlobal()->debug()));

    if(Logger::errorCount == 0){
        ILGen gen;
        il::Document document;
        Semantic::solveByteLengthDependencies(context);
        for(auto ast : asts){
            gen.visitGlobal(ast,&document);
        }

        for(auto &package : packages){
            document.addDependenceLibrary(package);
        }
        Logger::print(Channel::IL,document.toString());
        fstream file(output_name,ios::binary | ios::out);
        document.toHex(file);
    }


    cout<<endl<<Logger::errorCount<<" error(s),"
        <<Logger::warningCount<<" warning(s)."<<endl;

    return 0;
}
