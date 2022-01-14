//
// Created by yorkin on 8/4/21.
//

#ifndef EVOBASIC_CMDDISTRIBUTE_H
#define EVOBASIC_CMDDISTRIBUTE_H

#include<functional>
#include<string>
#include<map>

namespace evoBasic {
    using CmdHandlerWithInput = std::function<void(std::string)>;
    using CmdHandler = std::function<void()>;
    class CmdDistributor {
        class Node {
        public:
            bool isEnd = false;
            CmdHandlerWithInput handler;
            CmdHandler handler_void;
            std::vector<Node*> childs{256, nullptr};
            ~Node();
        };
        Node *root = new Node();
        CmdHandlerWithInput other,unmatch;
    public:
        CmdDistributor &on(const std::string &cmd, CmdHandlerWithInput handler);
        CmdDistributor &on(const std::string &cmd, CmdHandler handler);
        CmdDistributor &others(CmdHandlerWithInput handler);
        CmdDistributor &unmatched(CmdHandlerWithInput handler);
        CmdDistributor &on(const std::string &cmd, const std::string &alias, CmdHandlerWithInput handler);
        CmdDistributor &on(const std::string &cmd, const std::string &alias, CmdHandler handler);
        void distribute(std::string raw);
        ~CmdDistributor();

        Node *configure_cmd(const std::string &cmd);
    };
}


#endif // EVOBASIC_CMDDISTRIBUTE_H