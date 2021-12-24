//
// Created by yorkin on 12/13/21.
//

#ifndef EVOBASIC_ILGEN_H
#define EVOBASIC_ILGEN_H
#include <any>
#include <semantic.h>
#include "context.h"
#include "ast.h"
#include "il.h"
#include "visitor.h"
#include "defaultVisitor.h"

namespace evoBasic{
    
    struct ILGenArgs{
        type::Symbol *dot_expression_context = nullptr;
        bool need_lookup = false;
        Context *context = nullptr;
        il::FtnWithDefinition *ftn = nullptr;
        il::Block *previous_block = nullptr,
                  *next_block = nullptr;
    };

#define Visit(RETURN,AST,VAR,...) RETURN visit##AST(ast::AST *VAR##_node,##__VA_ARGS__);
    class ILGen{
        il::ILFactory *factory = nullptr;
        il::Block *for_next = nullptr,*loop_next = nullptr;
    public:
        Visit(il::IL*,Global,global)
        Visit(il::Class*,Class,class)
        Visit(il::Module*,Module,module)
        Visit(il::Interface*,Interface,interface)
        Visit(il::Record*,Type,type)
        Visit(il::Enum*,Enum,enum)

        Visit(il::Ftn*,Function,function)
        Visit(il::Ext*,External,external)
        Visit(il::Ctor*,Constructor,ctor)

        Visit(void,Let,let,               il::Block *current,il::Block *next)
        Visit(il::Block*,Select,select,         il::Block *current,il::Block *next)
        Visit(il::Block*,Loop,loop,             il::Block *current,il::Block *next)
        Visit(il::Block*,If,if,                 il::Block *current,il::Block *next)
        Visit(void,Case,case,             il::Block *current,il::Block *next)
        Visit(il::Block*,For,for,               il::Block *current,il::Block *next)
        Visit(il::Block*,ExprStmt,expr_stmt,    il::Block *current,il::Block *next)
        Visit(void,Return,return,         il::Block *current,il::Block *next)
        Visit(il::Block*,Exit,exit,             il::Block *current)
        Visit(il::Block*,Continue,continue,     il::Block *current,il::Block *next)
        Visit(il::Block*,Statement,statement,   il::Block *current,il::Block *next)

        Visit(void,Expression,expression,   il::Block *current)
        Visit(void,Unary,unary,             il::Block *current)
        Visit(void,Binary,binary,           il::Block *current)
        Visit(void,Assign,assign,           il::Block *current)
        Visit(void,Cast,cast,               il::Block *current)
        Visit(void,Parentheses,parentheses, il::Block *current)
        Visit(void,ArrayElement,element,    il::Block *current)
        Visit(void,Delegate,delegate,       il::Block *current)
        Visit(void,Argument,argument,       il::Block *current)
        Visit(void,New,new,                 il::Block *current)
        Visit(void,FtnCall,ftn,             il::Block *current)
        Visit(void,SFtnCall,sftn,           il::Block *current)
        Visit(void,VFtnCall,vftn,           il::Block *current)
        Visit(void,ExtCall,ext,             il::Block *current)
        Visit(void,SFld,sfld,               il::Block *current)
        Visit(void,Fld,fld,                 il::Block *current)
        Visit(void,Local,local,             il::Block *current)
        Visit(void,Arg,arg,                 il::Block *current)
        Visit(void,Digit,digit,             il::Block *current)
        Visit(void,Decimal,decimal,         il::Block *current)
        Visit(void,String,string,           il::Block *current)
        Visit(void,Char,char,               il::Block *current)
        Visit(void,Boolean,boolean,         il::Block *current)

        void loadCalleeArguments(ast::Call *call, il::Block *current);

        std::vector<il::Member*> visitMember(ast::Member *member);

        il::SFld *visitStaticField(ast::Variable *variable_node);

        il::Fld *visitField(ast::Variable *variable_node);

        std::vector<il::Param *> visitParameter(std::vector<type::Parameter *> parameters);


    };
#undef Visit
}


#endif //EVOBASIC_ILGEN_H
