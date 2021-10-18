#include<iostream>
#include<fstream>
#include<filesystem>
#include<map>
#include<string>
#include<utility>

#include"frontend/Lexer.h"
#include"frontend/Parser.h"
#include"frontend/Semantic.h"
#include"utils/cmdDistribute.h"
#include"utils/Link.h"
#include"config.h"
#include"interpreter/Interpreter.h"

using namespace evoBasic;
using namespace std;
using namespace cmd;
namespace fs = std::filesystem;
using namespace evoBasic::env;

int main(int argc, char* argv[]) {
//    auto test = make_shared<Type::Class>();
//    dynamic_pointer_cast<Type::Domain>(test)->add(AccessFlag::Public,{});

    string outputName = config::defaultOutputName;

    auto currentPath = argv[0];

    LinkManager linkMgr;
    SourceManager sourceMgr;
    CmdDistributor distributor;

    distributor.on("-dev",[&](const string& str){Logger::debugMode = true;})
               .on("-I=",[&](string str){sourceMgr.addHeaderDirectory(std::move(str));})
               .on("-G=",[&](string str){outputName=std::move(str);})
               .on("-l",[&](string str){linkMgr.addLibrary(std::move(str));})
               .on("-L=",[&](string str){linkMgr.addLibraryDirectory(std::move(str));})
               .others([&](string str){sourceMgr.addSource(std::move(str));})
               .unmatched([&](const string& str){Logger::error(Format()<<"未知的编译指令'"<<str<<"'");});

    for(int i=1;i<argc;i++){
        distributor.distribute(argv[i]);
    }


    vector<AST> trees;
    stringstream ast_info_stream;

    for(auto& sourcePath:sourceMgr.getSourcesPath()){
        auto source = make_shared<Source>(sourcePath);
        Logger::error(Position(5,2,3,8,source),"testsettetttttttttttt");
        Lexer lexer(source);
        Parser parser(lexer);
        auto ast = parser.parse();
        ast.root->print(ast_info_stream);
        trees.push_back(ast);
    }

    SymbolTable table(trees);

    Logger::dev(ast_info_stream.str());


    if(Logger::errorCount == 0){
        auto interpreter = make_shared<Interpreter>(table);
        interpreter->execute();
    }

    cout<< endl << Logger::errorCount << " error(s), "
        << Logger::warningCount << " warning(s)" <<endl;

    //Analyzer::check(table,trees);
    auto ma = dynamic_pointer_cast<Type::Domain>(table.global->find("mymodulea").symbol);

}

