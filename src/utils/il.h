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
#include <list>
#include <variant>
#include "utils.h"
#include "context.h"

namespace evoBasic::il{
    class Class;
    class Interface;
    class Inst;
    class SFtn;
    class VFtn;
    class BasicBlock;
    class FunctionDeclare;
    class Node;
    class Member;
    class Document;
    class InterfaceFunction;


    class Node {
        Document *document = nullptr;
    protected:
        Bytecode begin_mark;
    public:
        Node(Document *document);
        Node(Document *document,Bytecode begin_mark);
        Node(Document *document,Bytecode begin_mark,std::istream &stream);
        Document *getDocument();
        virtual std::string toString()=0;
        virtual void toHex(std::ostream &stream);
        virtual DebugInfo *toStructuredInfo();
    };

    class Inst : public Node{
    public:
        Inst(Document *document,Bytecode begin_mark);
        Inst(Document *document,Bytecode begin_mark,std::istream &stream);
        using ByteSize = data::u64;
        virtual ByteSize getByteSize() = 0;
    };


    class TokenDef : public Node{
    public:
        using ID = data::u64;
    protected:
        ID id;
    public:
        TokenDef(Document *document,Bytecode begin_mark,ID id);
        TokenDef(Document *document,Bytecode begin_mark,std::istream &stream);
        virtual std::string getName()=0;
        virtual std::list<std::string> getFullName()=0;
        void toHex(std::ostream &stream)override;
        ID getID();
    };

    class TextTokenDef : public TokenDef{
        std::string text;
    public:
        TextTokenDef(Document *document,ID id,std::string text);
        TextTokenDef(Document *document,std::istream &stream);
        std::string toString()override;
        std::string getName()override;
        std::list<std::string> getFullName()override;
        void toHex(std::ostream &stream)override;
    };

    class ConstructedTokenDef : public TokenDef{
        std::list<TokenRef*> sub_token_list;
    public:
        ConstructedTokenDef(Document *document,ID id,std::list<TokenRef*> sub_tokens);
        ConstructedTokenDef(Document *document,std::istream &stream);
        std::string toString()override;
        std::string getName()override;
        std::list<std::string> getFullName()override;
        void toHex(std::ostream &stream)override;
    };

    class TokenRef : public Node{
        TokenDef::ID id = -1;
    public:
        using ID = data::u64;
        TokenRef(Document *document, data::u64 id);
        TokenRef(Document *document, std::istream &stream);
        void toHex(std::ostream &stream)override;
        std::string toString()override;
        TokenDef::ID getID();
        TokenDef *getDef();
        bool isEmpty();
    };

    class Member : public Node{
        AccessFlag access = AccessFlag::Private;
        TokenRef *name = nullptr;
    public:
        Member(Document *document,Bytecode begin_mark,AccessFlag access,TokenRef *name);
        Member(Document *document,Bytecode begin_mark,std::istream &stream);
        std::string toString()override;
        AccessFlag getAccessFlag();
        TokenRef *getNameToken();
        virtual type::Symbol *prepareSymbol()=0;
        virtual void fillSymbolDetail(Context *context)=0;
        void toHex(std::ostream &stream)override;
        ~Member();
    };

    class Class : public Member{
        TokenRef *extend_class = nullptr;
        std::list<TokenRef*> impl_interface_list;
        std::list<Member*> members;
        type::Class *symbol = nullptr;
    public:
        Class(Document *document,AccessFlag access,TokenRef *name,TokenRef *extend,std::list<TokenRef*> impl,std::list<Member*> members);
        Class(Document *document,std::istream &stream);
        std::string toString()override;
        DebugInfo *toStructuredInfo()override;
        void toHex(std::ostream &stream)override;
        type::Symbol *prepareSymbol()override;
        void fillSymbolDetail(Context *context)override;
        ~Class();
    };

    class Module : public Member{
        std::list<Member*> members;
        type::Module *symbol = nullptr;
    public:
        Module(Document *document,AccessFlag access,TokenRef *name,std::list<Member*> members);
        Module(Document *document,std::istream &stream);
        std::string toString()override;
        DebugInfo *toStructuredInfo()override;
        void toHex(std::ostream &stream)override;
        type::Symbol *prepareSymbol()override;
        void fillSymbolDetail(Context *context) override;
        ~Module();
    };

    class Interface : public Member{
        std::list<InterfaceFunction*> ftns;
        type::Interface *symbol = nullptr;
    public:
        Interface(Document *document,AccessFlag access,TokenRef *name,std::list<InterfaceFunction*> functions);
        Interface(Document *document,std::istream &stream);
        std::string toString()override;
        DebugInfo *toStructuredInfo()override;
        void toHex(std::ostream &stream)override;
        type::Symbol *prepareSymbol()override;
        void fillSymbolDetail(Context *context) override;
        ~Interface();
    };

    class Enum : public Member{
    public:
        using Value = data::u32;
        using Pair = std::pair<TokenRef*,Value>;
    private:
        std::list<Pair> enums;
        type::Enumeration *symbol = nullptr;
    public:
        Enum(Document *document,AccessFlag access,TokenRef *name,std::list<Pair> enums);
        Enum(Document *document,std::istream &stream);
        std::string toString()override;
        DebugInfo *toStructuredInfo()override;
        void toHex(std::ostream &stream)override;
        type::Symbol *prepareSymbol()override;
        void fillSymbolDetail(Context *context) override;
        ~Enum();
    };


    class Fld : public Member{
        TokenRef *type = nullptr;
        type::Variable *symbol = nullptr;
    public:
        Fld(Document *document,AccessFlag access,TokenRef *name,TokenRef *type);
        Fld(Document *document,std::istream &stream);
        std::string toString()override;
        DebugInfo *toStructuredInfo()override;
        void toHex(std::ostream &stream)override;
        type::Symbol *prepareSymbol()override;
        void fillSymbolDetail(Context *context) override;
        ~Fld();
    };

    class SFld : public Member{
        TokenRef *type = nullptr;
        type::Variable *symbol = nullptr;
    public:
        SFld(Document *document,AccessFlag access,TokenRef *name,TokenRef *type);
        SFld(Document *document,std::istream &stream);
        std::string toString()override;
        DebugInfo *toStructuredInfo()override;
        void toHex(std::ostream &stream)override;
        type::Symbol *prepareSymbol()override;
        void fillSymbolDetail(Context *context) override;
        ~SFld();
    };

    class Record : public Member{
        std::list<Fld*> fields;
        type::Record *symbol = nullptr;
    public:
        Record(Document *document,AccessFlag access,TokenRef *name,std::list<Fld*> fields);
        Record(Document *document,std::istream &stream);
        std::string toString()override;
        DebugInfo *toStructuredInfo()override;
        void toHex(std::ostream &stream)override;
        type::Symbol *prepareSymbol()override;
        void fillSymbolDetail(Context *context) override;
        ~Record();
    };

    enum class ParamKind {Regular,Opt,Inf};

    class Param : public Node{
        bool is_ref = false;
        TokenRef *name = nullptr,*type = nullptr;
    public:
        TokenRef *getNameToken();
        TokenRef *getTypeToken();
        virtual ParamKind getKind()=0;
        bool isRef();
        Param(Document *document,TokenRef *name,TokenRef *type,bool ref,Bytecode begin_mark);
        Param(Document *document,Bytecode begin_mark,std::istream &stream);
        void toHex(std::ostream &stream)override;
        std::string toString()override;
    };

    class Regular : public Param{
    public:
        ParamKind getKind(){return ParamKind::Regular;}
        Regular(Document *document,TokenRef *name,TokenRef *type,bool ref);
        Regular(Document *document,std::istream &stream);
    };

    class Opt : public Param{
        BasicBlock *initial = nullptr;
        data::u8 *initial_memory = nullptr;
        Inst::ByteSize initial_memory_size = 0;
    public:
        ParamKind getKind(){return ParamKind::Opt;}
        Opt(Document *document,TokenRef *name,TokenRef *type,bool ref,BasicBlock *initial);
        Opt(Document *document,std::istream &stream);
        std::string toString()override;
        DebugInfo *toStructuredInfo()override;
        void toHex(std::ostream &stream)override;
        ~Opt();
    };

    class Inf : public Param{
    public:
        ParamKind getKind(){return ParamKind::Inf;}
        Inf(Document *document,TokenRef *name,TokenRef *type,bool ref);
        Inf(Document *document,std::istream &stream);
        std::string toString()override;
    };

    class Local : public Node{
    public:
        using ID = data::u16;
    private:
        ID address = -1;
        TokenRef *name = nullptr,*type = nullptr;
    public:
        Local(Document *document,TokenRef *name,TokenRef *type,ID address);
        Local(Document *document,std::istream &stream,ID address);
        std::string toString()override;
        void toHex(std::ostream &stream)override;
    };

    class Result : public Node{
        TokenRef *type = nullptr;
    public:
        Result(Document *document,TokenRef *type);
        Result(Document *document,std::istream &stream);
        std::string toString()override;
        TokenRef *getTypeToken();
        void toHex(std::ostream &stream)override;
    };

    class FunctionDeclare : public Member{
        std::list<Param*> params;
        Result *result = nullptr;
    protected:
        void fillParameterList(Context *context,type::Function *symbol);
    public:
        FunctionDeclare(Document *document, Bytecode begin_mark, AccessFlag access, TokenRef *name, std::list<Param*> params, Result *result);
        FunctionDeclare(Document *document, Bytecode begin_mark, std::istream &stream);
        void toHex(std::ostream &stream)override;
        DebugInfo *toStructuredInfo()override;
    };

    class FunctionDefine : public FunctionDeclare{
        Inst::ByteSize block_byte_length = 0;
        data::Byte *blocks_memory = nullptr;
    protected:
        std::list<Local*> locals;
        std::list<BasicBlock*> blocks;
    public:
        FunctionDefine(Document *document, Bytecode begin_mark, AccessFlag access, TokenRef *name,
                       std::list<Param*> params, Result *result, std::list<Local*> locals, std::list<BasicBlock*> blocks);
        FunctionDefine(Document *document, Bytecode begin_mark, std::istream &stream);
        void toHex(std::ostream &stream)override;
        DebugInfo *toStructuredInfo()override;
        data::Byte *getBlocksMemory();
        ~FunctionDefine();
    };

    class Ctor : public FunctionDefine{
        type::Constructor *symbol = nullptr;
    public:
        Ctor(Document *document, std::list<Param*> params, std::list<Local*> locals, std::list<BasicBlock*> blocks);
        Ctor(Document *document, std::istream &stream);
        std::string toString()override;
        type::Symbol *prepareSymbol()override;
        void fillSymbolDetail(Context *context) override;
    };

    class Ftn : public FunctionDefine{
        type::UserFunction *symbol = nullptr;
    public:
        Ftn(Document *document, AccessFlag access, TokenRef *name,
            std::list<Param*> params, Result *result, std::list<Local*> locals, std::list<BasicBlock*> blocks);
        Ftn(Document *document, std::istream &stream);
        std::string toString()override;
        type::Symbol *prepareSymbol()override;
        void fillSymbolDetail(Context *context) override;
    };

    class VFtn : public FunctionDefine{
        type::UserFunction *symbol = nullptr;
    public:
        VFtn(Document *document, AccessFlag access, TokenRef *name,
             std::list<Param*> params, Result *result, std::list<Local*> locals, std::list<BasicBlock*> blocks);
        VFtn(Document *document, std::istream &stream);
        std::string toString()override;
        type::Symbol *prepareSymbol()override;
        void fillSymbolDetail(Context *context) override;
    };

    class SFtn : public FunctionDefine{
        type::UserFunction *symbol = nullptr;
    public:
        SFtn(Document *document, AccessFlag access, TokenRef *name,
             std::list<Param*> params, Result *result, std::list<Local*> locals, std::list<BasicBlock*> blocks);
        SFtn(Document *document, std::istream &stream);
        std::string toString()override;
        type::Symbol *prepareSymbol()override;
        void fillSymbolDetail(Context *context) override;
    };


    class Ext : public FunctionDeclare{
        TokenRef *lib = nullptr;
        TokenRef *alias = nullptr;
        type::ExternalFunction *symbol = nullptr;
    public:
        Ext(Document *document, AccessFlag access, TokenRef *name,TokenRef *library,TokenRef *alias,
            std::list<Param*> params, Result *result);
        Ext(Document *document, std::istream &stream);
        std::string toString()override;
        void toHex(std::ostream &stream)override;
        type::Symbol *prepareSymbol()override;
        void fillSymbolDetail(Context *context) override;
        ~Ext();
    };

    class InterfaceFunction : public FunctionDeclare{
        type::UserFunction *symbol = nullptr;
    public:
        InterfaceFunction(Document *document, TokenRef *name,std::list<Param*> params, Result *result);
        InterfaceFunction(Document *document, std::istream &stream);
        std::string toString()override;
        type::Symbol *prepareSymbol()override;
        void fillSymbolDetail(Context *context) override;
    };


    class InstWithOp : public Inst{
    public:
        enum Op{Nop,Ret,CallVirt,CallExt,Callstatic,Call,Ldnull,And,Or,Xor,Ldloca,Ldarga,Ldelema,Not}op;
    private:
        Bytecode opToBytecode(Op op);
    public:
        InstWithOp(Document *document,Op op);
        std::string toString()override;
        void toHex(std::ostream &stream)override;
        ByteSize getByteSize()override;
    };

    class InstWithToken : public Inst{
    public:
        enum Op{Ldftn,Ldsftn,Ldvftn,Ldc,Newobj,Invoke,Ldflda,Ldsflda}op;
    private:
        TokenRef *token = nullptr;
        Bytecode opToBytecode(Op op);
    public:
        InstWithToken(Document *document,Op op,TokenRef *token);
        std::string toString()override;
        ByteSize getByteSize()override;
        void toHex(std::ostream &stream)override;
    };

    class InstJif : public Inst{
        BasicBlock *target = nullptr;
    public:
        InstJif(Document *document, BasicBlock *target);
        std::string toString()override;
        ByteSize getByteSize()override;
        void toHex(std::ostream &stream)override;
    };

    struct InstBr : public Inst{
        BasicBlock *target = nullptr;
    public:
        InstBr(Document *document, BasicBlock *target);
        std::string toString()override;
        ByteSize getByteSize()override;
        void toHex(std::ostream &stream)override;
    };

    class InstPush : public Inst{
        DataType type;
        std::any value;
    public:
        InstPush(Document *document,DataType type,std::any value);
        std::string toString()override;
        ByteSize getByteSize()override;
        void toHex(std::ostream &stream)override;
    };

    class InstWithData : public Inst{
    public:
        enum Op{Ldelem,Stelem,Stelema,Ldarg,Starg,Ldloc,Stloc,
                Add,Sub,Mul,Div,FDiv,EQ,NE,LT,GT,LE,GE,Neg,Pop,Dup};
    private:
        Op op;
        DataType type;
        Bytecode opToBytecode(Op op);
    public:
        InstWithData(Document *document,Op op,DataType type);
        std::string toString()override;
        void toHex(std::ostream &stream)override;
        ByteSize getByteSize()override;
    };

    class InstWithDataToken : public Inst{
    public:
        enum Op{Ldfld,Ldsfld,Stfld,Stsfld};
    private:
        Op op;
        DataType type;
        TokenRef *token = nullptr;
        Bytecode opToBytecode(Op op);
    public:
        InstWithDataToken(Document *document,Op op, DataType type, TokenRef *token);
        std::string toString()override;
        ByteSize getByteSize()override;
        void toHex(std::ostream &stream)override;
    };

    class InstCastcls : public Inst{
        TokenRef *src_class = nullptr,*dst_class = nullptr;
    public:
        InstCastcls(Document *document,TokenRef *src_class,TokenRef *dst_class);
        std::string toString()override;
        ByteSize getByteSize()override;
        void toHex(std::ostream &stream)override;
    };


    class InstConv : public Inst{
        DataType src,dst;
    public:
        InstConv(Document *document,DataType src,DataType dst);
        std::string toString()override;
        ByteSize getByteSize()override;
        void toHex(std::ostream &stream)override;
    };


    class Document : public Node{
        friend TokenRef;

        std::map<std::string,int> token_pool_map;
        std::vector<TokenDef*> token_pool;

        std::vector<Member*> members;
        std::list<TokenRef*> dependencies_token;
        std::list<Document*> dependencies;

        std::list<Node*> resources;

        std::string name;
    public:
        explicit Document(std::string package_name);
        Document(std::string package_name,std::istream &stream);

        void addResource(Node* node);
        std::string toString()override;
        void toHex(std::ostream &stream)override;

        TokenRef *getTokenRef(data::u64 id);
        TokenRef *getTokenRef(std::string text);
        TokenRef *getTokenRef(std::list<std::string> full_name_list);

        TokenDef *findTokenDef(data::u64 id);
        TokenDef *findTokenDef(std::string text);

        void pushSymbolsInto(Context *context);
        void fillSymbolsDetail(Context *context);

        std::list<std::string> getDependenciesPath();

        void addDependenceLibrary(std::string name);

        void add(Member *member);

        SFtn *getEntrance();

        std::string getPackageName();
    };


    class BasicBlock : public Node{
        Inst::ByteSize address = -1;
        Inst::ByteSize size = -1;
        std::vector<Inst*> insts;
    public:
        Inst::ByteSize getAddress();
        Inst::ByteSize getByteSize();
        void setAddress(Inst::ByteSize value);

        std::string toString()override;
        DebugInfo *toStructuredInfo()override;
        void toHex(std::ostream &stream)override;

        BasicBlock(Document *document);

        BasicBlock &Br(BasicBlock *block);
        BasicBlock &Jif(BasicBlock *block);
        BasicBlock &EQ(DataType data);
        BasicBlock &NE(DataType data);
        BasicBlock &LT(DataType data);
        BasicBlock &GT(DataType data);
        BasicBlock &LE(DataType data);
        BasicBlock &GE(DataType data);
        BasicBlock &Add(DataType data);
        BasicBlock &Sub(DataType data);
        BasicBlock &Mul(DataType data);
        BasicBlock &Div(DataType data);
        BasicBlock &FDiv(DataType data);
        BasicBlock &Neg(DataType data);
        BasicBlock &And();
        BasicBlock &Or();
        BasicBlock &Xor();
        BasicBlock &Not();
        BasicBlock &Nop();
        BasicBlock &Pop(DataType data);
        BasicBlock &Dup(DataType data);
        BasicBlock &Ret();
        BasicBlock &Push(DataType data, std::any value);
        BasicBlock &Ldc(TokenRef *token);
        BasicBlock &Ldftn(TokenRef *ftn);
        BasicBlock &Ldsftn(TokenRef *sftn);
        BasicBlock &Ldvftn(TokenRef *vftn);
        BasicBlock &Ldarg(DataType data);
        BasicBlock &Starg(DataType data);
        BasicBlock &Ldarga();
        BasicBlock &Ldloc(DataType data);
        BasicBlock &Ldloca();
        BasicBlock &Stloc(DataType data);
        BasicBlock &Ldfld(DataType data, TokenRef *fld);
        BasicBlock &Ldsfld(DataType data, TokenRef *sfld);
        BasicBlock &Ldflda(TokenRef *fld);
        BasicBlock &Ldsflda(TokenRef *sfld);
        BasicBlock &Stfld(DataType data, TokenRef *fld);
        BasicBlock &Stsfld(DataType data, TokenRef *sfld);
        BasicBlock &Ldelem(DataType data);
        BasicBlock &Ldelema();
        BasicBlock &Stelem(DataType data);
        BasicBlock &Ldnull();
        BasicBlock &Newobj(TokenRef *cls);
        BasicBlock &Castcls(TokenRef *src, TokenRef *dst);
        BasicBlock &Conv(DataType src, DataType dst);
        BasicBlock &Callvirt();
        BasicBlock &Invoke(TokenRef *external);
        BasicBlock &Callstatic();
        BasicBlock &Call();
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
