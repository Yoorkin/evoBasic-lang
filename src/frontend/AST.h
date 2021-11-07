//
// Created by yorkin on 7/12/21.
//

#ifndef EVOBASIC_AST_H
#define EVOBASIC_AST_H
#include"Lexer.h"
#include<list>
#include<vector>
#include<map>
#include<initializer_list>
#include<any>
#include<stack>
#include<memory>
#include<functional>
#include<typeindex>
#include<stdexcept>
#include"Exception.h"

namespace evoBasic{
    using namespace std;
    enum class Tag:int{
        Module=0,Class,Function,Sub,Enum,Type,TypeField,Let,Variable,Annotation,
        Statements,EnumMember,
        If,ElseIf,Else,Loop,Select,Case,DefaultCase,
        For,Return,Continue,ExitFor,ExitLoop,ExitFunc,ExitSub,
        Callee,ParameterArg,GenericArg,Assign,
        Dot,Comma,
        SelfNeg,SelfPot,
        Add,Minus,Mul,Div,FDiv,
        EQ,NE,GT,GE,LT,LE,
        And,Or,Xor,Not,
        ParameterList,Parameter,Generic,Locating,
        ID,Digit,Decimal,String,Char,
        ClassMember,ModuleMember,
        Public,Private,Friend,Static,Virtual,Override,Normal,Global,
        Error,Empty,Cast,Exprssion,Import,ExternalFunction,Path,Impl,Extend,Init,Operator,Interface
    };

    enum class MethodFlag{Virtual,Override,Normal,Static};
    enum class AccessFlag{Public,Private,Friend,Protected};

    enum class Attr{
        Type,Name,Value,ValueKind,IsByval,IsOptional,Lexeme,Position,AccessFlag,MethodFlag,Symbol
    };

    class Node{
    protected:
        map<Attr,any> attr;
        void print(string prefix,const string& mark,bool isLast,ostream& out);
    public:

        static vector<string> TagToString;
        static vector<string> AttrToString;

        Tag tag;
        vector<shared_ptr<Node>> child;

        template<typename T>
        T get(Attr attribute){
            auto ret = attr.find(attribute);
            if(ret==attr.end())throw "unexpected attribute";
            return any_cast<T>(ret->second);
        }

        template<typename T>
        void set(Attr attribute,T value){
            attr[attribute]=value;
        }

        Position pos();

        Node(Tag tag,initializer_list<pair<Attr,any>> attribute,initializer_list<shared_ptr<Node>> child);
        Node(Tag tag,initializer_list<pair<Attr,any>> attribute);
        Node(Tag tag,initializer_list<shared_ptr<Node>> child);
        explicit Node(Tag tag);
        static shared_ptr<Node> Empty;
        static shared_ptr<Node> Error;

        void print(ostream &out);
    };

    shared_ptr<Node> make_node(Tag tag,initializer_list<pair<Attr,any>> attribute,initializer_list<shared_ptr<Node>> child);

    shared_ptr<Node> make_node(Tag tag,initializer_list<pair<Attr,any>> attribute);

    shared_ptr<Node> make_node(Tag tag,initializer_list<shared_ptr<Node>> child);

    shared_ptr<Node> make_node(Tag tag);

    class AST{
    public:
        shared_ptr<Node> root;
        AST(std::shared_ptr<Node> root):root(root){}
    };
}


#endif //EVOBASIC_AST_H
