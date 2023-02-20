namespace evoBasic;
using System.Text;
using evoBasic.Symbols;
using Antlr4.Runtime.Misc;
using System.Collections.Generic;
using Parser = evoBasicParser;

static class Utils{
    public static Type SearchAnnotation([NotNull] Parser.AnnotationContext context,[NotNull]Scope scope,[NotNull] CompileEnv env){
        Type ret;
        if(context.annotation()!=null){
            ret = SearchAnnotation(context.annotation(),scope,env);
        }
        else{
            ret = SearchTypePath(context.typePath(), scope, env);
        }

        if(context.subScriptSuffix()!=null){
            ret = new Symbols.Array(env){
                type = ret,
            };
        }
        return ret;
    }

    public static Type SearchTypePath(Parser.TypePathContext context, Scope scope, CompileEnv env){
        var path = new List<string>();
        foreach(var id in context.ID())path.Add(id.GetText());
        var ret = scope.LookUp(path) as Type;
        if(ret == null){
            Logger.Add(Log.Error, context, Formater.singleton.ObjectNotfound(context.ID().First().GetText()));
            return new ErrorType();
        }

        if(!scope.Reachable(env.global.scope,ret)){
            Logger.Add(Log.Error, context, Formater.singleton.TargetUnreachable(ret.getFullName()));
        }

        return ret;
    }

    public static Type SearchType(List<string> path,Scope scope,CompileEnv env){
        var ret = env.global.scope.LookUp(new List<string>(path)) as Type;
        if(ret == null){
            var fullname = path.First();
            for(int i=1;i<path.Count;i++)fullname = fullname + '.' + path[i];
            Logger.Add(Log.Error, Formater.singleton.ObjectNotfound(fullname));
            return new ErrorType();
        }

        if(!scope.Reachable(env.global.scope,ret)){
            Logger.Add(Log.Error, Formater.singleton.TargetUnreachable(ret.getFullName()));
        }

        return ret;
    }

    public static Type SearchToken(TokenDict.Token token, CompileEnv env){
        switch(token){
            case TokenDict.SingleToken singleToken: 
                return SearchType(singleToken.QualifiedNameList(),env.global.scope,env);
            case TokenDict.ConstructedToken constructed:
                return SearchType(constructed.QualifiedNameList(),env.global.scope,env);
            case TokenDict.ArrayToken arrayToken:
                return new Symbols.Array(env){
                    type = SearchToken(arrayToken.getTypeToken(),env)
                };
        }
        throw new ArgumentException();
    }

    public static string TextEscape(string str){
        StringBuilder sb = new StringBuilder();
        for(int i = 0; i<str.Length; i++){
            if(str[i]!='\\')sb.Append(str[i]);
            else{
                switch(str[i+1]){
                    case 'n': sb.Append('\n'); break;
                    case 'r': sb.Append('\r'); break;
                    case 't': sb.Append('\n'); break;
                    case '\\': sb.Append('\n'); break;
                    default: 
                        Logger.Add(Log.Error, Formater.singleton.UnkonwnEscapeSequences(str[i+1]));
                        break;
                }
                i++;
            }
        }
        return new String(sb.ToString());
    }

    public static string RemoveQuote(string str){
        return str.Substring(1,str.Length-2);
    }

    public static Scope GetSymbolScope(Symbol symbol) => symbol switch{
        Module mod => mod.scope,
        Array ary => ary.scope,
        Class cls => cls.scope,
        Record rcd => rcd.scope,
        Interface itf => itf.scope,
        Enumeration em => em.scope,
        Variable var => GetSymbolScope(var.type),
        _ => null
    };

}

interface IDumpObject{
    public DumpObject Dump();
}

class DumpObject{
    public void PrintIndent(int indent){
        Console.Write("    ");
        if(indent>0)PrintIndent(indent-1);
    }
    public virtual void Print(int indent=0){}
}

class DumpBlock : DumpObject{
    public string head;
    public List<DumpObject> childern = new List<DumpObject>();
    public override void Print(int indent=0){
        PrintIndent(indent);
        if(head=="" && childern.Count==0)return;
        Console.WriteLine($"{head}{{");
        foreach(var child in childern){
            child.Print(indent+1);
        }
        PrintIndent(indent);
        Console.WriteLine("}");
    }
}

class DumpPair : DumpObject{
    public string key,value;
    public override void Print(int indent=0){
        PrintIndent(indent);
        Console.WriteLine($"{key} : {value}"); 
    }
}

class DumpMark : DumpObject{
    public string mark;
    public override void Print(int indent=0){
        var strs = mark.Split("\n\r");
        foreach(var str in strs){
            PrintIndent(indent);
            Console.Write(str);
            Console.Write('\n');
        }
    }
}

class DunpMutiline : DumpObject{
    public string[] lines;
    public override void Print(int indent = 0){
        PrintIndent(indent);
        Console.WriteLine("----------------------------------");
        foreach(var line in lines){
            PrintIndent(indent);
            Console.Write("|");
            Console.WriteLine(line);
        }
        PrintIndent(indent);
        Console.WriteLine("----------------------------------");
    }
    public DunpMutiline(string text){
        lines = text.Replace('\r','|').Split("\n");
    }
}


class Vertex<T>{
    public T value;
    public bool valid = true;
    public Vertex(T t){
        value = t;
    }
    public List<Vertex<T>> dstVexList = new List<Vertex<T>>();
    public int inDegree = 0; 
    public void PointTo(Vertex<T> vex){
        dstVexList.Add(vex);
        vex.inDegree++;
    }
}

class TopologicalOrder<T>{
    List<Vertex<T>> ls = new List<Vertex<T>>();
    Dictionary<T,Vertex<T>> graph = new Dictionary<T, Vertex<T>>();

    public TopologicalOrder(){}

    // public List<List<T>> Circles(){

    // }

    public void AddEdge(T src,T dst){
        var vsrc = AddVertex(src);
        var vdst = AddVertex(dst);
        vsrc.dstVexList.Add(vdst);
        vdst.inDegree++;
    }

    public Vertex<T> AddVertex(T t){
        if(!graph.ContainsKey(t)) {
            var ret = new Vertex<T>(t);
            graph.Add(t,ret);
            return ret;
        }
        else{
            return graph[t];
        }
    }

    public bool Solve(){
        Queue<Vertex<T>> queue = new Queue<Vertex<T>>();
        foreach(var (_,vex) in graph)queue.Enqueue(vex);
        while(queue.Count>0){
            var vex = queue.Dequeue();
            if(vex.inDegree==0 && vex.valid){
                vex.valid = false;
                ls.Add(vex);
                foreach(var dst in vex.dstVexList){
                    dst.inDegree--;
                    queue.Enqueue(dst);
                }
            }
        }
        return Exists();
    }

    public bool Exists(){
        return ls.Count==graph.Count;
    }

    public List<T> Order(){
        return ls.ConvertAll(v => v.value);
    }
}
