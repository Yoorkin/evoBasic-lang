//
// Created by yorkin on 12/11/21.
//

#ifndef EVOBASIC_IL_H
#define EVOBASIC_IL_H
#include <string>
#include <ostream>
#include <vector>
#include <any>
#include <map>
#include <data.h>
#include <parseTree.h>
#include <list>
#include <type.h>
#include <variant>

namespace evoBasic::il{
    class Class;
    class Interface;
    class Inst;
    class SFtn;
    class VFtn;
    class Block;
    class FtnBase;
    class Node;
    class Member;
    class Document;

//    using namespace data;
//
//    using MemberSet = std::vector<Member*>;
//    using ConstantPack = std::tuple<DataType,std::any>;
//    using BytecodePlaceholder = std::variant<i8*,i16*,i32*,i64*,u8*,u16*,u32*,u64*,f32*,f64*,
//                           std::string*,Bytecode,Node*,MemberSet*,ConstantPack*>;
//
//    using BytecodeSeq = std::list<BytecodePlaceholder>;
//
//    enum class BytecodePlaceholderEnum {
//        i8,i16,i32,i64,u8,u16,u32,u64,f32,f64,Notation,Constructed,MemberSet,ConstantPack
//    };
//
//    BytecodeSeq getSeq(){
//        data::u32 name;
//        auto seq = BytecodeSeq{
//            Bytecode::TokenRef,&name
//        };
//
//    }



    class Node {
        Document *document = nullptr;
    public:
        Document *getDocument();
        void setDocument(Document *document);
        virtual std::string toString()=0;
        virtual void toHex(std::ostream &stream)=0;
        virtual void fromHex(std::istream &stream){};
    };


    class TokenDef : public Node{
    protected:
        data::u64 id;
    public:
        virtual std::string getName()=0;
        data::u64 getID();
    };

    class TextTokenDef : public TokenDef{
        std::string text;
    public:

        TextTokenDef(Document *document,data::u64 id,std::string text) : text(text){
            this->id = id;
            this->setDocument(document);
        }
        TextTokenDef(Document *document,data::u64 id) : TextTokenDef(document,id,"") {}

        std::string toString()override;
        std::string getName()override;
        void toHex(std::ostream &stream)override;
        void fromHex(std::istream &stream)override;
    };

    class ConstructedTokenDef : public TokenDef{
        std::list<data::u64> sub_token_list;
    public:

        ConstructedTokenDef(Document *document,data::u64 id,std::list<data::u64> sub_tokens) : sub_token_list(sub_tokens){
            this->id = id;
        }
        ConstructedTokenDef(Document *document,data::u64 id) : ConstructedTokenDef(document,id,{}){}
        std::string toString()override;
        std::string getName()override;
        virtual void toHex(std::ostream &stream)override;
        virtual void fromHex(std::istream &stream)override;
    };

    class TokenRef : public Node{
        data::u64 id = -1;
    public:
        using ID = data::u64;
        TokenRef()=default;
        TokenRef(Document *document, data::u64 id);
        explicit TokenRef(Document *document);

        void toHex(std::ostream &stream)override;
        void fromHex(std::istream &stream)override;
        std::string toString()override;
        data::u64 getID();
        bool isEmpty();
    };


    class Access : public Node{
        AccessFlag flag = AccessFlag::Private;
    public:
        Access()=default;
        explicit Access(AccessFlag flag) : flag(flag){}
        std::string toString()override;
        void toHex(std::ostream &stream)override;
        void fromHex(std::istream &stream)override;
    };

    class Extend : public Node{
    public:
        TokenRef target;
        std::string toString()override;
        void toHex(std::ostream &stream)override;
        void fromHex(std::istream &stream)override;
    };

    class Impl : public Node{
    public:
        TokenRef target;
        std::string toString()override;
        void toHex(std::ostream &stream)override;
        void fromHex(std::istream &stream)override;
    };

    class Lib : public Node{
    public:
        Lib()=default;
        explicit Lib(TokenRef target) : target(target){}
        TokenRef target;
        std::string toString()override;
        void toHex(std::ostream &stream)override;
        void fromHex(std::istream &stream)override;
    };

    class Member : public Node{
    public:
        TokenRef name;
        Access access;
        void toHex(std::ostream &stream)override;
        void fromHex(std::istream &stream)override;
    };

    class Class : public Member{
    public:
        Extend extend;
        std::vector<Impl> impls;
        std::vector<Member*> members;
        std::string toString()override;
        void toHex(std::ostream &stream)override;
        void fromHex(std::istream &stream)override;
    };

    class Module : public Member{
    public:
        std::vector<Member*> members;
        std::string toString()override;
        void toHex(std::ostream &stream)override;
        void fromHex(std::istream &stream)override;
    };

    class Interface : public Member{
    public:
        std::vector<FtnBase*> ftns;
        std::string toString()override;
        void toHex(std::ostream &stream)override;
        void fromHex(std::istream &stream)override;
    };

    class Pair : public Node{
    public:
        TokenRef name;
        data::u32 value;
        std::string toString()override;
        void toHex(std::ostream &stream)override;
        void fromHex(std::istream &stream)override;
    };

    class Enum : public Member{
    public:
        std::vector<Pair> pairs;
        std::string toString()override;
        void toHex(std::ostream &stream)override;
        void fromHex(std::istream &stream)override;
    };

    class Fld : public Member{
    public:
        TokenRef type;
        std::string toString()override;
        void toHex(std::ostream &stream)override;
        void fromHex(std::istream &stream)override;
    };

    class SFld : public Fld{
    public:
        std::string toString()override;
        void toHex(std::ostream &stream)override;
        void fromHex(std::istream &stream)override;
    };

    class Record : public Member{
    public:
        std::vector<Fld*> fields;
        std::string toString()override;
        void toHex(std::ostream &stream)override;
        void fromHex(std::istream &stream)override;
    };

    class Param : public Node{
    public:
        bool is_ref = false;
        TokenRef name,type;
        void toHex(std::ostream &stream)override;
        void fromHex(std::istream &stream)override;
    };

    class Regular : public Param{
    public:
        std::string toString()override;
        void toHex(std::ostream &stream)override;
        void fromHex(std::istream &stream)override;
    };

    class Opt : public Param{
    public:
        Block *initial = nullptr;
        std::string toString()override;
        void toHex(std::ostream &stream)override;
        void fromHex(std::istream &stream)override;
    };

    class Inf : public Param{
    public:
        std::string toString()override;
        void toHex(std::ostream &stream)override;
        void fromHex(std::istream &stream)override;
    };

    class Local : public Node{
    public:
        data::u16 address = -1;
        TokenRef name,type;
        std::string toString()override;
        void toHex(std::ostream &stream)override;
        void fromHex(std::istream &stream)override;
    };

    class Result : public Node{
    public:
        TokenRef type;
        std::string toString()override;
        void toHex(std::ostream &stream)override;
        void fromHex(std::istream &stream)override;
    };

    class FtnBase : public Member{
    public:
        std::vector<Param*> params;
        Result *result = nullptr;
        std::string toString()override;
        void toHex(std::ostream &stream)override;
        void fromHex(std::istream &stream)override;
    };

    class FtnWithDefinition : public FtnBase{
    public:
        std::vector<Local*> locals;
        std::string toString()override;
        std::list<Block*> blocks;
        void toHex(std::ostream &stream)override;
        void fromHex(std::istream &stream)override;
    };

    class Ctor : public FtnWithDefinition{
    public:
        std::string toString()override;
        void toHex(std::ostream &stream)override;
        void fromHex(std::istream &stream)override;
    };

    class Ftn : public FtnWithDefinition{
    public:
        std::string toString()override;
        void toHex(std::ostream &stream)override;
        void fromHex(std::istream &stream)override;
    };

    class VFtn : public FtnWithDefinition{
    public:
        std::string toString()override;
        void toHex(std::ostream &stream)override;
        void fromHex(std::istream &stream)override;
    };

    class SFtn : public FtnWithDefinition{
    public:
        std::string toString()override;
        void toHex(std::ostream &stream)override;
        void fromHex(std::istream &stream)override;
    };

    class ExtAlias : public Node{
        TokenRef token;
    public:
        ExtAlias()=default;
        explicit ExtAlias(TokenRef token) : token(token){}
        std::string toString()override;
        void toHex(std::ostream &stream)override;
        void fromHex(std::istream &stream)override;
    };

    class Ext : public FtnBase{
    public:
        Lib lib;
        ExtAlias alias;
        std::string toString()override;
        void toHex(std::ostream &stream)override;
        void fromHex(std::istream &stream)override;
    };


    class Inst : public Node{
    public:
        using ByteSize = data::u32;
        virtual ByteSize getByteSize() = 0;
    };

    class InstWithOp : public Inst{
    public:
        enum Op{Nop,Ret,CallVirt,CallExt,Callstatic,Call,Ldnull,And,Or,Xor,Ldloca,Ldarga,Ldelema,Not}op;
        std::string toString()override;
        void toHex(std::ostream &stream)override;
        data::u32 getByteSize()override;
        InstWithOp(Op op):op(op){}
    };

    class InstWithToken : public Inst{
    public:
        enum Op{Ldftn,Ldsftn,Ldvftn,Ldc,Newobj,Invoke,Ldflda,Ldsflda}op;
        InstWithToken(Op op, TokenRef token): op(op), token(token){}
        TokenRef token;
        std::string toString()override;
        data::u32 getByteSize()override;
        void toHex(std::ostream &stream)override;
    };

    class InstJif : public Inst{
    public:
        Block *target = nullptr;
        std::string toString()override;
        data::u32 getByteSize()override;
        void toHex(std::ostream &stream)override;
    };

    struct InstBr : public Inst{
    public:
        Block *target = nullptr;
        std::string toString()override;
        data::u32 getByteSize()override;
        void toHex(std::ostream &stream)override;
    };

    class InstPush : public Inst{
    public:
        DataType type;
        std::any value;
        std::string toString()override;
        data::u32 getByteSize()override;
        void toHex(std::ostream &stream)override;
    };

    class InstWithData : public Inst{
    public:
        enum Op{Ldelem,Stelem,Stelema,Ldarg,Starg,Ldloc,Stloc,
                Add,Sub,Mul,Div,FDiv,EQ,NE,LT,GT,LE,GE,Neg,Pop,Dup}op;
        DataType type;
        std::string toString()override;
        void toHex(std::ostream &stream)override;
        data::u32 getByteSize()override;
        InstWithData(Op op,DataType type):op(op),type(type){}
    };

    class InstWithDataToken : public Inst{
    public:
        enum Op{Ldfld,Ldsfld,Stfld,Stsfld}op;
        InstWithDataToken(Op op, DataType type, TokenRef token) : op(op), type(type), token(token){}
        DataType type;
        TokenRef token;
        std::string toString()override;
        data::u32 getByteSize()override;
        void toHex(std::ostream &stream)override;
    };

    class InstCastcls : public Inst{
    public:
        TokenRef src_class,dst_class;
        std::string toString()override;
        data::u32 getByteSize()override;
        void toHex(std::ostream &stream)override;
    };


    class InstConv : public Inst{
    public:
        DataType src,dst;
        std::string toString()override;
        data::u32 getByteSize()override;
        void toHex(std::ostream &stream)override;
    };


    class Document : public Node{
        friend TokenRef;

        std::map<std::string,int> token_pool_map;
        std::vector<TokenDef*> token_pool;

        std::vector<Member*> members;
        std::list<TokenRef> dependencies;
    public:
        Document()=default;
        static Document* loadFromHex(std::istream &stream);

        std::string toString()override;
        void toHex(std::ostream &stream)override;
        void fromHex(std::istream &stream)override;

        TokenRef getTokenRef(data::u64 id);
        TokenRef getTokenRef(std::string text);
        TokenRef getTokenRef(std::list<std::string> full_name_list);

        TokenDef *getTokenDef(data::u64 id);

        Module *createModule(std::string name,AccessFlag access,std::vector<Member*> members);
        Class *createClass(std::string name,AccessFlag access,Extend extend,std::vector<Impl> impls,std::vector<Member*> members);
        Interface *createInterface(std::string name,AccessFlag access,std::vector<FtnBase*> ftns);
        Enum *createEnum(std::string name,AccessFlag access,std::vector<Pair> pairs);
        Record *createRecord(std::string name,AccessFlag access,std::vector<Fld*> fields);
        Fld *createField(std::string name,AccessFlag access,type::Prototype *prototype);
        SFld *createStaticField(std::string name,AccessFlag access,type::Prototype *prototype);

        Ftn *createFunction(std::string name, AccessFlag access, std::vector<Param*> params, Result *result, std::vector<Local*> locals, std::list<Block*> blocks);
        Ctor *createConstructor(AccessFlag access, std::vector<Param*> params, std::vector<Local*> locals, std::list<Block*> blocks);
        VFtn *createVirtualFunction(std::string name, AccessFlag access, std::vector<Param*> params, Result *result, std::vector<Local*> locals, std::list<Block*> blocks);
        SFtn *createStaticFunction(std::string name, AccessFlag access, std::vector<Param*> params, Result *result, std::vector<Local*> locals, std::list<Block*> blocks);

        FtnBase *createInterfaceFunction(std::string name, AccessFlag access, std::vector<Param*> params, Result *result);
        Ext *createExternalFunction(std::string name, std::string lib, ExtAlias alias, AccessFlag access, std::vector<Param*> params, Result *result);

        Regular *createParam(std::string name, type::Prototype *prototype, bool byref);
        Opt *createOption(std::string name,type::Prototype *prototype,bool byref,Block *initial);
        Inf *createParamArray(std::string name,type::Prototype *prototype,bool byref);
        Pair createPair(std::string name,data::u32 value);
        Extend createExtend(type::Class *cls);
        Impl createImplements(type::Interface *interface);
        Local *createLocal(std::string name,type::Prototype *prototype, data::u16 address);
        Result *createResult(type::Prototype *prototype);

        void add(Member* member);

        void addDependenceLibrary(std::string name);
        ExtAlias createExtAlias(std::string text);
    };


    class Block : public Node{
        Inst::ByteSize address = -1;
        Inst::ByteSize size = -1;
    public:
        Inst::ByteSize getAddress();
        Inst::ByteSize getByteSize();
        void setAddress(Inst::ByteSize value);

        std::string toString()override;
        void toHex(std::ostream &stream)override;
        std::vector<Inst*> insts;
        Block(std::initializer_list<Inst*> inst_init);
        Block()=default;
        Block &Br(Block *block);
        Block &Jif(Block *block);
        Block &EQ(DataType data);
        Block &NE(DataType data);
        Block &LT(DataType data);
        Block &GT(DataType data);
        Block &LE(DataType data);
        Block &GE(DataType data);
        Block &Add(DataType data);
        Block &Sub(DataType data);
        Block &Mul(DataType data);
        Block &Div(DataType data);
        Block &FDiv(DataType data);
        Block &Neg(DataType data);
        Block &And();
        Block &Or();
        Block &Xor();
        Block &Not();
        Block &Nop();
        Block &Pop(DataType data);
        Block &Dup(DataType data);
        Block &Ret();
        Block &Push(DataType data, std::any value);
        Block &Ldc(TokenRef token);
        Block &Ldftn(TokenRef ftn);
        Block &Ldsftn(TokenRef sftn);
        Block &Ldvftn(TokenRef vftn);
        Block &Ldarg(DataType data);
        Block &Starg(DataType data);
        Block &Ldarga();
        Block &Ldloc(DataType data);
        Block &Ldloca();
        Block &Stloc(DataType data);
        Block &Ldfld(DataType data, TokenRef fld);
        Block &Ldsfld(DataType data, TokenRef sfld);
        Block &Ldflda(TokenRef fld);
        Block &Ldsflda(TokenRef sfld);
        Block &Stfld(DataType data, TokenRef fld);
        Block &Stsfld(DataType data, TokenRef sfld);
        Block &Ldelem(DataType data);
        Block &Ldelema();
        Block &Stelem(DataType data);
        Block &Ldnull();
        Block &Newobj(TokenRef cls);
        Block &Castcls(TokenRef src, TokenRef dst);
        Block &Conv(DataType src, DataType dst);
        Block &Callvirt();
        Block &Invoke(TokenRef external);
        Block &Callstatic();
        Block &Call();
    };


    template<typename T>
    void write(std::ostream &stream,const T t){
        stream.write((const char*)(&t),sizeof(t));
    }

    template<typename T>
    void read(std::istream &stream,T &t){
        stream.read((char*)&t,sizeof(T));
    }

    template<Bytecode>
    void write(std::ostream &stream,Bytecode code){
        write(stream,(data::u8)code);
    }

}

#endif //EVOBASIC_IL_H
