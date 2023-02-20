namespace evoBasic{
    using System.IO;
    using ParserRuleContext = Antlr4.Runtime.ParserRuleContext;
    using TokenContext = Antlr4.Runtime.Tree.ITerminalNode;

    class ParserErrorListener : Antlr4.Runtime.BaseErrorListener{
        public static ParserErrorListener singleton = new ParserErrorListener();
        public override void SyntaxError(TextWriter output, Antlr4.Runtime.IRecognizer recognizer, Antlr4.Runtime.IToken offendingSymbol, 
                                            int line, int charPositionInLine, string msg, Antlr4.Runtime.RecognitionException e){
            var location = new Source.SourceLocation(offendingSymbol.InputStream.SourceName,
                                                                        offendingSymbol.Column,
                                                                        offendingSymbol.Column + offendingSymbol.Text.Length,
                                                                        offendingSymbol.Line,
                                                                        offendingSymbol.Text.ToString());
            Logger.Add(Log.SyntaxError, location, msg);
        }
    }

    class LexerErrorListener : Antlr4.Runtime.IAntlrErrorListener<int>{
        public static LexerErrorListener singleton = new LexerErrorListener();
        public void SyntaxError(TextWriter output, Antlr4.Runtime.IRecognizer recognizer, int offendingSymbol, int line, int charPositionInLine, string msg, Antlr4.Runtime.RecognitionException e){
            var location = new Source.SourceLocation(e.InputStream.SourceName,
                                                    charPositionInLine,
                                                    charPositionInLine,
                                                    line,
                                                    "");
            Logger.Add(Log.SyntaxError, location, msg);
        }
    }

    class Formater{
        public static Formater singleton = new Formater();
        public virtual string CannotInferVariableType(string name) => $"cannot infer type of '{name}'";
        public virtual string CannotInitVariableHere() => $"cannot initialize vaiable here";
        public virtual string NameDuplicate(string name) => $"duplicate name '{name}'"; 
        public virtual string DirectoryNotFound(string path) => $"directory '{path}' not found";
        public virtual string ResourceNotFound(string name) => $"resource '{name}' not found";
        public virtual string InvalidImpl(string name) => $"'{name}' is not Interface";
        public virtual string InvalidAssign() => $"cannot assign value to RValue";
        public virtual string ElementTypeDifferent(List<Symbols.Type> types){
            string typesStr = types.First().getFullName();
            for(int i = 1; i < types.Count(); i++){
                typesStr = typesStr + "," + types[i].getFullName();
            }
            return $"array with multiple types [{typesStr}] is not allowed";
        }
        public virtual string ObjectNotfound(string objectText) => $"object '{objectText}' not found";
        public virtual string InvalidOptInit() => "invalid optional parameter initialization";
        public virtual string OptNotFound(string text) => $"optional parameter '{text}' not found";
        public virtual string TargetIsNotCallable(string text) => $"'{text}' is not an callable object";
        public virtual string TargetIsNotArray(string text) =>$"'{text}' is not an array";
        public virtual string TypeMismatch(string expected,string actual) =>$"expected '{expected}' here,but actual type is '{actual}'";
        public virtual string LhsIsNotScope() => $"lhs is not a Module, Class, Enum or Type";
        public virtual string InvalidDotExpression() => $"invalid dot expression";
        public virtual string CannotAccessNonStaticMember() => $"cannot access non-static member from static environment";
        public virtual string CannotAccessStaticMember() => $"cannot access static member from variable";
        public virtual string ArgAfterOptNotAllowed(string text) => $"argument '{text}' after option is not allowed";
        public virtual string OptNotFound(string opt,string function) => $"option '{opt}' not found in '{function}'";
        public virtual string ArgCountMismatch(int require,string function,int provide) => provide > 1 ? $"Requires {require} arguments for {function}, but {provide} were provided" 
                                                                                                    : $"Requires {require} arguments for {function}, but {provide} was provided";
        public virtual string CannotReturnInSub() => $"cannot return value in sub. Use 'Exit Sub' instead";
        public virtual string MissingReturnTypeInFunction(string functionName) => $"Missing return type of function '{functionName}'";
        public virtual string InvalidReturnTypeInSub(string subName) => $"Sub '{subName}' cannot have return type";
        public virtual string ContinueOnlyAllowedInLoop() => "'Continue' only allowed in While or For statement";
        public virtual string ExitOnlyAllowedIn(string flag) => $"'Exit {flag}' only allowed in {flag}";
        public virtual string TargetIsNotClass(string target) => $"'{target}' is not a class";
        public virtual string TargetUnreachable(string target) => $"'{target}' is unreachable in current scope";
        public virtual string CircleInherit() => $"circle inherit";
        public virtual string NoVirtualMethodToOverride(string methodName) => $"no virtual method '{methodName}' to override";
        public virtual string OverrideMethodIncompatible(string baseCls,string derivedCls,string overrideMethod) 
                                                                => $"override '{derivedCls}.{overrideMethod}' is incompatible with virtual method '{baseCls}.{overrideMethod}'";
        public virtual string VirtualMethodNameDuplicate(string baseCls,string derivedCls,string method)
                                                                => $"virtual method '{derivedCls}.{method}' conflict with '{baseCls}.{method}'";
        public virtual string ConversionNotFound(string srcType,string dstType) => $"available conversion from '{srcType}' to '{dstType}' not found";
        public virtual string ParamArrayMustBeObjectArray() => "type of ParamArray must be 'Object[]'";
        public virtual string ParamArrayAlwaysByVal() => "a ParamArray parameter is always declared using ByVal";
        public virtual string CannotPassRValueByVal() => "pass rvalue by reference is not allowed";
        public virtual string UnkonwnEscapeSequences(char c) => $"unkonwn escape sequence '\\{c}'";
        public virtual string IsNotOptionalParameter(string name) => $"'{name}' is not an optional parameter";

        public virtual string TypeAnyMustByref() => "when a parameter type is 'Any', it must be 'ByRef'";
    }

    enum Log{SyntaxError,Error,Warning,Tips};

    static class Logger{
        static Dictionary<String, List<Tuple<Log, Location, String>>> data = new Dictionary<string, List<Tuple<Log, Location, string>>>();
        

        public static int SyntaxErrorCount { private set; get; }

        public static int ErrorCount{private set;get;}

        private static List<Tuple<Log, Location, String>> getDataList(string key){
            if (!data.ContainsKey(key)) {
                data.Add(key, new List<Tuple<Log, Location, string>>());
            }
            return data[key];
        }

        private static void countError(Log log){
            switch (log){
                case Log.SyntaxError:
                    SyntaxErrorCount++;
                    ErrorCount++;
                    break;
                case Log.Error:
                    ErrorCount++;
                    break;
                case Log.Tips:
                    break;
            }
        }

        public static void Add(Log log, string text){
            countError(log);
            getDataList("Global").Add(new Tuple<Log, Location, string>(log,null,text));
        }

        public static void Add(Log log,Location location,string text){
            countError(log);
            getDataList(location.source()).Add(new Tuple<Log, Location, string>(log,location,text));
        }

        public static void Add(Log log,ParserRuleContext context,string text){
            countError(log);
            var location = new Source.SourceLocation(context);
            getDataList(location.source()).Add(new Tuple<Log, Location, string>(log,location,text));
        }

        public static void Add(Log log,TokenContext context,string text){
            countError(log);
            var location = new Source.SourceLocation(context);
            getDataList(location.source()).Add(new Tuple<Log, Location, string>(log,location,text));
        }

        public static void Dump(){
            foreach((string key, var ls) in data){
                Console.WriteLine($"In {key}");
                foreach((Log log, Location? location, String message) in ls){
                    if(location==null){
                        Console.WriteLine($"    {log.ToString()}: {message}"); 
                    }
                    else{
                        Console.WriteLine($"    {log.ToString()}{location.position()}: {message}"); 
                    }
                }
            }
        }
    }
    
}