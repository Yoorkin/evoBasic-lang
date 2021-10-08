//
// Created by yorkin on 8/4/21.
//

#ifndef EVOBASIC_LINK_H
#define EVOBASIC_LINK_H
#include<string>
#include<map>
#include<vector>
#include<filesystem>
#include"Logger.h"

namespace evoBasic{
    namespace env{
        using namespace std;
        namespace fs = std::filesystem;

        enum class LibraryType {Dynamic,Static};

        class LinkManager {
            map<string,pair<fs::path,LibraryType>> optionalLibrary;
            map<string,fs::path> optionalHeader;
            vector<fs::path> library;
        public:
            void addLibraryDirectory(string path_str);
            void addLibrary(string filename,bool pickDynamic=true);
            const vector<fs::path>& getLibrarysPath();
        };

        class SourceManager {
            map<string,fs::path> optionalHeader;
            vector<fs::path> sourcePath;
        public:
            void addHeaderDirectory(string path_str);
            void addHeader(string filename);
            void addSource(string path_str);
            const vector<fs::path>& getSourcesPath();
        };
    }
}

#endif // EVOBASIC_LINK_H
