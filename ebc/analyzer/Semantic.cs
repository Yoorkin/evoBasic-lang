
namespace evoBasic;

using System.Collections.Generic;
using Symbols;
using Antlr4.Runtime;
using System.IO;

class CompileEnv{
    public String packagePath, packageName;
    public UInt64 packageVerison;

    public readonly Type ErrorType,Void,Any,Boolean,Short,Integer,Long,Byte,UShort,UInteger,ULong,Single,Double,Rune,Ptr;
    public readonly Type EBNothing = new Symbols.Nothing();
    public Class EBObject,EBString,EBArray;

    public ConversionRule conversion;
    public Promotion promotion;
    public ConversionRule truncation;
    public ConversionRule widening;
    public ConversionRule floatingPointConvertion;
    public Boxing boxing;

    public ForeignEntry getArrayLength;

    public TokenDict tokenDict = new TokenDict();

    public Module global;
    

    public CompileEnv(String outputName, UInt64 version, bool complileBuiltinPackage,string packageOrSourcePath){
        this.packageName = outputName;
        this.packageVerison = version;

        ErrorType = new ErrorType();
        Void = new Primitive(PrimitiveKind.Void){name = "#Void"};
        Any = new Symbols.Any();
        Boolean = new Primitive(PrimitiveKind.Boolean){name = "Boolean"};
        Byte = new Primitive(PrimitiveKind.u8){name = "Byte"};
        UShort = new Primitive(PrimitiveKind.u16){name = "UShort"};
        Short = new Primitive(PrimitiveKind.i16){name = "Short"};
        Rune = new Primitive(PrimitiveKind.u32){name = "Rune"};
        UInteger = new Primitive(PrimitiveKind.u32){name = "UInteger"};
        Integer = new Primitive(PrimitiveKind.i32){name = "Integer"};
        Long = new Primitive(PrimitiveKind.i64){name = "Long"};
        ULong = new Primitive(PrimitiveKind.u64){name = "ULong"};
        Single = new Primitive(PrimitiveKind.f32){name = "Single"};
        Double = new Primitive(PrimitiveKind.f64){name = "Double"};
        Ptr = new Primitive(PrimitiveKind.ptr) { name = "Ptr" };

        global = new Module{
            name="global",
            flag = new Flag(Flags.Public),
            location = new BuilitInLocation(),
            completer = new SourceModuleCompleter()
        };

        if(complileBuiltinPackage)
            CompileBuiltinPackage(packageOrSourcePath);
        else{
            packagePath = packageOrSourcePath;
            Package package = Package.FromFile(this, packageOrSourcePath + "/core.bkg");
            dependencies.Add(package);
        }


        global.scope.Add(Boolean);
        global.scope.Add(Byte);
        global.scope.Add(Short);
        global.scope.Add(Rune);
        global.scope.Add(Integer);
        global.scope.Add(Long);
        global.scope.Add(Single);
        global.scope.Add(Double);
        global.scope.Add(Any);
        global.scope.Add(Void);
        global.scope.Add(UShort);
        global.scope.Add(UInteger);
        global.scope.Add(ULong);
        global.scope.Add(Ptr);

        EBArray = global.scope.Find("array") as Class;
        EBObject = global.scope.Find("object") as Class;
        EBString = global.scope.Find("string") as Class;
        getArrayLength = global.scope.Find("len") as ForeignEntry;

        int Y = 1, N = 0, NA = 2;
        conversion = new ConversionRule();
        Type[] primitives = {Boolean,Byte,Short,Rune,Integer,Long,Single,Double,EBObject};

        int[][] rule = {    /*Boolean Byte Short Rune Integer Long Single Double Object*/
        /*Boolean*/new int[]{   NA,    N,   N,    N,    N,     N,    N,     N,     Y   },
        /*Byte*/   new int[]{   N,     NA,  Y,    N,    Y,     Y,    Y,     Y,     Y   },
        /*Short*/  new int[]{   N,     Y,   NA,   N,    Y,     Y,    Y,     Y,     Y   },
        /*Rune*/   new int[]{   N,     N,   N,    N,    N,     N,    N,     N,     Y   },
        /*Integer*/new int[]{   N,     Y,   Y,    N,    NA,    Y,    Y,     Y,     Y   },
        /*Long*/   new int[]{   N,     Y,   Y,    N,    Y,     NA,   Y,     Y,     Y   },
        /*Single*/ new int[]{   N,     Y,   Y,    N,    Y,     Y,    NA,    Y,     Y   },
        /*Double*/ new int[]{   N,     Y,   Y,    N,    Y,     Y,    Y,     NA,    Y   },
        /*Object*/ new int[]{   Y,     Y,   Y,    Y,    Y,     Y,    Y,     Y,     NA  } //unboxing
        };
        for(int s=0; s<primitives.Length; s++){
            for(int d=0; d<primitives.Length; d++){
                if(rule[s][d]==Y){
                    var src = primitives[s];
                    var dst = primitives[d];
                    conversion.Add(src,dst);
                }
            }
        }

        promotion = new Promotion();
        Type[][] promo = {    /*Boolean   Byte   Short   Rune   Integer   Long   Single   Double   Object*/
        /*Boolean*/new Type[]{   null,    null,   null,  null,   null,    null,   null,    null,   null  },
        /*Byte*/   new Type[]{   null,    null,  Integer,null,  Integer,  Long,  Double,  Double,  null  },
        /*Short*/  new Type[]{   null,  Integer,  null,  null,  Integer,  Long,  Double,  Double,  null  },
        /*Rune*/   new Type[]{   null,    null,   null,  null,   null,    null,   null,    null,   null  },
        /*Integer*/new Type[]{   null,  Integer, Integer,null,   null,    Long,  Double,  Double,  null  },
        /*Long*/   new Type[]{   null,    Long,   Long,  null,   Long,    null,  Double,  Double,  null  },
        /*Single*/ new Type[]{   null,  Double,  Double, null,  Double,  Double,  null,    null,   null  },
        /*Double*/ new Type[]{   null,  Double,  Double, null,  Double,  Double, Double,   null,   null  },
        /*Object*/ new Type[]{   null,    null,   null,  null,   null,    null,   null,    null,   null  }
        };
        for(int lhs=0; lhs<primitives.Length; lhs++){
            for(int rhs=0; rhs<primitives.Length; rhs++){
                promotion.Add(primitives[lhs],primitives[rhs],promo[lhs][rhs]);
            }
        }

        truncation = new ConversionRule();
        int[][] trunc = {    /*Boolean Byte Short Rune Integer Long Single Double Object*/
        /*Boolean*/new int[]{   NA,     N,    N,   N,   N,     N,   NA,    NA,    NA  },
        /*Byte*/   new int[]{   N,      NA,   N,   N,   N,     N,   NA,    NA,    NA  },
        /*Short*/  new int[]{   N,      Y,    NA,  N,   N,     N,   NA,    NA,    NA  },
        /*Rune*/   new int[]{   N,      N,    N,   NA,  N,     N,   NA,    NA,    NA  },
        /*Integer*/new int[]{   N,      Y,    Y,   N,   NA,    N,   NA,    NA,    NA  },
        /*Long*/   new int[]{   N,      Y,    Y,   N,   Y,     NA,  NA,    NA,    NA  },
        /*Single*/ new int[]{   NA,     NA,   NA,  NA,  NA,    NA,  NA,    NA,    NA  },
        /*Double*/ new int[]{   NA,     NA,   NA,  NA,  NA,    NA,  NA,    NA,    NA  },
        /*Object*/ new int[]{   NA,     NA,   NA,  NA,  NA,    NA,  NA,    NA,    NA  }
        };
        for(int s=0; s<primitives.Length; s++){
            for(int d=0; d<primitives.Length; d++){
                if(trunc[s][d]==Y) truncation.Add(primitives[s],primitives[d]);
            }
        }


        widening = new ConversionRule();
        int[][] widen = {    /*Boolean Byte Short Rune Integer Long Single Double Object*/
        /*Boolean*/new int[]{   NA,     N,    N,   N,   N,     N,   NA,    NA,    NA },
        /*Byte*/   new int[]{   N,      NA,   Y,   N,   Y,     Y,   NA,    NA,    NA },
        /*Short*/  new int[]{   N,      N,    NA,  N,   Y,     Y,   NA,    NA,    NA },
        /*Rune*/   new int[]{   N,      N,    N,   NA,  N,     N,   NA,    NA,    NA },
        /*Integer*/new int[]{   N,      N,    N,   N,   NA,    Y,   NA,    NA,    NA },
        /*Long*/   new int[]{   N,      N,    N,   N,   N,     NA,  NA,    NA,    NA },
        /*Single*/ new int[]{   NA,     NA,   NA,  NA,  NA,    NA,  NA,    NA,    NA },
        /*Double*/ new int[]{   NA,     NA,   NA,  NA,  NA,    NA,  NA,    NA,    NA },
        /*Object*/ new int[]{   NA,     NA,   NA,  NA,  NA,    NA,  NA,    NA,    NA }
        };
        for(int s=0; s<primitives.Length; s++){
            for(int d=0; d<primitives.Length; d++){
                if(widen[s][d]==Y) widening.Add(primitives[s],primitives[d]);
            }
        }
        
        boxing = new Boxing(this);

        floatingPointConvertion = new ConversionRule();
        int[][] floatingPoint = {/*Boolean Byte Short Rune Integer Long Single Double Object*/
            /*Boolean*/new int[]{   NA,     NA,   NA,  NA,  NA,    NA,  N,     N,     NA },
            /*Byte*/   new int[]{   NA,     NA,   NA,  NA,  NA,    NA,  Y,     Y,     NA },
            /*Short*/  new int[]{   NA,     NA,   NA,  NA,  NA,    NA,  Y,     Y,     NA },
            /*Rune*/   new int[]{   NA,     NA,   NA,  NA,  NA,    NA,  N,     N,     NA },
            /*Integer*/new int[]{   NA,     NA,   NA,  NA,  NA,    NA,  Y,     Y,     NA },
            /*Long*/   new int[]{   NA,     NA,   NA,  NA,  NA,    NA,  Y,     Y,     NA },
            /*Single*/ new int[]{   N,      Y,    Y,   N,   Y,     Y,   NA,    Y,     NA },
            /*Double*/ new int[]{   N,      Y,    Y,   N,   Y,     Y,   Y,     NA,    NA },
            /*Object*/ new int[]{   NA,     NA,   NA,  NA,  NA,    NA,  NA,    NA,    NA }
        };
        for(int s=0; s<primitives.Length; s++){
            for(int d=0; d<primitives.Length; d++){
                if(floatingPoint[s][d]==Y) floatingPointConvertion.Add(primitives[s],primitives[d]);
            }
        }
        
    }

    void CompileBuiltinPackage(string dir){
        var fstPass = new FstPass();
        foreach(var file in Directory.EnumerateFiles(dir, "*.eb", SearchOption.AllDirectories)){
            var source = Source.FromFilePath(file);
            evoBasicLexer lexer = new evoBasicLexer(source.Stream);
            lexer.RemoveErrorListeners();
            lexer.AddErrorListener(LexerErrorListener.singleton);
            ITokenStream tokens = new CommonTokenStream(lexer);
            evoBasicParser parser = new evoBasicParser(tokens);
            parser.RemoveErrorListeners();
            parser.AddErrorListener(ParserErrorListener.singleton);
            parser.BuildParseTree = true;
            source.Document = parser.document();
            fstPass.Traverse(this,source);
        }
        EBObject = global.scope.Find("object") as Class;
        EBString = global.scope.Find("string") as Class;
    }

    Queue<String> dependQueue = new Queue<String>();
    List<Package> dependencies = new List<Package>();

    List<String> envPaths = new List<String>();

    Queue<Source> sourcesQueue = new Queue<Source>();
    List<Source> sources = new List<Source>();
    
    public Formater fmt = new Formater();

    public TopologicalOrder<Class> inheritDependent = new TopologicalOrder<Class>();

    public Dictionary<string,UInt32> TokenID;

    public void AddEnvPath(String path){
        if(Directory.Exists(path)){
            envPaths.Add(path);
        }
        else{
            Logger.Add(Log.Error, fmt.DirectoryNotFound(path));
        }
    }

    public void AddSource(Source source){
        sourcesQueue.Enqueue(source);
        sources.Add(source);
    }

    public List<Source> getSouces(){
        return sources;
    }

    public bool AddDependencies(String packageName){
        var target = packagePath + "/" + packageName + ".bkg";
        if(File.Exists(target)){
            dependQueue.Enqueue(target);
            return true;
        }
        else{
            Logger.Add(Log.Error, fmt.ResourceNotFound(target));
            return false;
        }
    }

    public IEnumerable<Package> GetDependencies() => dependencies;

    //添加所有可用的转换规则到conversion
    void loadConversionRule(Class cls){
        var baseCls = cls.baseClass;
        while(baseCls!=null){
            conversion.Add(cls,baseCls);
            conversion.Add(baseCls,cls);
            baseCls = baseCls.baseClass;
        }
    }

    void inheritCheck(){
        //检查继承覆写和接口是否实现
        if(!inheritDependent.Solve()){
            Logger.Add(Log.Error, Formater.singleton.CircleInherit());
        }
        foreach(var cls in inheritDependent.Order()){

            loadConversionRule(cls);

            if(cls.baseClass!=null){
                cls.virtualMethod = new Dictionary<string, Method>(cls.baseClass.virtualMethod);
            }
            else{
                cls.virtualMethod = new Dictionary<string, Method>();
            }

            foreach(var (name,sym) in cls.scope.childern){
                if(sym is Method method){
                    if(method.flag.Exists(Flags.Virtual)){
                        var virtualKey = method.name.ToLower();
                        if(cls.virtualMethod.ContainsKey(virtualKey)){
                            var baseVirtualMethod = cls.virtualMethod[virtualKey];
                            Logger.Add(Log.Error, method.location, Formater.singleton.VirtualMethodNameDuplicate(baseVirtualMethod.owner.attch.name,cls.name,method.name));
                        }
                        else{
                            cls.virtualMethod.Add(virtualKey,method);
                        } 
                    }
                    else if(method.flag.Exists(Flags.Override)){
                        var virtualKey = method.name.ToLower();
                        if(!cls.virtualMethod.ContainsKey(virtualKey)){
                            Logger.Add(Log.Error, method.location, Formater.singleton.NoVirtualMethodToOverride(method.name));
                        }
                        else{
                            var virtualMethod = cls.virtualMethod[virtualKey];
                            if(!virtualMethod.Compatible(method)){
                                Logger.Add(Log.Error, method.location, Formater.singleton.OverrideMethodIncompatible(virtualMethod.owner.attch.name,cls.name,method.name));
                            }
                        }
                    }
                }
            }
        }
    }

    public void Compile(){
        var fstPass = new FstPass();
        while(sourcesQueue.Count > 0){
            var source = sourcesQueue.Dequeue();
            evoBasicLexer lexer = new evoBasicLexer(source.Stream);
            lexer.RemoveErrorListeners();
            lexer.AddErrorListener(LexerErrorListener.singleton);
            ITokenStream tokens = new CommonTokenStream(lexer);
            evoBasicParser parser = new evoBasicParser(tokens);
            parser.RemoveErrorListeners();
            parser.AddErrorListener(ParserErrorListener.singleton);
            parser.BuildParseTree = true;
            source.Document = parser.document();
            fstPass.Traverse(this,source);
        }

        if (Logger.SyntaxErrorCount > 0) return;

        while(dependQueue.Count > 0){
            Package package = Package.FromFile(this,dependQueue.Dequeue());
            dependencies.Add(package);
            foreach(var dep in package.Denpendencies){
                AddDependencies(dep);
            }
        }

        global.completer.Complete(global,this,null);
        
        inheritCheck();

        new TrdPass(this).AnalyzeAll();
        
    }

    public void genreateIL(){
        var gen = new IL.ILGen(this);
        var queue = new Queue<Symbols.Scope>();
        queue.Enqueue(global.scope);
        while (queue.Count() > 0) {
            var handling = queue.Dequeue();
            foreach (var (_, child) in handling.childern) {
                switch (child) {
                    case Symbols.ExecuableFunction func:
                        if (func.location is Source.SourceLocation || func.location is CompilerGeneratedLocation)
                            func.ilBlock = gen.FromASTBlock(func, tokenDict);
                        break;
                    case Symbols.Module mod:
                        queue.Enqueue(mod.scope);
                        break;
                    case Symbols.Class cls:
                        queue.Enqueue(cls.scope);
                        break;
                }
            }
        }
    }
}


class Conversion{
    HashSet<Tuple<Type,Type>> conversion = new HashSet<Tuple<Type, Type>>();

    public bool Exist(Type src,Type dst){
        return conversion.Contains(new Tuple<Type,Type>(src,dst));
    }

    public void Add(Type src,Type dst){
        conversion.Add(new Tuple<Type,Type>(src,dst));
    }
}

class Promotion{
    Dictionary<Tuple<Type,Type>,Type> promotion = new Dictionary<Tuple<Type,Type>,Type>();

    public bool Exist(Type src,Type dst){
        return promotion.ContainsKey(new Tuple<Type, Type>(src,dst));
    }

    public Type? Query(Type lhs,Type rhs){
        var key = new Tuple<Type, Type>(lhs,rhs);
        if(promotion.ContainsKey(key)){
            return promotion[key];
        }
        else{
            return null;
        }
    }

    public void Add(Type lhs,Type rhs,Type result){
        promotion[new Tuple<Type, Type>(lhs,rhs)] = result;
    }
}


class ConversionRule{
    HashSet<Tuple<Type,Type>> rule = new HashSet<Tuple<Type, Type>>();
    public bool Exist(Type src,Type dst) => rule.Contains(new Tuple<Type, Type>(src,dst));
    public void Add(Type src,Type dst){
        rule.Add(new Tuple<Type, Type>(src,dst));
    }
}


class Boxing{
    Dictionary<Type,Class> boxing = new Dictionary<Type,Class>();

    public Boxing(CompileEnv env){
        new List<Type>{
            env.Boolean,
            env.Byte,
            env.Short,
            env.Rune,
            env.Integer,
            env.Long,
            env.Single,
            env.Double,
            env.UShort,
            env.UInteger,
            env.ULong
        }.ForEach(
            x => boxing.Add(x,env.global.scope.Find(x.name.ToLower() + "box") as Class)
        );
    }

    public bool Exist(Type type) => boxing.ContainsKey(type);
    public Class FindBox(Type type) => boxing[type];
}




