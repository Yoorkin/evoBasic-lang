//
// Created by yorkin on 8/4/21.
//

#include "Link.h"

#include <utility>
#include "../config.h"
namespace evoBasic{
    namespace env{
        void LinkManager::addLibraryDirectory(std::string path_str) {
            fs::path path(path_str);
            if(!fs::exists(path)){
                Logger::error(Format()<<"找不到路径'"<<path_str<<"'");
                return;
            }
            fs::directory_entry entry(path_str);
            if(entry.status().type()!=fs::file_type::directory){
                Logger::error(Format()<<"'"<<path_str<<"'不是一个文件夹");
                return;
            }
            fs::directory_iterator list(path_str);
            for(auto& x:list){
                if(!x.is_regular_file())continue;
                auto opt = optionalLibrary.find(x.path().stem());
                if(opt!=optionalLibrary.end()){
                    if(opt->second.second == LibraryType::Static && x.path().extension() == config::sharedlib_ext){
                        optionalLibrary[x.path().stem().string()]=make_pair(x.path(),LibraryType::Dynamic);
                    }
                    else{
                        Logger::error(Format()<<"LibraryPath中存在多个名为'"<<x.path().stem()<<"'的库文件");
                    }
                }
                else{
                    LibraryType type;
                    if(x.path().extension()==config::sharedlib_ext) type = LibraryType::Dynamic;
                    else type = LibraryType::Static;

                    optionalLibrary.insert(make_pair(x.path().stem().string(),make_pair(x.path(),type)));
                }
            }
        }

        void LinkManager::addLibrary(string filename, bool pickDynamic) {
            auto target = optionalLibrary.find(filename);
            if(target == optionalLibrary.end()){
                Logger::error(Format()<<"找不打库'"<<filename<<"'");
            }
            else {
                library.push_back(target->second.first);
            }
        }

        const vector<fs::path> &LinkManager::getLibrarysPath() {
            return this->library;
        }

        void SourceManager::addHeaderDirectory(string path_str) {
            fs::path path(path_str);
            if(!fs::exists(path)){
                Logger::error(Format()<<"找不到路径'"<<path_str<<"'");
                return;
            }
            fs::directory_entry entry(path_str);
            if(entry.status().type()!=fs::file_type::directory){
                Logger::error(Format()<<"'"<<path_str<<"'不是一个文件夹");
                return;
            }
            fs::directory_iterator list(path_str);
            for(auto& x:list){
                if(!x.is_regular_file() || x.path().extension()!=config::header_ext)continue;
                optionalHeader.insert(make_pair(x.path().stem(),path));
            }
        }

        void SourceManager::addHeader(string filename) {
            auto target = optionalHeader.find(filename);
            if(target == optionalHeader.end()){
                Logger::error(Format()<<"找不打声明文件'"<<filename<<"'");
            }
            else {
                sourcePath.push_back(target->second);
            }
        }

        void SourceManager::addSource(string path_str) {
            fs::path path(path_str);
            if(!fs::exists(path)){
                Logger::error(Format()<<"找不到文件'"<<path_str<<"'");
                return;
            }
            sourcePath.push_back(path);
        }

        const vector<fs::path> &SourceManager::getSourcesPath() {
            return this->sourcePath;
        }

    }
}
