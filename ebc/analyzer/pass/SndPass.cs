namespace evoBasic;
using Antlr4.Runtime.Misc;
using System.Collections.Generic;
using Parser = evoBasicParser;
using Symbols;
using Antlr4.Runtime;
using System.Text;



class SourceModuleCompleter : Compeleter{
    public override void Complete(object obj, CompileEnv env, Scope scope){
        var mod = obj as Module;
        foreach(var (_,child) in mod.scope.childern){
            child.completer.Complete(child,env,mod.scope);
        }
    }
}

class SourceClassCompleter : Compeleter{
    public Parser.ClassDeclContext context;
    public override void Complete(Object obj,CompileEnv env,Scope scope){
        var cls = obj as Class;
        if(cls!=env.EBObject){
            if(context.extend==null){
                cls.baseClass = env.EBObject;
            }
            else{
                cls.baseClass = Utils.SearchAnnotation(context.extend,scope,env) as Class;
            }
            cls.scope.AddExtendScope(cls.baseClass.scope);
        }

        if(context.implPart()!=null){
            foreach(var impl in context.implPart().annotation()){
                var itf = Utils.SearchAnnotation(impl,scope,env);
                if(itf is Interface){
                    cls.implInterfaces.Add(itf.getFullName(),itf as Interface);
                }
                else {
                    Logger.Add(Log.Error, impl, Formater.singleton.InvalidImpl(impl.GetText()));
                }
            }            
        }
        
        foreach(var (_,child) in cls.scope.childern){
            child.completer.Complete(child,env,cls.scope);
        }

        if(cls.ctor == null){ //如果没有定义类的构造函数，就生成一个
            cls.ctor = new Constructor() {
                location = new CompilerGeneratedLocation(),
                astBlock = new AST.Block(),
            };
            
            if(cls!=env.EBObject){
                cls.ctor.extendCtorAST = new AST.BaseCtorCall{
                    function = cls.baseClass.ctor
                };
            }
        }
        cls.ctor.ownerClass = cls;
        cls.ctor.retType = cls;
        cls.scope.Add(cls.ctor);

        if(cls!=env.EBObject) env.inheritDependent.AddEdge(cls.baseClass,cls);
    }
}

class SourceRecordCompleter : Compeleter{
    public override void Complete(object obj,CompileEnv env,Scope scope){
        foreach(var (_,child) in (obj as Record).scope.childern){
            child.completer.Complete(child,env,scope);
        }
    }
}

class SourceFunMethodCompleter : Compeleter{
    public Parser.FunctionDeclContext context;
    public override void Complete(object obj, CompileEnv env, Scope scope){
        var t = obj as Method;
        SourceFunMethodCompleter.CompleteFunctionSignature(context.parameterList().parameter(), context.retType, t, env, scope);

        if(t.retType == env.Void){
            //Function类型不能没有返回值
            t.retType = env.ErrorType;
            Logger.Add(Log.Error, context, Formater.singleton.MissingReturnTypeInFunction(t.name));
        }

        var flag = context.methodFlag();
        if(flag!=null){
            if(flag.Virtual()!=null)t.flag.Set(Flags.Virtual);
            else if(flag.Override()!=null)t.flag.Set(Flags.Override);
            else if(flag.Static()!=null)t.flag.Set(Flags.Static);
        }
        // 如果是Module中的方法，打上静态标志。
        if(t.owner.attch is Module)t.flag.Set(Flags.Static);

        t.parseTreeBlock = context.statement();
    }

    public static void CompleteFunctionSignature(Parser.ParameterContext[] parameters,Parser.AnnotationContext? ret,Function function,CompileEnv env,Scope scope){
        foreach(var param in parameters){
            var p = new Parameter(){
                name = param.ID().GetText(),
                type = Utils.SearchAnnotation(param.annotation(),scope,env)
                //initial = param.initial, todo: fill initial expression here
            };

            if(param.ParamArray()!=null) 
                p.flag.Set(Flags.ParamArray);

            if(param.Byval()!=null)
                p.flag.Set(Flags.Byval);
            else if(param.Byref()!=null)
                p.flag.Set(Flags.Byref);
            else 
                p.flag.Set(Flags.Byval); // 缺省时为Byval

            // 检查当参数类型为Any时，是否为Byref
            if (p.type == env.Any && !p.flag.Exists(Flags.Byref)) {
                Logger.Add(Log.Error, param.annotation(), Formater.singleton.TypeAnyMustByref());
            }

            if (param.Optional()!=null){
                p.flag.Set(Flags.Optional);
                function.optionMap.Add(p.name.ToLower(),p);
                function.optionParameters.Add(p);
            }
            else if(param.ParamArray()!=null){
                p.flag.Set(Flags.ParamArray);
                function.paramArray = p;
                if(p.flag.Exists(Flags.Byref)){
                    Logger.Add(Log.Error,Formater.singleton.ParamArrayAlwaysByVal());
                }
                if(!(p.type is Array array) || array.type != env.EBObject){
                    Logger.Add(Log.Error,Formater.singleton.ParamArrayMustBeObjectArray());
                }
            }
            else{
                function.normalParameters.Add(p);
            }
            function.scope.Add(p);// add parameter symbol to function local scope
        }

        if(ret!=null){
            function.retType = Utils.SearchAnnotation(ret,scope,env);
        }
        else{
            function.retType = env.Void;
        }
    }
}

class SourceSubMethodCompleter : Compeleter{
    public Parser.SubDeclContext context;
    public override void Complete(object obj, CompileEnv env, Scope scope){
        var t = obj as Method;
        SourceFunMethodCompleter.CompleteFunctionSignature(context.parameterList().parameter(), context.retType, t,env,scope);

        if(t.retType != env.Void){
            //Sub类型必须没有返回值
            t.retType = env.Void;
            Logger.Add(Log.Error, context, Formater.singleton.InvalidReturnTypeInSub(t.name));
        }

        var flag = context.methodFlag();
        if(flag!=null){
            if(flag.Virtual()!=null)t.flag.Set(Flags.Virtual);
            else if(flag.Override()!=null)t.flag.Set(Flags.Override);
            else if(flag.Static()!=null)t.flag.Set(Flags.Static);
        }
        // 如果是Module中的方法，打上静态标志。
        if(t.owner.attch is Module)t.flag.Set(Flags.Static);

        t.parseTreeBlock = context.statement();
    }
}

class SourceConstructorCompleter : Compeleter{
    public Parser.CtorDeclContext context;
    public override void Complete(object obj, CompileEnv env, Scope scope){
        var ctor = obj as Constructor;
        SourceFunMethodCompleter.CompleteFunctionSignature(context.parameterList().parameter(), null, ctor,env,scope);
        ctor.parseTreeBlock = context.statement();
        ctor.extendPartContext = context.extendPart();
    }
}


class SourceFunForeignEntryCompleter : Compeleter{
    public Parser.ExternalFuncContext context;
    public override void Complete(object obj, CompileEnv env, Scope scope){
        var t = obj as ForeignEntry;
        t.flag.Set(Flags.Static);
        SourceFunMethodCompleter.CompleteFunctionSignature(context.parameterList().parameter(), context.retType, t,env,scope);

        if (t.retType == env.Void){
            //Function类型不能没有返回值
            t.retType = env.ErrorType;
            Logger.Add(Log.Error, context, Formater.singleton.MissingReturnTypeInFunction(t.name));
        }


        if (context.Lib()!=null)t.lib = Utils.RemoveQuote(context.lib.Text);
        if(context.Alias()!=null)t.alias = Utils.RemoveQuote(context.alias.Text);
    }
}

class SourceSubForeignEntryCompleter : Compeleter{
    public Parser.ExternalSubContext context;
    public override void Complete(object obj, CompileEnv env, Scope scope){
        var t = obj as ForeignEntry;
        t.flag.Set(Flags.Static);
        SourceFunMethodCompleter.CompleteFunctionSignature(context.parameterList().parameter(), context.retType, t,env,scope);

        if (t.retType != env.Void){
            //Sub类型必须没有返回值
            t.retType = env.Void;
            Logger.Add(Log.Error, context, Formater.singleton.InvalidReturnTypeInSub(t.name));
        }

        if (context.Lib()!=null)t.lib = Utils.RemoveQuote(context.lib.Text);
        if(context.Alias()!=null)t.alias = Utils.RemoveQuote(context.alias.Text);
    }
}

class SourceVariableCompleter : Compeleter{
    public Parser.VariableDeclContext context;

    public override void Complete(object obj, CompileEnv env, Scope scope){
        var variable = obj as Variable;
        if(context.annotation()==null){
            Logger.Add(Log.Error, context.ID(), Formater.singleton.CannotInferVariableType(context.ID().GetText()));
            return;
        }
        if(context.initial!=null){
            Logger.Add(Log.Error, context.ID(), Formater.singleton.CannotInitVariableHere());
            return;
        }
        variable.type = Utils.SearchAnnotation(context.annotation(),scope,env);
    }
}

