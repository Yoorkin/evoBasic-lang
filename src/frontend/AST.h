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
#include"AST.h"
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
        Error,Empty,Cast,Exprssion,Import,ExternalFunction,Path
    };

    enum class MethodFlag{Virtual,Override,Normal,Static};
    enum class AccessFlag{Public,Private,Friend};

    enum class Attr{
        Type,Name,Value,ValueKind,IsByval,IsOptional,Lexeme,Position,AccessFlag,MethodFlag,DeclarationSymbol
    };

    enum class ValueKind{
        lvalue,rvalue
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
        Logger &logger;
        shared_ptr<Node> root;
        AST(Logger &logger,shared_ptr<Node> root):logger(logger),root(root){}
    };


//    template<typename Args,typename Rets>
//    class Visitor{
//    public:
//        using Callback = function<Rets(shared_ptr<Node> node,Args args)>;
//        using CallbackWithNoReturn = function<void(shared_ptr<Node> node,Args args)>;
//        using ExceptionHandler = function<void(exception_ptr)>;
//    private:
//        map<Tag,Callback> events;
//        map<Tag,CallbackWithNoReturn> noRetEvents;
//        ExceptionHandler handler;
//        CallbackWithNoReturn defaultCallback;
//    public:
//
//        Visitor &on(Tag tag,Callback callback){
//            events.insert(make_pair(tag,callback));
//            return *this;
//        }
//
//        Visitor &on(Tag tag,CallbackWithNoReturn callback){
//            noRetEvents.insert(make_pair(tag,callback));
//            return *this;
//        }
//
//        Visitor &onException(ExceptionHandler handler){
//            this->handler=handler;
//            return *this;
//        }
//
//        Visitor &onDefault(CallbackWithNoReturn callback){
//            this->defaultCallback = callback;
//            return *this;
//        }
//
//        void visit(shared_ptr<Node> root,Args args){
//            //cout<<Node::TagToString[(int)root->tag]<<endl;
//            try{
//                auto target = events.find(root->tag);
//                auto noRetTarget = noRetEvents.find(root->tag);
//                if(target!=events.end()){
//                    target->second(root,args);
//                }
//                else if(noRetTarget!=noRetEvents.end()){
//                    noRetTarget->second(root,args);
//                }
//                else{
//                    if(defaultCallback)defaultCallback(root,args);
//                }
//            }
//            catch(...){
//                if(handler)handler(std::current_exception());
//                else std::rethrow_exception(std::current_exception());
//            }
//        }
//    };
}


#endif //EVOBASIC_AST_H
