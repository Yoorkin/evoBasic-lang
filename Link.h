//
// Created by yorkin on 8/4/21.
//

#ifndef EVOBASIC_LINKMANAGER_H
#define EVOBASIC_LINKMANAGER_H
#include<string>
#include<map>
#include<vector>
#include<filesystem>
#include"Logger.h"

namespace evoBasic{
    namespace link{
        using namespace std;
        namespace fs = std::filesystem;

        enum class LibraryType {Dynamic,Static};

        class LinkManager {
            map<string,pair<fs::path,LibraryType>> optionalLibrary;
            map<string,fs::path> optionalHeader;
            vector<fs::path> library;
        public:
            void addLibraryDirectory(string path_str);
            void addHeaderDirectory(string path_str);
            void addLibrary(string filename,bool pickDynamic=true);
            void addHeader(string header);
            const vector<fs::path> getLibrarysPath();
            fs::path getHeaderPath(string name);
        };
    }
}

#endif // EVOBASIC_LINKMANAGER_H
