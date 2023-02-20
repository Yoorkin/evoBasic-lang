using Newtonsoft.Json;


namespace evoBasic.Symbols {
    using Parser = evoBasicParser;

    [Flags]
    enum Flags {
        Nothing = 0,
        Public = 1,
        Private = 1 << 1,
        Virtual = 1 << 2,
        Override = 1 << 3,
        Static = 1 << 4,
        Byval = 1 << 5,
        Byref = 1 << 6,
        Optional = 1 << 7,
        ParamArray = 1 << 8
    }

    class Flag {
        [JsonProperty]
        Flags value;

        bool Test(Flags flag, Flags test) {
            return ((int)(flag & test) > 0);
        }

        public void Set(Flags flag) {
            value |= flag;
        }

        public void Reset(Flags flag) {
            value |= ~flag;
        }

        public bool Exists(Flags flag) {
            return Test(value, flag);
        }

        public static bool operator ==(Flag a, Flag b) {
            return a.value == b.value;
        }

        public static bool operator !=(Flag a, Flag b) {
            return !(a == b);
        }

        public Flag() { value = Flags.Nothing; }
        public Flag(Flags value) { this.value = value; }
        public Flag(UInt32 value) { this.value = (Flags)value; }

        public Flags GetValue() => value;

        public DumpObject Dump() {
            return new DumpPair { key = "access", value = value.ToString() };
        }

        public override string ToString() {
            return value.ToString();
        }
    }

    class Compeleter {
        public virtual void Complete(Object obj, CompileEnv env, Scope scope) { }
    }

    class Symbol {
        public String name;
        [JsonIgnoreAttribute]
        public Scope? owner;
        [JsonIgnoreAttribute]
        public Location location;
        [JsonIgnoreAttribute]
        public Compeleter completer = new Compeleter();

        public Flag flag = new Flag();

        public virtual string getFullName() {
            if (owner == null || owner.attch.name == "global") return name;
            else return owner.attch.getFullName() + "." + name;
        }

        public virtual List<string> getFullNameList() {
            if (owner == null || owner.attch.name == "global") {
                var ret = new List<string>();
                ret.Add(name);
                return ret;
            }
            else {
                var ret = owner.attch.getFullNameList();
                ret.Add(name);
                return ret;
            }
        }
    }

    class ErrorType : Type {
        public override bool Equal(Type type) {
            return true;
        }
    }

    class Type : Symbol {
        public virtual bool Equal(Type type) {
            if (type is ErrorType || type is Any) return true;
            return this == type;
        }
    }

    class Nothing : Type {
        public Nothing() {
            name = "Nothing";
        }
        public override bool Equal(Type type) {
            if (type is Class || type is Nothing || type is Array || type is ErrorType) return true;
            return false;
        }
    }

    class Any : Type {
        public Any() {
            name = "Any";
        }

        public override bool Equal(Type type) {
            return true;
        }
    }

    class Variable : Symbol {
        UInt16 idx = 0;
        bool hasIdx = false;
        public Type type;

        public UInt16 getIndex() {
            if (!hasIdx) throw new Exception();
            return idx;
        }

        public void setIndex(UInt16 value) {
            idx = value;
            hasIdx = true;
        }

        public override string ToString() {
            return $"Variable {flag.ToString()} {name} : {type.getFullName()} {(hasIdx ? ", index: " + idx : "")}";
        }
    }


    class Array : Class{
        Type type_;
        public Array(CompileEnv env){
            scope = new Scope(this);
            baseClass = env.EBArray;
            scope.AddExtendScope(baseClass.scope);
        }

        public Type type{
            get=>type_;
            set{
                type_ = value;
                name = $"{type_.name}[]";
            }
        }
        public override string getFullName(){
            return type_.getFullName() + $"[]";
        }
        public override List<string> getFullNameList(){
            var ls = type_.getFullNameList();
            ls.Add("[]");
            return ls;
        }
        public override bool Equal(Type type){
            if(type is Nothing || type is ErrorType || type is Any)return true;
            if(type is Array ary)return this.type.Equal(ary.type);
            else return false;
        }

        public override string ToString(){
            return $"Variable {name} : {type.getFullName()}[]";
        }
    }

    class Scope {
        [JsonIgnoreAttribute]
        public Scope? parent;
        [JsonIgnoreAttribute]
        public Symbol attch;
        [JsonIgnoreAttribute]
        List<Scope> extendScopes = new List<Scope>();

        public Scope(Symbol attch){
            this.attch = attch;
        }

        [JsonProperty]
        public Dictionary<String,Symbol> childern = new Dictionary<string, Symbol>();

        public bool Add(Symbol symbol){
            if(childern.ContainsKey(symbol.name.ToLower()))return false;
            childern.Add(symbol.name.ToLower(),symbol);
            var childScope = symbol switch{
                Module mod => mod.scope,
                Class cls => cls.scope,
                Enumeration em => em.scope,
                Record rcd => rcd.scope,
                Function function => function.scope,
                _ => null
            };
            if(childScope!=null)childScope.parent = this;
            symbol.owner = this;
            return true;
        }

        public Symbol? Find(String name,bool findInExtendScope = true){
            name = name.ToLower();
            if(childern.ContainsKey(name)){
                return childern[name];
            }
            else if(findInExtendScope){
                foreach(var ext in extendScopes){
                    var target = ext.Find(name);
                    if(target!=null)return target;
                }
            }
            return null;
        }

        public Symbol? Find(String name,Func<Symbol,bool> predict){
            var ret = Find(name);
            if(ret==null || !predict(ret))return null;
            else return ret;
        }

        public Symbol? LookUp(List<String> fullName){
            string firstName = fullName.First();
            fullName.RemoveAt(0);

            var symbol = LookUp(firstName);

            foreach(var name in fullName){
                if(symbol==null)return null;
                var nextScope = symbol switch{
                    Module mod => mod.scope,
                    Class cls => cls.scope,
                    Record rcd => rcd.scope,
                    Enumeration em => em.scope
                };
                symbol = nextScope.Find(name);
            }
            return symbol;
        }

        public bool Reachable(Scope global,Symbol target){
            var fullname = target.getFullName();
            return global.PublicItems().Contains(fullname) || InternalItems().Contains(fullname);
        }

        HashSet<string> internalPublicItems,internalItems;
        private HashSet<string> PublicItems(){
            if(attch.flag.Exists(Flags.Public)){
                return InternalPublicItems();
            }
            else{
                return new HashSet<string>();
            }
        }

        public HashSet<string> InternalPublicItems(){
            if(internalPublicItems!=null)return internalPublicItems;
            var ret = new HashSet<string>();
            foreach(var (_,sym) in childern){
                if(sym.flag.Exists(Flags.Public))ret.Add(sym.getFullName());
                if(Utils.GetSymbolScope(sym)!=null)
                    ret.UnionWith(Utils.GetSymbolScope(sym).PublicItems());
            }
            return internalPublicItems = ret;
        }

        private HashSet<string> InternalItems(){
            if(internalItems!=null)return internalItems;
            var ret = new HashSet<string>();
            foreach(var (_,sym) in childern){
                ret.Add(sym.getFullName());
                if(Utils.GetSymbolScope(sym) is Scope scope){
                    ret.UnionWith(scope.InternalPublicItems());
                }
            }
            if(parent!=null)ret.UnionWith(parent.InternalItems());
            return internalItems=ret;
        }

        public Symbol? LookUp(string name){
            name = name.ToLower();
            var p = this;
            while(p!=null){
                var ret = p.Find(name);
                if(ret!=null){
                    return ret;
                }
                else if(p.attch!=null && p.attch.name.ToLower()==name){
                    return p.attch;
                }
                p=p.parent;
            }
            return null;
        }

        public void AddExtendScope(Scope scope){
            extendScopes.Add(scope);
        }
    }

    class Module : Symbol{
        public Scope scope;
        public Module(){
            scope = new Scope(this);
        }
        
        public override string ToString(){
            return $"Module {flag.ToString()} {name}";
        }
    }

    class Record : Type{
        public Scope scope;
        public Record(){
            scope = new Scope(this);
        }

        public override string ToString(){
            var str = $"Record {flag.ToString()} {name} \n";
            foreach(var c in scope.childern){
                str += (c.ToString() + '\n');
            }
            return str;
        }
    }

    class Class : Type{
        public Scope scope;
        public Class(){
            scope = new Scope(this);
        }
        public Constructor ctor;
        public Class baseClass;
        public bool isAbstract = false;
        public Dictionary<string,Interface> implInterfaces = new Dictionary<string, Interface>();
        public Dictionary<string,Method> virtualMethod = null;

        public override bool Equal(Type type){
            if(type==this || type is Nothing || type is ErrorType || type is Any)return true;
            if(type is Class cls){
                Class p = cls;
                while(p!=null){
                    if(p==this)return true;
                    p = p.baseClass;
                }
                return false;
            }
            else if(type is Interface itf){
                return implInterfaces.ContainsKey(itf.getFullName());
            }
            else return false;
        }

        public override string ToString(){
            var str = $"Class {flag.ToString()} {name} : {(baseClass==null ? "null" : baseClass.getFullName())} \n";
            if(implInterfaces.Count>0){
                str += "Impl{\n";
                foreach(var (_,itf) in implInterfaces){
                    str += $"    {itf.getFullName()}\n";
                }
                str += "}\n";
            }
            str += ctor.ToString();
            return str;
        }
    }

    class Interface : Type{
        public Scope scope;
        public Interface(){
            scope = new Scope(this);
        }
    }

    class Parameter : Variable{
        public AST.Expression astInitialExpression;

        public bool Compatible(Parameter parameter){
            if(!type.Equal(parameter.type))return false;
            if(flag!=parameter.flag)return false;
            return true;
        }

        public override string ToString(){
            return $"{flag.ToString()} {name} : {type.getFullName()}";
        }
    }

    class Function : Symbol{
        public Scope scope;
        public Function(){
            scope = new Scope(this);
        }
        //只包含必选参数
        public List<Parameter> normalParameters = new List<Parameter>();
        //只包含可选参数
        public List<Parameter> optionParameters = new List<Parameter>();
        public Dictionary<string,Parameter> optionMap = new Dictionary<string, Parameter>();
        public Parameter paramArray = null;
        [JsonIgnoreAttribute]
        public Type? retType;
        /*
         *  判断function是否兼容this
         *  兼容条件：函数参数和返回值类型调用Compatible为真；
         *          this中出现的可选参数在function中均存在且Compatible为真
         */
        public bool Compatible(Function function){
            if(this.normalParameters.Count!=function.normalParameters.Count)return false;
            for(int i=0;i<normalParameters.Count;i++){
                if(!this.normalParameters[i].Compatible(function.normalParameters[i]))return false;
            }
            foreach(var (name,opt) in optionMap){
                if(!function.optionMap.ContainsKey(name))return false;
                if(!function.optionMap[name].Compatible(optionMap[name]))return false;
            }
            return true;
        }
    }

    class ExecuableFunction : Function{
        public Parser.StatementContext[] parseTreeBlock;
        public AST.Block astBlock;
        public IL.Blocks ilBlock;
        public Byte[] bytecodes;
        public List<Tuple<Variable,UInt32>> localIndex = new List<Tuple<Variable, UInt32>>();
        protected string getIlBlockString(){
            return ilBlock == null ? "<Binary>" : ilBlock.ToString();
        }
    }

    class Method : ExecuableFunction{

        public override string ToString(){
            string str = $"Function {flag.ToString()} {name}(";
            var ps = new List<String>();
            foreach(var p in normalParameters){
                ps.Add(p.ToString());
            }
            foreach(var opt in optionParameters){
                ps.Add(opt.ToString());
            }
            str += String.Join(',',ps.ToArray());
            str += ") As " + retType.getFullName() + "\n";

            if(astBlock!=null){
                str += astBlock.DumpDebugNode().ToString();
            }

            foreach(var (variable,_) in localIndex){
                str += $"\n @{variable.getIndex()} {variable.name} : {variable.type.getFullName()}";
            }

            str += "\n" + getIlBlockString();
            return str;
        }
    }

    class Constructor : ExecuableFunction{
        public Class ownerClass;
        public Parser.ExtendPartContext extendPartContext;
        public AST.BaseCtorCall extendCtorAST;
        public Constructor(){
            this.name = "#ctor";
            astBlock = new AST.Block();
        }

        public override string ToString(){
            string str = $"Ctor {flag.ToString()} {name}(";
            var ps = new List<String>();
            foreach(var p in normalParameters){
                ps.Add(p.ToString());
            }
            foreach(var opt in optionParameters){
                ps.Add(opt.ToString());
            }
            str += String.Join(',',ps.ToArray());
            str += ")";

            if(extendCtorAST!=null) str += $"\n extend {extendCtorAST.DumpDebugNode()}";

            if(astBlock!=null){
                str += $"block {astBlock.DumpDebugNode().ToString()}";
            }
            str += "\n" + getIlBlockString();

            foreach(var (variable,_) in localIndex){
                str += $"\n @{variable.getIndex()} {variable.name} : {variable.type.getFullName()}";
            }

            return str;
        }
    }

    class ForeignEntry : Function{
        public string? lib,alias;
        public override string ToString(){
            string str = $"ForeignEntry {flag.ToString()} {name}";          
            if(lib!=null)str += " Lib " + lib;
            if(alias!=null)str += "Alias " + alias;
            str += " (";

            var ps = new List<String>();
            foreach(var p in normalParameters){
                ps.Add(p.ToString());
            }
            foreach(var opt in optionParameters){
                ps.Add(opt.ToString());
            }
            str += String.Join(',',ps.ToArray());

            str += ") As " + retType.getFullName();
            return str;
        }
    }


    enum PrimitiveKind {Void,Boolean,i8,i16,i32,i64,u8,u16,u32,u64,f32,f64,rune,ptr};
    class Primitive : Type{
        public PrimitiveKind kind;
        public Primitive(PrimitiveKind kind){
            this.kind = kind;
            this.flag.Set(Flags.Public);
            this.location = location = new BuilitInLocation();
        }

        public override string ToString(){
            return $"Primitive {name}";
        }
    }

    class Enumeration : Type{
        public Scope scope;
        public Enumeration(){
            scope = new Scope(this);
        }

        public override string ToString(){
            return $"Enum {flag.ToString()} {name}";
        }
    }

    class EnumConstant : Symbol{
        public UInt32 value;

        public override string ToString(){
            return $"EnumConstant {name} = {value}";
        }
    }


}