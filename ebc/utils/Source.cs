namespace evoBasic{
    using Antlr4.Runtime;
    using DocumentContext = evoBasicParser.DocumentContext;

    public abstract class Location{
        public abstract string source();
        public abstract string position();
    }

    class BuilitInLocation : Location{
        public override string position() => "";
        public override string source() => "BuiltIn";
    }

    class CompilerGeneratedLocation : Location{
        public override string position() => "";
        public override string source() => "CompilerGenerated";
    } 

    class Source{

        public ICharStream Stream{get; private set;}
        public DocumentContext Document; 

        public Source(string text, string name){
            var s = CharStreams.fromString(text + "\n") as CodePointCharStream;
            s.name = name;
            Stream = s;
            sourcesMap[name] = this;
        }

        public static Source FromFilePath(string path){
            var text = System.IO.File.ReadAllText(path);
            var name = Path.GetFullPath(path);
            return new Source(text, name);
        }

        public string GetName(){
            return Stream.SourceName;
        }

        static Dictionary<String,Source> sourcesMap = new Dictionary<String, Source>();

        public class SourceLocation : Location{
            String sourceName;

            public readonly int beginColumn,beginLine,endColumn,endLine;
            public readonly string text;

            public Source GetSource(){
                return sourcesMap[sourceName];
            }

            public override string position() => $"(line {beginLine},column {beginColumn})";
            public override string source() => $"\"{GetSource().GetName()}\"";
   
            public SourceLocation(String sourceName, int beginCol, int endCol, int beginLine, string text){
                this.sourceName = sourceName;
                this.beginLine = beginLine;
                this.beginColumn = beginCol;
                this.endColumn = endCol;
                this.text = text;
            }

            public SourceLocation(Antlr4.Runtime.Tree.ITerminalNode node){
                sourceName = node.Symbol.InputStream.SourceName;
                beginColumn = node.Symbol.Column;
                endColumn = node.GetText().Length + beginColumn;
                beginLine = endLine = node.Symbol.Line;
                text = node.GetText();
            }
            
            public SourceLocation(Antlr4.Runtime.ParserRuleContext context){
                sourceName = context.Start.InputStream.SourceName;
                text = context.Start.InputStream.GetText(context.SourceInterval);
                beginColumn = context.Start.Column;
                beginLine = context.Start.Line;
                endColumn = context.Stop.Column;
                endLine = context.Stop.Line;
            }
        } 
    }
}