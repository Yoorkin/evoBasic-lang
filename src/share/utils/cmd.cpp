//
// Created by yorkin on 8/4/21.
//

#include "cmd.h"

using namespace std;

namespace evoBasic {

    CmdDistributor::Node *CmdDistributor::configure_cmd(const string &cmd){
        Node *current = root;
        for (auto x:cmd) {
            if (current->childs[x] == nullptr)
                current->childs[x] = new Node();
            current = current->childs[x];
        }
        current->isEnd = true;
        return current;
    }

    CmdDistributor &CmdDistributor::on(const string &cmd, CmdHandlerWithInput handler) {
        configure_cmd(cmd)->handler = std::move(handler);
        return *this;
    }

    CmdDistributor &CmdDistributor::on(const string &cmd, CmdHandler handler) {
        configure_cmd(cmd)->handler_void = std::move(handler);
        return *this;
    }


    CmdDistributor &CmdDistributor::others(CmdHandlerWithInput handler) {
        other = std::move(handler);
        return *this;
    }

    void CmdDistributor::distribute(string raw) {
        Node *current = root;
        for (int i = 0; i < raw.size(); i++) {
            auto x = raw[i];
            if (current->isEnd) {
                auto arg = raw.substr(i, raw.size()-i);
                if(current->handler){
                    current->handler(arg);
                }
                else if(current->handler_void){
                    current->handler_void();
                }
                return;
            }
            else if (current->childs[x] != nullptr){
                current = current->childs[x];
            }
            else if (current==root){
                other(raw);
                return;
            }
            else {
                unmatch(raw);
                return;
            }
        }

        if(current->handler){
            current->handler("");
        }
        else if(current->handler_void){
            current->handler_void();
        }
    }

    CmdDistributor::~CmdDistributor() {
        delete root;
    }

    CmdDistributor &CmdDistributor::unmatched(CmdHandlerWithInput handler) {
        unmatch = move(handler);
        return *this;
    }

    CmdDistributor &CmdDistributor::on(const std::string &cmd, const std::string &alias, CmdHandlerWithInput handler) {
        return on(cmd,handler).on(alias,handler);
    }

    CmdDistributor &CmdDistributor::on(const string &cmd, const string &alias, CmdHandler handler) {
        return on(cmd,handler).on(alias,handler);
    }


    CmdDistributor::Node::~Node() {
        for(auto& x:childs)delete x;
    }
}