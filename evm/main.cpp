#include "backage.pb.h"
#include "loader.h"
#include "processor.h"
#include "runtime.h"

#include <cstdint>
#include <fstream>
#include <string>
#include <iostream>

#include "unicode.h"

using namespace std;

class CmdDispatcher{
public:
    using StrHandler = std::function<bool(std::string)>;
    using VoidHandler = std::function<bool()>;
private:
    struct Data{
        std::string cmd, short_cmd, description;
        bool consume_input;
        StrHandler str_handler;
        VoidHandler void_handler;
    };
    StrHandler fallback_func;
    std::vector<Data> datas;
    std::map<std::string,int> handlers;
public:

    CmdDispatcher &add(std::string cmd, std::string short_cmd, std::string description, StrHandler handler){
        handlers.insert({cmd,datas.size()});
        handlers.insert({short_cmd,datas.size()});
        datas.push_back(Data{cmd,short_cmd,description,true,handler,{}});
        return *this;
    }

    CmdDispatcher &add(std::string cmd, std::string short_cmd, std::string description, VoidHandler handler){
        handlers.insert({cmd,datas.size()});
        handlers.insert({short_cmd,datas.size()});
        datas.push_back(Data{cmd,short_cmd,description,false,{},handler});
        return *this;
    }

    CmdDispatcher &fallback(StrHandler handler){
        fallback_func = handler;
        return *this;
    }

    bool dispatch(std::vector<std::string> input){
        bool flag = true;
        for(int i=0;i<input.size();i++){
            auto &str = input[i];
            if(str[0]=='-'){
                std::string key;
                if(str[1]=='-'){
                    key = str.substr(2,str.size()-2);
                }
                else{
                    key = str.substr(1,str.size()-1);
                }
                auto target = handlers.find(key);
                if(target==handlers.end()){
                    std::cout<<"unexpected option '"<<key<<"'"<<std::endl;
                    flag = false;
                }
                else{
                    auto data = datas[target->second];
                    if(data.consume_input){
                        if(!(data.str_handler(input[++i])))flag = false;
                    }
                    else{
                        if(!data.void_handler())flag = false;
                    }
                }
            }
            else{//fallback
                fallback_func(input[i]);
            }
        }
        return flag;
    }

    void printMenu(){
        std::string ret;
        for(auto data : datas){
            std::cout<<"--"<<std::setfill(' ')<<std::setw(15)<<data.cmd
                     <<"-"<<std::setfill(' ')<<std::setw(15)<<data.short_cmd
                     <<data.description<<std::endl;
        }
    }
};


int main(int argc, char *argv[]){
    std::ofstream out("debug.txt");
    std::clog.rdbuf(out.rdbuf());
    LOG(ARGS[0],std::string(argv[0])<<std::endl);

    std::vector<std::string> args;
    for(int i=1;i<argc;i++){
        args.push_back(std::string(argv[i]));
    }

    std::string run_target = "";
    std::string package_folder = ".";

    CmdDispatcher dispatcher;
    auto flag = dispatcher.add("package-folder","p","path to .bkg folder",[&](std::string path){
        package_folder = path;
        return true;
    })
    .add("help","h","print help info",[&](){
        dispatcher.printMenu();
        return true;
    })
    .fallback([&](std::string target){   
        if(run_target == "") {
            run_target = target;
            return true;
        }
        else {
            std::cout<<"Error: cannot run more than one bkg file."<<std::endl;
            return false;
        }
    }).dispatch(args);

    if(run_target==""){
        std::cout<<"Error: nothing to run"<<std::endl;
        flag = false;
    }

    if(!flag)return 0;

    Loader loader(unicode::fromPlatform(package_folder));
    loader.fromPackageFolder(unicode::fromPlatform(run_target));
    loader.load();

    auto cls = (runtime::Class*)(loader.getGlobal()->getChildern().find("OutOfRangeException"_utf32)->second);
    auto ctor = ((runtime::Ctor*)cls->find("#ctor"_utf32));
    
    Processor processor(&loader);
    
    processor.execute(loader.getGlobal()->getMainMethod());
}