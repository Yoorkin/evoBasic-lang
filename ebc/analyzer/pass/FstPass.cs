namespace evoBasic;
using Antlr4.Runtime.Misc;
using System.Collections.Generic;
using Parser = evoBasicParser;
using Symbols;
using Antlr4.Runtime;
using System.Text;


class FstPass : evoBasicBaseVisitor<Symbol>{
    CompileEnv env;
    Source source;
    Class currentClass;

    public void Traverse(CompileEnv env,Source source){
        this.env = env;
        this.source = source;
        VisitDocument(source.Document);
    }

    void AddDeclarationsTo(Scope scope, Parser.DeclarationContext[] decls){
        List<Symbol> ls = new List<Symbol>();
        foreach(var declaration in decls){
            if(declaration.declarationNoAccessFlag().dimDecl()!=null){
                //设置static或者const的标志
                Flag dimFlag = new Flag();
                if(declaration.declarationNoAccessFlag().dimDecl().Static()!=null){
                    dimFlag.Set(Flags.Static);
                }

                foreach(var sym in VisitDimDecl(declaration.declarationNoAccessFlag().dimDecl().variableDecl())){
                    FillAccessFlag(sym,declaration.accessFlag());
                    sym.flag.Set(dimFlag.GetValue());
                    ls.Add(sym);
                }
            }
            else{
                Symbol symbol = Visit(declaration.declarationNoAccessFlag()) as Symbol;
                FillAccessFlag(symbol,declaration.accessFlag());
                if(symbol!=null)ls.Add(symbol);
            }
        }
        foreach(var sym in ls){
            if(!scope.Add(sym)){
                Logger.Add(Log.Error, sym.location, Formater.singleton.NameDuplicate(sym.name));
            }
        }
    }

    public override Symbol VisitDocument([NotNull] evoBasicParser.DocumentContext context){
        AddDeclarationsTo(env.global.scope, context.declaration());
        return env.global;
    }

    public void FillAccessFlag(Symbol? symbol,Parser.AccessFlagContext? context){
        if(symbol!=null){
            if(context==null)symbol.flag.Set(Flags.Private);
            else if(context.Public()!=null) symbol.flag.Set(Flags.Public);
            else if(context.Private()!=null) symbol.flag.Set(Flags.Private);
        }
    }

    public override Symbol VisitImportDecl([NotNull] evoBasicParser.ImportDeclContext context){
        env.AddDependencies(Utils.RemoveQuote(context.StringLiteral().GetText()));
        return null;
    }

    public override Symbol VisitModuleDecl([NotNull] evoBasicParser.ModuleDeclContext context){
        Module mod = new Module{
            name = context.ID().GetText(),
            location = new Source.SourceLocation(context.ID()),
            completer = new SourceModuleCompleter()
        };
        AddDeclarationsTo(mod.scope, context.declaration());
        return mod;
    }

    public override Symbol VisitClassDecl([NotNull] evoBasicParser.ClassDeclContext context){
        Class cls = new Class{
            name = context.ID().GetText(),
            location = new Source.SourceLocation(context.ID()),
            completer = new SourceClassCompleter{
                context = context
            }
        };
        currentClass = cls;
        AddDeclarationsTo(cls.scope, context.declaration());
        currentClass = null;
        return cls;
    }

    public override Symbol VisitCtorDecl([NotNull] Parser.CtorDeclContext context){
        var ctor = new Constructor(){
            name = "#ctor",
            location = new Source.SourceLocation(context.New().First()),
            completer = new SourceConstructorCompleter{
                context = context
            }
        };
        currentClass.ctor = ctor;
        return ctor;
    }

    public override Symbol VisitEnumDecl([NotNull] Parser.EnumDeclContext context){
        Enumeration em = new Enumeration{
            name = context.ID().GetText(),
            location = new Source.SourceLocation(context.ID())
        };
        UInt32 value = 0;
        foreach(var constant in context.enumConstant()){
            if(constant.Digit()!=null)value = UInt32.Parse(constant.Digit().GetText());
            em.scope.Add(new EnumConstant{
                name = constant.ID().GetText(),
                value = value,
                location = new Source.SourceLocation(constant.ID()),
                flag = new Flag(Flags.Public)
            });
            value++;
        }
        return em;
    }

    public override Symbol VisitTypeDecl([NotNull] Parser.TypeDeclContext context){
        var rcd = new Record{
            name = context.ID().GetText(),
            location = new Source.SourceLocation(context.ID()),
            completer = new SourceRecordCompleter()
        };
        foreach(var varibale in VisitDimDecl(context.variableDecl())){
            varibale.flag.Set(Flags.Public);
            rcd.scope.Add(varibale);
        }
        return rcd;
    }

    public new List<Symbol> VisitDimDecl([NotNull] Parser.VariableDeclContext[] variableDecls){
        var variables = new List<Symbol>();
        foreach(var variableContext in variableDecls){
            variables.Add(new Variable{
                name = variableContext.ID().GetText(),
                location = new Source.SourceLocation(variableContext.ID()),
                completer = new SourceVariableCompleter{
                    context = variableContext
                }
            });
        }
        return variables;
    }
    public override Symbol VisitFunctionDecl([NotNull] Parser.FunctionDeclContext context){
        return new Method{
            name = context.ID().GetText(),
            location = new Source.SourceLocation(context.ID()),
            completer = new SourceFunMethodCompleter{
                context = context
            } 
        };
    }
    public override Symbol VisitSubDecl([NotNull] Parser.SubDeclContext context){
        return new Method{
            name = context.ID().GetText(),
            location = new Source.SourceLocation(context.ID()),
            completer = new SourceSubMethodCompleter{
                context = context
            }
        };
    }
    public override Symbol VisitExternalFunc([NotNull] Parser.ExternalFuncContext context){
        return new ForeignEntry{
            name = context.ID().GetText(),
            location = new Source.SourceLocation(context.ID()),
            completer = new SourceFunForeignEntryCompleter{
                context = context
            }
        };
    }
    public override Symbol VisitExternalSub([NotNull] Parser.ExternalSubContext context){
        return new ForeignEntry{
            name = context.ID().GetText(),
            location = new Source.SourceLocation(context.ID()),
            completer = new SourceSubForeignEntryCompleter{
                context = context
            }
        };
    }
}
