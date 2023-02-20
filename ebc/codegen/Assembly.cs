namespace evoBasic;
using System;
using System.Runtime.InteropServices;
using Newtonsoft.Json.Linq;
using Google.Protobuf;

using Symbols;
using System.Text;
using Handler = IntPtr;
using uint32_t = UInt32;
using Google.Protobuf.Collections;
using Backage;

class AssemblyLocation : Location{
    Package package;
    public override string source() => $"package {package.path}";
    public override string position() => "";
    public AssemblyLocation(Package package){
        this.package = package;
    }
}

class TokenDict{
    Dictionary<string,Token> data = new Dictionary<string, Token>();
    List<Token> list = new List<Token>();
    
    public IEnumerable<Token> GetAllTokens() => list;

    public void Add(Token token){
        var key = token.QualifiedName().ToLower();
        if(!data.ContainsKey(key)){
            data.Add(key,token);
            list.Add(token);
        }
    }

    public Token QueryToken(UInt32 index){
        return list[(int)index-1];
    }

    public Token QueryToken(string str){
        var target = str.ToLower();
        if(!data.ContainsKey(target)){
            var token = new SingleToken((UInt32)data.Count()+1, str);
            Add(token);
        }
        return data[target];
    }

    public Token QueryToken(Symbol symbol){
        var key = symbol.getFullName().ToLower();
        if(!data.ContainsKey(key)){
            Token token;
            if(symbol is Symbols.Array array){
                var typeToken = QueryToken(array.type).id;
                token = new ArrayToken(this, (UInt32)data.Count()+1, typeToken);
            }
            else{
                var fullnameList = symbol.getFullNameList();
                if(fullnameList.Count == 1){
                    token = new SingleToken((UInt32)data.Count()+1,fullnameList.First());
                }
                else{
                    var ls = new List<UInt32>();
                    foreach(var sub in symbol.getFullNameList()){
                        ls.Add(QueryToken(sub).id);
                    }
                    token = new ConstructedToken(this, (UInt32)data.Count()+1, ls);                        
                }
            }
            Add(token);
        }
        return data[key];
    }

    public abstract class Token{
        public UInt32 id;
        public abstract string QualifiedName();
        public abstract List<string> QualifiedNameList();
    }

    public class SingleToken : Token{
        string text;
        public override string QualifiedName(){
            return text;
        }
        public override List<string> QualifiedNameList(){
            var ret = new List<string>();
            ret.Add(text);
            return ret;
        }
        public SingleToken(UInt32 id, string text){
            this.id = id;
            this.text = text;
        }
    }

    public class ConstructedToken : Token{
        TokenDict dict;
        List<UInt32> subTokens;

        public IEnumerable<UInt32> GetAllSubToken() => subTokens;

        public override string QualifiedName(){
            var ls = QualifiedNameList();
            var name = ls.First();
            for(int i=1;i<ls.Count;i++){
                name = name + '.' + ls[i];
            }
            return name;
        }

        public override List<string> QualifiedNameList(){
            var ret = new List<string>();
            foreach(var id in subTokens){
                ret.Add(dict.QueryToken(id).QualifiedName());
            }
            return ret;
        }

        public ConstructedToken(TokenDict dict, UInt32 id, List<UInt32> subTokens){
            this.dict = dict;
            this.id = id;
            this.subTokens = subTokens;
        }
    }

    public class ArrayToken : Token{
        TokenDict dict;
        UInt32 typeToken;

        public Token getTypeToken() => dict.QueryToken(typeToken);

        public override string QualifiedName(){
            var ret = dict.QueryToken(typeToken).QualifiedName();
            ret+=("[]");
            return ret;
        }

        public override List<string> QualifiedNameList(){
            var ret = dict.QueryToken(typeToken).QualifiedNameList();
            ret.Add("[]");
            return ret;
        }

        public ArrayToken(TokenDict dict, UInt32 id, UInt32 typeToken){
            this.id = id;
            this.dict = dict;
            this.typeToken = typeToken;
        }
    }

}



class Package{
    public string path;
    public List<string> Denpendencies = new List<string>();
    Backage.Package internalData;
    TokenDict dict;

    public override string ToString(){
        return JToken.Parse(internalData.ToString()).ToString(Newtonsoft.Json.Formatting.Indented);
    }

    public string getName(){
        return Path.GetFileNameWithoutExtension(path);
    }

    public static Package FromFile(CompileEnv env, string path){
        Package warpper = new Package(){
            path = path,
            dict = env.tokenDict
        };
        var file = System.IO.File.OpenRead(path);
        Backage.Package package = Backage.Package.Parser.ParseFrom(file);

        foreach(var dependency in package.Dependencies){
            warpper.Denpendencies.Add(dependency.Text);
        }

        foreach(var token in package.Tokens){
            switch(token.DefinitionCase){
                case TokenDefinition.DefinitionOneofCase.TextToken:
                    warpper.dict.Add(new TokenDict.SingleToken(token.Id, token.TextToken.Text));
                    break;
                case TokenDefinition.DefinitionOneofCase.ConstructedToken:
                    var ls = new List<UInt32>();
                    foreach(var sub in token.ConstructedToken.Tokens){
                        ls.Add(sub);
                    }
                    warpper.dict.Add(new TokenDict.ConstructedToken(warpper.dict, token.Id, ls));
                    break;
                case TokenDefinition.DefinitionOneofCase.ArrayToken:
                    warpper.dict.Add(new TokenDict.ArrayToken(warpper.dict, token.Id, token.ArrayToken.TypeToken));
                    break;
            }
        }

        Stack<Scope> scope = new Stack<Scope>();
        scope.Push(env.global.scope);
        warpper.VisitDecls(env,scope,package.Declarations);
        return warpper;
    }

    void VisitDecls(CompileEnv env,Stack<Scope> scope,RepeatedField<Declaration> decls){
        foreach(var decl in decls){
            switch(decl.MemberCase){
                case Backage.Declaration.MemberOneofCase.ClassDecl:
                    VisitClassDecl(env,scope,decl.ClassDecl);
                    break;
                case Backage.Declaration.MemberOneofCase.RecordDecl:
                    VisitRecordDecl(env,scope,decl.RecordDecl);
                    break;
                case Backage.Declaration.MemberOneofCase.EnumDecl:
                    VisitEnumDecl(env,scope,decl.EnumDecl);
                    break;
                case Backage.Declaration.MemberOneofCase.VaraibleDecl:
                    VisitVariableDecl(env,scope,decl.VaraibleDecl);
                    break;
                case Backage.Declaration.MemberOneofCase.ModuleDecl:
                    VisitModuleDecl(env,scope,decl.ModuleDecl);
                    break;
                case Backage.Declaration.MemberOneofCase.MethodDecl:
                    VisitMethodDecl(env,scope,decl.MethodDecl);
                    break;
                case Backage.Declaration.MemberOneofCase.CtorDecl:
                    VisitCtorDecl(env,scope,decl.CtorDecl);
                    break;
                case Backage.Declaration.MemberOneofCase.ForeignEntryDecl:
                    VisitForeignEntryDecl(env,scope,decl.ForeignEntryDecl);
                    break;
                case Declaration.MemberOneofCase.EnumConstant:
                    VisitEnumConstantDecl(env,scope,decl.EnumConstant);
                    break;
            }
        }
    }

    void VisitClassDecl(CompileEnv env,Stack<Scope> scope,ClassDecl decl){
        var ret = new Class{
            flag = new Flag(decl.Flag),
            name = dict.QueryToken(decl.NameToken).QualifiedName(),
            location = new AssemblyLocation(this),
            completer = new ClassCompleter{
                decl = decl,
                package = this
            }
        };
        scope.Peek().Add(ret);
        scope.Push(ret.scope);
        VisitDecls(env,scope,decl.Declarations);
        scope.Pop();
    }

    void VisitModuleDecl(CompileEnv env,Stack<Scope> scope,ModuleDecl decl){
        var ret = new Module{
            flag = new Flag(decl.Flag),
            name = dict.QueryToken(decl.NameToken).QualifiedName(),
            location = new AssemblyLocation(this),
            completer = new ModuleCompleter{
                decl = decl,
                package = this
            }
        };
        scope.Peek().Add(ret);
        scope.Push(ret.scope);
        VisitDecls(env,scope,decl.Declarations);
        scope.Pop();
    }

    void VisitEnumDecl(CompileEnv env,Stack<Scope> scope,EnumDecl decl){
        var ret = new Enumeration{
            flag = new Flag(decl.Flag),
            name = dict.QueryToken(decl.NameToken).QualifiedName(),
            location = new AssemblyLocation(this),
        };
        scope.Peek().Add(ret);
        scope.Push(ret.scope);
        foreach(var c in decl.Constants){
            VisitEnumConstantDecl(env,scope,c);
        }
        scope.Pop();
    }

    void VisitEnumConstantDecl(CompileEnv env,Stack<Scope> scope,Backage.EnumConstant decl){
        scope.Peek().Add(new Symbols.EnumConstant{
            flag = new Flag(Flags.Public),
            name = dict.QueryToken(decl.NameToken).QualifiedName(),
            location = new AssemblyLocation(this),
            value = decl.Value,
        });
    }

    void VisitVariableDecl(CompileEnv env,Stack<Scope> scope,VariableDecl decl){
        scope.Peek().Add(new Variable{
            flag = new Flag(decl.Flag),
            name = dict.QueryToken(decl.NameToken).QualifiedName(),
            location = new AssemblyLocation(this),
            completer = new VariableCompleter{
                decl = decl,
                package = this
            }
        });
    }

    void VisitRecordDecl(CompileEnv env,Stack<Scope> scope,RecordDecl decl){
        var ret = new Record{
            flag = new Flag(decl.Flag),
            name = dict.QueryToken(decl.NameToken).QualifiedName(),
            location = new AssemblyLocation(this),
            completer = new RecordCompleter{
                decl = decl,
                package = this
            }
        };
        scope.Peek().Add(ret);
        scope.Push(ret.scope);
        VisitDecls(env,scope,decl.Declarations);
        scope.Pop();
    }

    void VisitMethodDecl(CompileEnv env,Stack<Scope> scope,MethodDecl decl){
        var ret = new Method{
            flag = new Flag(decl.Flag),
            name = dict.QueryToken(decl.NameToken).QualifiedName(),
            location = new AssemblyLocation(this),
            completer = new MethodCompleter{
                decl = decl,
                package = this
            }
        };
        scope.Peek().Add(ret);
    }

    void VisitCtorDecl(CompileEnv env,Stack<Scope> scope,CtorDecl decl){
        scope.Peek().Add(new Constructor{
            flag = new Flag(decl.Flag),
            location = new AssemblyLocation(this),
            completer = new CtorCompleter{
                decl = decl,
                package = this
            }
        });
    }

    void VisitForeignEntryDecl(CompileEnv env,Stack<Scope> scope,ForeignEntryDecl decl){
        scope.Peek().Add(new ForeignEntry{
            flag = new Flag(decl.Flag),
            name = dict.QueryToken(decl.NameToken).QualifiedName(),
            location = new AssemblyLocation(this),
            completer = new ForeignEntryCompleter{
                decl = decl,
                package = this
            }
        });
    }

    class BinaryCompleter : Compeleter{
        public Package package;
    }

    class ModuleCompleter : BinaryCompleter{
        public Backage.ModuleDecl decl;
        public override void Complete(object obj, CompileEnv env, Scope scope){
            var mod = obj as Module;
            foreach(var (_,child) in mod.scope.childern){
                child.completer.Complete(child,env,mod.scope);
            }
        }
    }

    class ClassCompleter : BinaryCompleter{
        public ClassDecl decl;
        public override void Complete(object obj, CompileEnv env, Scope scope){
            var cls = obj as Class;
            if (cls.name.ToLower() != "object"){
                cls.baseClass = Utils.SearchToken(package.dict.QueryToken(decl.BaseToken), env) as Class;
                cls.scope.AddExtendScope(cls.baseClass.scope);
            }
            else{
                cls.baseClass = null;
            }
            
            cls.ctor = cls.scope.Find("#ctor") as Symbols.Constructor;

            foreach(var token in decl.ImplToken){
                var itf = Utils.SearchToken(package.dict.QueryToken(token),env) as Interface;
                cls.implInterfaces.Add(itf.getFullName().ToLower(),itf);
            }

            foreach(var (_,child) in cls.scope.childern){
                child.completer.Complete(child,env,cls.scope);
            }
        }
    }

    class RecordCompleter : BinaryCompleter{
        public RecordDecl decl;
        public override void Complete(object obj, CompileEnv env, Scope scope){
            var record = obj as Record;
            foreach(var (_,child) in record.scope.childern){
                child.completer.Complete(child,env,record.scope);
            }
        }
    }

    class VariableCompleter : BinaryCompleter{
        public VariableDecl decl;
        public override void Complete(object obj, CompileEnv env, Scope scope){
            var variable = obj as Variable;
            variable.type = Utils.SearchToken(package.dict.QueryToken(decl.TypeToken),env);
        }
    }

    public void CompleteFunctionSignatire(Function function,RepeatedField<ParameterDecl> parameters,uint32_t retToken,CompileEnv env){
        foreach(var param in parameters){
            var flag = new Flag(param.Flag);
            var paramSym = new Parameter{
                flag = flag,
                name = dict.QueryToken(param.NameToken).QualifiedNameList().Last(),
                type = Utils.SearchToken(dict.QueryToken(param.TypeToken), env)
            };

            if (flag.Exists(Flags.ParamArray)){
                function.paramArray = paramSym;
            }
            else if (flag.Exists(Flags.Optional)){
                function.optionMap.Add(paramSym.name.ToLower(), paramSym);
                function.optionParameters.Add(paramSym);
            }
            else{
                function.normalParameters.Add(paramSym);
            }
            function.scope.Add(paramSym);
        }

        if(retToken==0) function.retType = env.Void;
        else function.retType = Utils.SearchToken(dict.QueryToken(retToken),env);
    }

    class MethodCompleter : BinaryCompleter{
        public MethodDecl decl;
        public override void Complete(object obj, CompileEnv env, Scope scope){
            var method = obj as Method;
            package.CompleteFunctionSignatire(method, decl.Params, decl.RetTypeToken, env);
        }
    }

    class ForeignEntryCompleter : BinaryCompleter{
        public ForeignEntryDecl decl;
        public override void Complete(object obj, CompileEnv env, Scope scope){
            var fe = obj as ForeignEntry;
            package.CompleteFunctionSignatire(fe, decl.Params, decl.RetTypeToken, env);
            if(decl.HasLib) fe.lib = decl.Lib.ToString();
            if(decl.HasAlias) fe.alias = decl.Alias.ToString();
        }
    }

    class CtorCompleter : BinaryCompleter{
        public CtorDecl decl;
        public override void Complete(object obj, CompileEnv env, Scope scope){
            var ctor = obj as Constructor;
            package.CompleteFunctionSignatire(ctor, decl.Params, 0, env);
        }
    }

    public void SaveTo(string path){
        using (var file = File.Create(path)){
            internalData.WriteTo(file);
        }
    }




    public static Package FromCompileEnv(CompileEnv env){
        Package warpper = new Package{
            dict = env.tokenDict
        };

        Backage.Package package = new Backage.Package {
            Identity = env.packageName,
            Version = env.packageVerison
        };

        warpper.internalData = package;
        foreach(var depend in env.GetDependencies()){
            var d = new Backage.Dependency();
            d.Text = Encoding.UTF8.GetString(Encoding.Default.GetBytes(depend.getName()));
            package.Dependencies.Add(d);
        }

        foreach(var (_,child) in env.global.scope.childern){
            if(child.location is Source.SourceLocation)
                package.Declarations.Add(VisitSymbol(env,warpper.dict,child));
        }

        foreach(var token in warpper.dict.GetAllTokens()){
            var def = new Backage.TokenDefinition();
            def.Id = token.id;
            if(token is TokenDict.SingleToken singleToken){
                var tok = new Backage.TextToken {
                    Text = Encoding.UTF8.GetString(Encoding.Default.GetBytes(singleToken.QualifiedName()))
                };
                def.TextToken = tok;
            }
            else if(token is TokenDict.ConstructedToken conToken){
                var tok = new Backage.ConstructedToken();
                foreach(var id in conToken.GetAllSubToken()){
                    tok.Tokens.Add(id);
                }
                def.ConstructedToken = tok;
            }
            else if(token is TokenDict.ArrayToken subToken){
                var tok = new Backage.ArrayToken();
                tok.TypeToken = subToken.getTypeToken().id;
                def.ArrayToken = tok;
            }
            package.Tokens.Add(def);
        }

        return warpper;
    }

    static Backage.Declaration VisitSymbol(CompileEnv env, TokenDict dict, Symbols.Symbol sym){
        var decl = new Backage.Declaration();
        switch(sym){
            case Module mod: decl.ModuleDecl = VisitModuleSym(env,dict,mod); break;
            case Class cls:  decl.ClassDecl = VisitClassSym(env,dict,cls); break;
            case Record rcd: decl.RecordDecl = VisitRecordSym(env,dict,rcd); break;
            case Enumeration em: decl.EnumDecl = VisitEnumSym(env,dict,em); break;
            case Symbols.EnumConstant ec: decl.EnumConstant = VisitEnumConstantSym(env,dict,ec); break;
            case Method method: decl.MethodDecl = VisitMethodSym(env,dict,method); break;
            case Constructor ctor: decl.CtorDecl = VisitCtorSym(env,dict,ctor); break;
            case ForeignEntry fe: decl.ForeignEntryDecl = VisitForeignEntrySym(env,dict,fe); break;
            case Variable variable: decl.VaraibleDecl = VisitVaribleSym(env,dict,variable); break;
        }
        return decl;
    }

    static ClassDecl VisitClassSym(CompileEnv env, TokenDict dict, Symbols.Class cls){
        var ret = new Backage.ClassDecl(){
            NameToken = dict.QueryToken(cls.name).id,
            BaseToken = env.EBObject == cls ? 0 : dict.QueryToken(cls.baseClass).id,
            Flag = (UInt32)cls.flag.GetValue()
        };

        foreach(var (_,impl) in cls.implInterfaces){
            ret.ImplToken.Add(dict.QueryToken(impl).id);
        }

        foreach(var (_,child) in cls.scope.childern){
            ret.Declarations.Add(VisitSymbol(env,dict,child));
        }

        return ret;
    }

    static ModuleDecl VisitModuleSym(CompileEnv env, TokenDict dict, Symbols.Module mod){
        var ret = new Backage.ModuleDecl{
            NameToken = dict.QueryToken(mod.name).id,
            Flag = (UInt32)mod.flag.GetValue()
        };

        foreach(var (_,child) in mod.scope.childern){
            if(child.location is Source.SourceLocation)
                ret.Declarations.Add(VisitSymbol(env,dict,child));
        }

        return ret;
    }

    static RecordDecl VisitRecordSym(CompileEnv env, TokenDict dict, Symbols.Record rcd){
        var ret = new Backage.RecordDecl{
            NameToken = dict.QueryToken(rcd.name).id,
            Flag = (UInt32)rcd.flag.GetValue()
        };
        foreach(var (_,child) in rcd.scope.childern){
            ret.Declarations.Add(VisitSymbol(env,dict,child));
        }
        return ret;
    }

    static EnumDecl VisitEnumSym(CompileEnv env, TokenDict dict, Symbols.Enumeration em){
        var ret = new Backage.EnumDecl{
            NameToken = dict.QueryToken(em.name).id,
            Flag = (UInt32)em.flag.GetValue()
        };

        foreach(var (_,child) in em.scope.childern){
            ret.Constants.Add(VisitEnumConstantSym(env,dict,child as Symbols.EnumConstant));
        }

        return ret;
    }

    static Backage.EnumConstant VisitEnumConstantSym(CompileEnv env, TokenDict dict, Symbols.EnumConstant emConst){
        return new Backage.EnumConstant{
            NameToken = dict.QueryToken(emConst.name).id,
            Value = emConst.value
        };
    }

    static ParameterDecl VisitParameter(CompileEnv env, TokenDict dict, Symbols.Parameter param){
        return new Backage.ParameterDecl{
            Flag = (UInt32)param.flag.GetValue(),
            NameToken = param.flag.Exists(Flags.Optional) ? dict.QueryToken(param).id : dict.QueryToken(param.name).id,
            TypeToken = dict.QueryToken(param.type).id
        };
    }

    static void FillParameterDecl(CompileEnv env, TokenDict dict, Symbols.Function function, RepeatedField<ParameterDecl> decl){
        foreach(var param in function.normalParameters){
            decl.Add(VisitParameter(env,dict,param));
        }

        foreach(var param in function.optionParameters){
            decl.Add(VisitParameter(env,dict,param));
        }

        if(function.paramArray!=null) 
            decl.Add(VisitParameter(env,dict,function.paramArray));
    } 

    static void FillLineNumbers(List<IL.Blocks.LineNumber> numbers, RepeatedField<Backage.LineNumber> decl){
        foreach(var n in numbers){
            decl.Add(new LineNumber {
                Offset = n.offset,
                Line = n.line
            });
        }
    }

    static void FillExceptionEntries(List<IL.Blocks.ExceptionEntry> entries, RepeatedField<Backage.ExceptionEntry> decl) {
        foreach (var e in entries) {
            decl.Add(new ExceptionEntry {
                Offset = e.offset,
                Length = e.length,
                Target = e.target,
                TypeToken = e.typeToken.id
            });
        }
    }

    static MethodDecl VisitMethodSym(CompileEnv env, TokenDict dict, Symbols.Method method){
        var ret = new Backage.MethodDecl{
            NameToken = dict.QueryToken(method.name).id,
            Flag = (UInt32)method.flag.GetValue(),
            RetTypeToken = dict.QueryToken(method.retType).id,
            Block = Google.Protobuf.ByteString.CopyFrom(method.bytecodes)
        };

        FillLineNumbers(method.ilBlock.getLineNumbers(), ret.LineNumbers);
        FillExceptionEntries(method.ilBlock.getExceptionEntries(), ret.ExceptionEntry);

        FillParameterDecl(env,dict,method,ret.Params);

        foreach(var local in method.localIndex){
            ret.LocalIndex.Add(new Backage.LocalIndex(){
                Id = local.Item1.getIndex(),
                TypeToken = local.Item2
            });
        }

        return ret;
    }

    static CtorDecl VisitCtorSym(CompileEnv env, TokenDict dict, Symbols.Constructor ctor){
        var ret = new Backage.CtorDecl{
            Flag = (UInt32)ctor.flag.GetValue(),
            Block = Google.Protobuf.ByteString.CopyFrom(ctor.bytecodes)
        };
        
        FillLineNumbers(ctor.ilBlock.getLineNumbers(), ret.LineNumbers);
        FillExceptionEntries(ctor.ilBlock.getExceptionEntries(), ret.ExceptionEntry);

        FillParameterDecl(env, dict, ctor, ret.Params);

        foreach (var local in ctor.localIndex){
            ret.LocalIndex.Add(new Backage.LocalIndex(){
                Id = local.Item1.getIndex(),
                TypeToken = local.Item2
            });
        }

        return ret;
    }

    static ForeignEntryDecl VisitForeignEntrySym(CompileEnv env, TokenDict dict, Symbols.ForeignEntry fe){
        var ret = new Backage.ForeignEntryDecl{
            Flag = (UInt32)fe.flag.GetValue(),
            NameToken = dict.QueryToken(fe.name).id,
            RetTypeToken = dict.QueryToken(fe.retType).id
        };

        if(fe.lib!=null)ret.Lib = Google.Protobuf.ByteString.CopyFrom(Encoding.UTF8.GetBytes(fe.lib));
        if(fe.alias!=null)ret.Alias = Google.Protobuf.ByteString.CopyFrom(Encoding.UTF8.GetBytes(fe.alias));

        FillParameterDecl(env,dict,fe,ret.Params);

        return ret;
    }

    static VariableDecl VisitVaribleSym(CompileEnv env, TokenDict dict, Symbols.Variable variable){
        return new Backage.VariableDecl{
            Flag = (UInt32)variable.flag.GetValue(),
            NameToken = dict.QueryToken(variable.name).id,
            TypeToken = dict.QueryToken(variable.type).id
        };
    }

}
