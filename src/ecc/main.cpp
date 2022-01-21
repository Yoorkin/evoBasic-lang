#include <iostream>
#include <filesystem>
#include <utils/nullSafe.h>
#include <parser/token.h>
#include <parser/lexer.h>
#include <utils/logger.h>
#include <parser/parser.h>
#include <analyzer/semantic.h>
#include <analyzer/context.h>
#include <codegen/ilGen.h>
#include <utils/cmd.h>
#include <utils/config.h>

namespace fs = std::filesystem;
using namespace evoBasic;
using namespace std;


bool enable_compile = true;
list<Source*> sources;
list<fs::path> direct_dependencies;

CompileTimeContext *context = nullptr;

string output_name = "out";


void addSourcesOrPackages(string str_path){
    if(fs::exists(str_path)){
        fs::path file_path(str_path);
        if(file_path.extension() == extensions::source){
            sources.push_back(new FileSource(str_path));
        }
        else if(file_path.extension() == extensions::package){
            context->getLoader()->addToWaitingDeque(str_path);
            direct_dependencies.push_back(str_path);
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
    output_name = name;
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
    context = new CompileTimeContext;

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

    if(!enable_compile)return 0;
    if(sources.empty()){
        Logger::error("no input source files.");
    }

    context->getLoader()->loadPackages();
    context->getLoader()->loadPackagesSymbols(context);

    list<parseTree::Global*> trees;

    for(auto source : sources){
        Lexer lexer(source);
        Parser parser(&lexer);
        auto parse_tree = parser.parseGlobal();
        trees.push_back(parse_tree);

        Logger::lazy_print(Channel::ParseTrees,[&](){
            return debugParseTree(parse_tree);
        });

        Semantic::collectSymbol(parse_tree, context);
        Semantic::collectDetail(parse_tree, context);
    }

    Semantic::solveInheritDependencies(context);

    list<ast::Global*> asts;

    for(auto tree : trees){
        auto ast = Semantic::typeCheck(tree,context);
        asts.push_back(ast);
        Logger::lazy_print(Channel::AbstractTrees,[&](){
            return debugAST(ast);
        });
    }

    Logger::lazy_print(Channel::SymbolTable,[&](){
        return type::debugSymbolTable(context->getGlobal()->debug());
    });

    //if(Logger::errorCount == 0){
        ILGen gen;
        il::Document document(output_name);
        Semantic::solveByteLengthDependencies(context);
        for(auto ast : asts){
            gen.visitGlobal(ast,&document);
        }

        for(auto &package : direct_dependencies){
            document.addDependenceLibrary(package.stem().string());
        }

        Logger::lazy_print(Channel::IL,[&](){
            return document.toString();
        });

        fstream file(output_name + extensions::package,ios::binary | ios::out);
        document.toHex(file);
    //}


    cout<<endl<<Logger::errorCount<<" error(s),"
        <<Logger::warningCount<<" warning(s)."<<endl;

    delete context;
    return 0;
}
