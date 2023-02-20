#ifndef ASCII_TREE
#define ASCII_TREE
#include <string>
#include <list>

class Node{
    std::string Print(std::string indent, bool isLast){
        std::string ret = indent;
        if(isLast){
            ret += "  └─";
            indent += "    ";
        }
        else{
            ret += "  ├─";
            indent += "  │ ";
        }


        ret += value;

        if(!childs.empty()){
            int i = 0;
            for(auto child : childs){
                ret += child.Print(indent,i==childs.size()-1);
                i++;
            }
        }

        return ret;
    }
public:
    std::string value;
    std::list<Node> childs;
    std::string ToString(){
        return Print("\n",true);
    }
}

#endif