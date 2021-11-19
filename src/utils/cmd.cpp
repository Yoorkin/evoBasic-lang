//
// Created by yorkin on 8/4/21.
//

#include "cmd.h"

using namespace std;

namespace evoBasic {

    CmdDistributor &CmdDistributor::on(const string &cmd, CmdHandler handler) {
        Node *current = root;
        for (auto x:cmd) {
            if (current->childs[x] == nullptr)
                current->childs[x] = new Node();
            current = current->childs[x];
        }
        current->isEnd = true;
        current->handler = std::move(handler);
        return *this;
    }

    CmdDistributor &CmdDistributor::others(CmdHandler handler) {
        other = std::move(handler);
        return *this;
    }

    void CmdDistributor::distribute(const string &raw) {
        Node *current = root;
        for (int i = 0; i < raw.size(); i++) {
            auto x = raw[i];
            if (current->isEnd) {
                current->handler(raw.substr(i, raw.size()));
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
        current->handler("");
    }

    CmdDistributor::~CmdDistributor() {
        delete root;
    }

    CmdDistributor &CmdDistributor::unmatched(CmdHandler handler) {
        unmatch = move(handler);
        return *this;
    }


    CmdDistributor::Node::~Node() {
        for(auto& x:childs)delete x;
    }
}