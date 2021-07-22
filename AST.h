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
        Public,Private,Friend,Static,Virtual,Override,
        Error
    };



    enum class Attr{
        Type,Name,Value,ValueKind,IsByval,IsOptional,Lexeme
    };

    enum class ValueKind{
        lvalue,rvalue
    };

    class Node{
        map<Attr,any> attr;
    public:
        Tag tag;
        vector<Node*> child;

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

        Node(Tag tag,initializer_list<pair<Attr,any>> attribute,initializer_list<Node*> child);
        Node(Tag tag,initializer_list<pair<Attr,any>> attribute);
        Node(Tag tag,initializer_list<Node*> child);
        explicit Node(Tag tag);
        static vector<string> TagToString;
        static vector<string> AttrToString;
        void print(string prefix,string mark,bool isLast,ostream& out);
    };

    typedef void (*Callback)(Node* node);
    class Visitor{
        map<Tag,Callback> subscribe;
    public:
        void on(Tag tag,Callback callback);
        void visit(Node* ast);
    };
}


#endif //EVOBASIC_AST_H
