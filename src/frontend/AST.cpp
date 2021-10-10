//
// Created by yorkin on 7/12/21.
//

#include"AST.h"
#include"Type.h"
namespace evoBasic{
    Node::Node(Tag tag,initializer_list<pair<Attr,any>> attribute,initializer_list<shared_ptr<Node>> child)
        : tag(tag), child(child), attr(attribute.begin(), attribute.end()){}

    Node::Node(Tag tag,initializer_list<pair<Attr,any>> attribute)
        : tag(tag), attr(attribute.begin(), attribute.end()){}

    Node::Node(Tag tag,initializer_list<shared_ptr<Node>> child)
        :tag(tag),child(child){}

    const vector<string> AccessFlagToString{"Public","Private","Friend"};
    const vector<string> MethodFlagToString{"Virtual","Override","Normal","Static"};

    void Node::print(ostream& out) {
        print("","*",true,out);
    }
    void Node::print(string prefix,const string& mark,bool isLast,ostream& out) {
        if(tag==Tag::Error){
            const string RedBegin = "\033[31m";
            const string ColorEnd = "\033[0m";
            out<<prefix<<mark<<"─"<<RedBegin<<TagToString[(int)tag]<<ColorEnd;
        }
        else out<<prefix<<mark<<"─"<<TagToString[(int)tag];
        if(!attr.empty()){
            out<<" {";
            bool firstAttr = true;
            for(auto x:attr){
                if(firstAttr)firstAttr=false;else out<<',';
                out<<AttrToString[(int)x.first]<<':';
                auto& target = x.second.type();
                if(target==typeid(string))out<<'"'<<any_cast<string>(x.second)<<'"';
                else if(target==typeid(int))out<<any_cast<int>(x.second);
                else if(target==typeid(bool))out<<(any_cast<bool>(x.second)?"true":"false");
                else if(target==typeid(Position)){
                    auto pos = any_cast<Position>(x.second);
                    out<<"("<<pos.y<<","<<pos.x<<')';
                }
                else if(target==typeid(AccessFlag))
                    out<<AccessFlagToString[(int)any_cast<AccessFlag>(x.second)];
                else if(target==typeid(MethodFlag))
                    out<<MethodFlagToString[(int)any_cast<MethodFlag>(x.second)];
            }
            out<<"}";
        }
        out<<endl;
        if(isLast)prefix+=' ';else prefix+=mark;
        for(int i=0;i<child.size();i++){
            if(child[i]==nullptr)continue;
            if(i!=child.size()-1){
                child[i]->print(prefix + "  ","│",false,out);
            }else{
                child[i]->print(prefix + "  ","└",true,out);
            }
        }
    }

    vector<string> Node::TagToString{
            "Module","Class","Function","Sub","Enum","Type","TypeField","Let","Variable","Annotation",
            "Statements","EnumMember",
            "If","ElseIf","Else","Loop","Select","Case","DefaultCase",
            "For","Return","Continue","ExitFor","ExitLoop","ExitFunc","ExitSub",
            "Callee","ParameterArg","GenericArg","Assign'='",
            "Dot'.'","Comma','",
            "SelfNeg'-'","SelfPot'+'",
            "Add'+'","Minus'-'","Mul'*'","Div'/'","FDiv'\\'",
            "EQ'=='","NE'<>'","GT'>'","GE'>='","LT'<'","LE'<='",
            "And","Or","Xor","Not",
            "ParameterList","Parameter","Generic","Locating",
            "ID","Digit","Decimal","String","CHAR",
            "ClassMember","ModuleMember",
            "Public","Private","Friend","Static","Virtual","Override","Normal","Global",
            "<Syntax Error>","Empty","Cast","Expression","Import","ExternalFunction","Path"
    };

    vector<string> Node::AttrToString{
        "Type","Name","Value","ValueKind","IsByval","IsOptional","Lexeme","Pos","Access","Method"
    };

    Node::Node(Tag tag):tag(tag){}

    shared_ptr<Node> Node::Empty(new Node(Tag::Empty));
    shared_ptr<Node> Node::Error(new Node(Tag::Error));

    Position Node::pos() {
        return get<Position>(Attr::Position);
    }

    shared_ptr<Node> make_node(Tag tag,initializer_list<pair<Attr,any>> attribute,initializer_list<shared_ptr<Node>> child){
        return make_shared<Node>(tag,attribute,child);
    }

    shared_ptr<Node> make_node(Tag tag,initializer_list<pair<Attr,any>> attribute){
        return make_shared<Node>(tag,attribute);
    }

    shared_ptr<Node> make_node(Tag tag,initializer_list<shared_ptr<Node>> child){
        return make_shared<Node>(tag,child);
    }

    shared_ptr<Node> make_node(Tag tag){
        return make_shared<Node>(tag);
    }

}