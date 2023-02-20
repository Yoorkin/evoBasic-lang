using evoBasic;
using System;
using System.ComponentModel.DataAnnotations;
using System.Text;

//D:\sources\evobasic-project-antlr\ebc\test.eb -i
//D:\sources\evobasic-project-antlr\langlibs\math\random.eb -o math -i
//--compile-core-package D:\sources\evobasic-project-antlr\langlibs\core -i
class CommandLine {
    string[] args;
    int i = 0;

    public CommandLine(string[] args){
        this.args = args;
    }
    
    public string Consume(){
        return args[i++];
    }

    public int Remain() => args.Length - i;
}


class Program{

    static string outputFileName = "output";
    static UInt64 outputVersion = 0;
    static string sourcePath = ".";
    static bool interativeDebugMode = false;
    static bool compileBuiltinPackage = false;
    static string builtinPackageSourcesPath = "";
    static string buildPath = ".";
    static string packagePath = ".";
    static List<string> sources = new List<string>();
    static List<string> packages = new List<string>();

    static void Main(string[] args){

        var cmd = new CommandLine(args);

        while (cmd.Remain()>0){
            var arg = cmd.Consume();
            switch(arg){
                case "-o":
                case "--output":
                    outputFileName = cmd.Consume();
                    break;
                case "-s":
                case "--source-path":
                    sourcePath = cmd.Consume();
                    break;
                case "-i":
                case "--interative-debug":
                    interativeDebugMode = true;
                    break;
                case "-p":
                case "--package-dir":
                    packagePath = cmd.Consume();
                    break;
                case "-b":
                case "--build-dir":
                    buildPath = cmd.Consume();
                    break;
                case "--compile-core-package":
                    compileBuiltinPackage = true;
                    outputFileName = "core";
                    builtinPackageSourcesPath = cmd.Consume();
                    break;
                case "-h":
                case "--help":
                    printHelp();
                    break;
                default:
                    var path = arg;
                    var seg = path.Split(".");
                    var ext = seg[seg.Length-1];
                    if(ext == "eb") sources.Add(path);
                    else if(ext == "bkg") packages.Add(path);
                    else Console.WriteLine($"Unexpected file format '{ext}'");
                    break;
            }
        }

        if (sources.Count() == 0 && !compileBuiltinPackage) {
            printHelp();
            return;
        }

        Console.WriteLine($"Working Directory: {Directory.GetCurrentDirectory()}");
        Console.WriteLine($"Package Directory: {Path.GetFullPath(packagePath)}");
        Console.WriteLine($"Compiling {outputFileName}...");

        CompileEnv env = new CompileEnv(outputFileName, outputVersion, compileBuiltinPackage, 
                                        compileBuiltinPackage ? builtinPackageSourcesPath : packagePath);

        foreach(var path in sources){
            env.AddSource(Source.FromFilePath(path));
        }

        env.Compile();

        if(Logger.ErrorCount==0){
            env.genreateIL();
            Package package = Package.FromCompileEnv(env);
            try{
                package.SaveTo(buildPath + "/" + outputFileName + ".bkg");
                if(interativeDebugMode) Console.WriteLine(package.ToString());
                Console.WriteLine($"Package has been written to '{Path.GetFullPath(buildPath + "/" + outputFileName + ".bkg")}'");
                Console.WriteLine("Done.");
            }
            catch(IOException e){
                Console.WriteLine("Cannot generate output.");
            }

            if(interativeDebugMode){
                Console.WriteLine(package.ToString());
            }
            
            Logger.Dump();
        }
        else{
            Logger.Dump();
            Console.WriteLine("Failed.");
        }


        if(interativeDebugMode) {
            Console.WriteLine(Encoding.Default);
            DebugRepl.Execute(env);
        }
    }

    static void printHelp(){
        Console.WriteLine(
@"
    evoBasic Compiler
    C# Antlr4版

    -h              --help                    打印帮助菜单
    -o <name>       --output <name>           生成的bkg包名字
    -b <directory>  --build-dir <directory>   bkg包输出路径
    -p <directory>  --package-dir <directory> 包依赖路径

    -i              --interative-debug        编译后进入交互式调试模式
    --compile-builtin-package <directory>     编译内建包源码
"
        );
    }
}


class DebugRepl{
    private static bool quit = false;
    public static void Execute(CompileEnv env){
        while(!quit){
            Console.Write("\n ebc> ");
            
            var args = Console.ReadLine().Split(' ');
            var cmd = args[0];
            var path = args.Count()>1 ? args[1] : "";
            var opt = args.Count()>2 ? args[2] : "";
            var ls = new List<string>();
            foreach(var p in path.Split('.'))ls.Add(p);
            var sym = path != "" ? env.global.scope.LookUp(ls) : null;
            Console.WriteLine(cmd switch {
                "child" => queryChild(sym),
                "list" => queryAllChild(sym),
                "show" => queryDetail(sym),
                "all" => showAllInfo(sym),
                "table" => queryAllChild(env.global),
                "quit" => Quit(),
                string other => $"unknown cmd '{other}'"
            });
        }
    }

    public static string Quit(){
        quit = true;
        return "Quit.";
    }

    public static string queryChild(evoBasic.Symbols.Symbol symbol){
        if(symbol==null)return "symbol not found";
        var scope = Utils.GetSymbolScope(symbol);
        string str = "* " + symbol.name + "\n";
        if(scope!=null){
            foreach(var (name,_) in scope.childern){
                str += $"\t| {name}\n";
            }
        }
        return str;
    }

    private static Node processSymbolOverview(evoBasic.Symbols.Symbol symbol){
        var ret = new Node{value = symbol.GetType().Name + " " + symbol.name};
        var scope = Utils.GetSymbolScope(symbol);
        if(scope!=null && !(symbol is evoBasic.Symbols.Variable)){
            var ls = new List<Node>();
            foreach(var (_,child) in scope.childern){
                ls.Add(processSymbolOverview(child));
            }
            ret.childs = ls;
        }
        return ret;
    }

    public static string queryAllChild(evoBasic.Symbols.Symbol symbol){
        if(symbol==null)return "symbol not found";
        return processSymbolOverview(symbol).ToString();
    }

    private static Node processSymbolDetail(evoBasic.Symbols.Symbol symbol){
        var ret = new Node{value = symbol.ToString()};
        var scope = Utils.GetSymbolScope(symbol);
        if(scope!=null && !(symbol is evoBasic.Symbols.Variable)){
            var ls = new List<Node>();
            foreach(var (_,child) in scope.childern){
                ls.Add(processSymbolDetail(child));
            }
            ret.childs = ls;
        }
        return ret;
    }

    public static string queryDetail(evoBasic.Symbols.Symbol symbol){
        if(symbol==null)return "symbol not found";
        return symbol.ToString();
    }

    public static string showAllInfo(evoBasic.Symbols.Symbol symbol){
        if(symbol==null)return "symbol not found";
        return processSymbolDetail(symbol).ToString();
    }

}

