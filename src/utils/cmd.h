//
// Created by yorkin on 8/4/21.
//

#ifndef EVOBASIC_CMDDISTRIBUTE_H
#define EVOBASIC_CMDDISTRIBUTE_H

#include<functional>
#include<string>
#include<map>

namespace evoBasic {
    using CmdHandler = std::function<void(std::string)>;

    class CmdDistributor {
        class Node {
        public:
            bool isEnd = false;
            CmdHandler handler;
            std::vector<Node*> childs{256, nullptr};
            ~Node();
        };
        Node *root = new Node();
        CmdHandler other,unmatch;
    public:
        CmdDistributor &on(const std::string &cmd, CmdHandler handler);
        CmdDistributor &others(CmdHandler handler);
        CmdDistributor &unmatched(CmdHandler handler);
        void distribute(std::string raw);
        ~CmdDistributor();
    };
}


#endif // EVOBASIC_CMDDISTRIBUTE_H