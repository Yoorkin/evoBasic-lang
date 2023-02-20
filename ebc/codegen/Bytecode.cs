namespace evoBasic.IL;

using System.Diagnostics;
using Token = TokenDict.Token;


public enum Bytecode : Byte{ 
    t_boolean = 80,
    t_i8 = 81,
    t_i16 = 82,
    t_i32 = 83,
    t_i64 = 84,
    t_u8 = 85,
    t_u16 = 86,
    t_u32 = 87,
    t_u64 = 88,
    t_f32 = 89,
    t_f64 = 90,
    t_ref = 91,
    t_ftn = 92,
    t_vftn = 93,
    t_sftn = 94,
    t_ctor = 95,
    t_record = 96,
    ldsftn = 97,
    ldvftn = 98,
    ldftn = 99,
    ldctor = 100,
    ldforeign = 101,
    callmethod = 102,
    callvirtual = 103,
    callstatic = 104,
    callforeign = 105,
    callintrinsic = 106,
    starg = 107,
    ldarg = 108,
    ldarga = 109,
    stloc = 110,
    ldloc = 111,
    ldloca = 112,
    stfld = 113,
    ldfld = 114,
    ldflda = 115,
    stsfld = 116,
    ldsfld = 117,
    ldsflda = 118,
    packopt = 119,
    stelem = 120,
    ldelem = 121,
    ldelema = 122,
    newarray = 123,
    arraylength = 124,
    jif = 125,
    br = 126,
    ret = 127,
    nop = 0,
    dup = 128,
    push = 129,
    store = 130,
    load = 131,
    ldnothing = 132,
    convert = 133,
    castClass = 134,
    instanceof = 145,
    throw_ = 146,
    enter = 147,
    leave = 148,
    add = 149,
    sub = 150,
    mul = 151,
    div = 152,
    and_ = 153,
    or_ = 154,
    xor_ = 155,
    eq = 156,
    ne = 157,
    lt = 158,
    gt = 159,
    le = 160,
    ge = 161,
    neg = 162,
    not_ = 163,
    callctor = 164,
    stelemr = 165,
    pop = 166,
    ldstr = 167,
    testopt = 168,
    ldoptinfo = 169,
    ldenumc = 170,
    t_emconst = 171,
    newobj = 172,
    mod = 173,
    t_hdl = 174,
    t_dlg = 175,
    wrapsftn = 176,
    wrapvftn = 177,
    wrapftn = 178,
    wrapctor = 179,
    wrapforeign = 180,
    calldlg = 181,
    t_ptr = 182
}

class DataType{
    public enum Kind{
        Boolean,I8,I16,I32,I64,U8,U16,U32,U64,F32,F64,Ref,VFtn,SFtn,EmConst,Record,Hdl,Dlg,Ptr
    }
    public Kind kind;
    public Token token;

    public override string ToString() => kind switch {
        Kind.Record => "Record<" + token.QualifiedName() + ">",
        _ => kind.ToString()
    };

    public UInt32 getSize() => kind switch{
        Kind.Record => 1 + 4,
        _ => 1
    };

    public void writeToStream(BinaryWriter writer){
        writer.Write((Byte)(kind switch{
            Kind.Boolean => Bytecode.t_boolean,
            Kind.I8 => Bytecode.t_i8,
            Kind.I16 => Bytecode.t_i16,
            Kind.I32 => Bytecode.t_i32,
            Kind.I64 => Bytecode.t_i64,
            Kind.U8 => Bytecode.t_u8,
            Kind.U16 => Bytecode.t_u16,
            Kind.U32 => Bytecode.t_u32,
            Kind.U64 => Bytecode.t_u64,
            Kind.F32 => Bytecode.t_f32,
            Kind.F64 => Bytecode.t_f64,
            Kind.Ref => Bytecode.t_ref,
            Kind.VFtn => Bytecode.t_vftn,
            Kind.SFtn => Bytecode.t_sftn,
            Kind.EmConst => Bytecode.t_emconst,
            Kind.Record => Bytecode.t_record,
            Kind.Hdl => Bytecode.t_hdl,
            Kind.Dlg => Bytecode.t_dlg,
            Kind.Ptr => Bytecode.t_ptr,
            _ => throw new Exception()
        }));

        if(kind==Kind.Record){
            writer.Write(token.id);
        }
    }
}

static class Instructions{
    public abstract class InstBase {}

    public class LineDelimiter : InstBase {
        public UInt32 nextLineNumber;
        public LineDelimiter(UInt32 nextLineNumber) {
            this.nextLineNumber = nextLineNumber;
        }
    }

    public abstract class RealInst : InstBase {
        public Bytecode instbyte;  
        public abstract UInt32 getLength();  
        public abstract void writeToStream(BinaryWriter writer);
    };

    public class nopinst : RealInst{
        public nopinst(Bytecode code){
            instbyte = code;
        }
        public override string ToString() 
            => instbyte.ToString();

        public override UInt32 getLength() => 1;
        public override void writeToStream(BinaryWriter writer){
            writer.Write((Byte)instbyte);
        }
    }

    public class tokinst : RealInst{ 
        public Token tok;
        public tokinst(Bytecode code){
            instbyte = code;
        }
        public override string ToString() 
            => instbyte.ToString() + " " + tok.QualifiedName();
        public override UInt32 getLength() => 1 + 4;
        public override void writeToStream(BinaryWriter writer){
            writer.Write((Byte)instbyte);
            writer.Write(tok.id);
        }
    }

    public class typinst : RealInst{ 
        public DataType typ;
        public typinst(Bytecode code){
            instbyte = code;
        }
        public override string ToString()
            => instbyte.ToString() + "." + typ.ToString();
        
        public override UInt32 getLength() => 1 + typ.getSize();
        public override void writeToStream(BinaryWriter writer){
            writer.Write((Byte)instbyte);
            typ.writeToStream(writer);
        }
    }

    public class typtokinst : RealInst{ 
        public DataType typ; public Token tok;
        public typtokinst(Bytecode code){
            instbyte = code;
        }
        public override string ToString()
            => instbyte.ToString() + "." + typ.ToString() + " " + tok.QualifiedName();

        public override UInt32 getLength() => 1 + 4 + typ.getSize();
        public override void writeToStream(BinaryWriter writer){
            writer.Write((Byte)instbyte);
            typ.writeToStream(writer);
            writer.Write(tok.id);
        }
    }

    public class jif : RealInst{ 
        public BasicBlock target;
        public jif(Bytecode code){
            instbyte = code;
        }
        public override string ToString()
            =>  "jif " + target.getOffset() + "         //jump to " + target.tag;
        
        public override UInt32 getLength() => 1 + 4;
        public override void writeToStream(BinaryWriter writer){
            writer.Write((Byte)instbyte);
            writer.Write(target.getOffset());
        }
    }

    public class br : RealInst{ 
        public BasicBlock target; 
        public br(){
            instbyte = Bytecode.br;
        }
        public override string ToString()
            =>  "br " + target.getOffset() + "         //jump to " + target.tag;
        public override UInt32 getLength() => 1 + 4;
        public override void writeToStream(BinaryWriter writer){
            writer.Write((Byte)instbyte);
            writer.Write(target.getOffset());
        }
    }

    public class convert : RealInst { 
        public DataType src,dst; 
        public convert(){
            instbyte = Bytecode.convert;
        }
        public override string ToString()
            =>  "convert." + src.ToString() + " " + dst.ToString();
        public override UInt32 getLength() => 1 + src.getSize() + dst.getSize();
        public override void writeToStream(BinaryWriter writer){
            writer.Write((Byte)instbyte);
            src.writeToStream(writer);
            dst.writeToStream(writer);
        }
    }
    public class castClass : RealInst { 
        public Token src,dst; 
        public castClass(){
            instbyte = Bytecode.castClass;
        }
        public override string ToString()
            =>  "castclass." + src.QualifiedName() + " " + dst.QualifiedName();

        public override UInt32 getLength() => 1 + 4 + 4;
        public override void writeToStream(BinaryWriter writer){
            writer.Write((Byte)instbyte);
            writer.Write(src.id);
            writer.Write(dst.id);
        }
    }
    public class enter : RealInst{
        public Token tok;
        public BasicBlock target;
        public enter(){
            instbyte = Bytecode.enter;
        }
        public override string ToString()
            => "enter." + tok.QualifiedName() + " " + target.tag;
        public override UInt32 getLength() => 1 + 4 + 4;
        public override void writeToStream(BinaryWriter writer){
            writer.Write((Byte)instbyte);
            writer.Write(tok.id);
            writer.Write(target.getOffset());
        }
    }
    public class push : RealInst{ 
        public DataType typ; 
        public object val;

        public push(){
            instbyte = Bytecode.push;
        }

        public override string ToString()
            =>  "push." + typ.ToString() + " " + val.ToString();
        
        public override UInt32 getLength() => 1 + typ.getSize() + (UInt32)(typ.kind switch{
            DataType.Kind.Boolean => 1,
            DataType.Kind.I8 => 1,
            DataType.Kind.I16 => 2,
            DataType.Kind.I32 => 4,
            DataType.Kind.I64 => 8,
            DataType.Kind.U8 => 1,
            DataType.Kind.U16 => 2,
            DataType.Kind.U32 => 4,
            DataType.Kind.F32 => 4,
            DataType.Kind.F64 => 8,
            DataType.Kind.Ref => 8,
            _ => throw new Exception()
        });

        public override void writeToStream(BinaryWriter writer){
            writer.Write((Byte)instbyte);
            typ.writeToStream(writer);
            switch(val){
                case Boolean x: writer.Write(x?(Byte)1:(Byte)0); break;
                case Byte x: writer.Write(x); break;
                case UInt16 x: writer.Write(x); break;
                case UInt32 x: writer.Write(x); break;
                case UInt64 x: writer.Write(x); break;
                case SByte x: writer.Write(x); break;
                case Int16 x: writer.Write(x); break;
                case Int32 x: writer.Write(x); break;
                case Int64 x: writer.Write(x); break;
                case float x: writer.Write(x); break;
                case double x: writer.Write(x); break;
                default: throw new InvalidDataException();
            }
        }
    }

}


class Blocks {
    public struct LineNumber {
        public UInt32 offset;
        public UInt32 line;
    }

    List<LineNumber> lineNumbers;

    public struct ExceptionEntry {
        public UInt32 offset;
        public UInt32 length;
        public UInt32 target;
        public TokenDict.Token typeToken;
    }

    List<ExceptionEntry> exceptionEntries;

    List<BasicBlock> basicBlocks;

    public void writeBytecodeToStream(BinaryWriter writer) {
        foreach (var block in basicBlocks) {
            block.writeToStream(writer);
        }
    }

    public List<ExceptionEntry> getExceptionEntries() => exceptionEntries;
    public List<LineNumber> getLineNumbers() => lineNumbers;

    public override string ToString() {
        var ret = String.Concat(basicBlocks.Select(x => x.ToString()));
        ret += "LineNumberTable\n";
        var lineFormat = "    {0,-7}{1,-7}\n";
        ret += String.Format(lineFormat, "line", "offset");
        foreach (var line in lineNumbers) {
            ret += String.Format(lineFormat,line.line, line.offset);
        }
        ret += "ExceptionEntries\n";
        var entryFormat = "    {0,-7}{1,-7}{2,-7}{3,-20}\n";
        ret += String.Format(entryFormat, "offset", "length", "target", "exception");
        foreach (var entry in exceptionEntries) {
            ret += String.Format(entryFormat, entry.offset, entry.length, entry.target, entry.typeToken.QualifiedName());
        }
        return ret;
    }

    public Blocks(BasicBlock entry) {
        // 确定必要的BasicBlock的顺序
        basicBlocks = new List<BasicBlock>();
        var queue = new Queue<BasicBlock>();
        queue.Enqueue(entry);
        while (queue.Count > 0) {
            var target = queue.Dequeue();
            target.visited = true;
            foreach(var outBlk in target.getOutBasicBlocks()) {
                if (outBlk.visited) continue;
                queue.Enqueue(outBlk);
            }
            basicBlocks.Add(target);
        }

        // 计算每个BasicBlock的偏移和大小，顺便生成LineNumberTable
        lineNumbers = new List<LineNumber>();
        UInt32 offset = 0;
        foreach (var block in basicBlocks) {
            UInt32 blockBeginOffset = offset;
            foreach(var inst in block.instructions) {
                if(inst is Instructions.RealInst real) {
                    offset += real.getLength();
                }
                else if(inst is Instructions.LineDelimiter delimiter) {
                    lineNumbers.Add(new LineNumber {
                        offset = offset,
                        line = delimiter.nextLineNumber
                    });
                }
            }
            block.setOffsetAndLength(blockBeginOffset, offset - blockBeginOffset);
        }

        // 计算ExceptionEntryTable
        exceptionEntries = new List<ExceptionEntry>();
        foreach(var block in basicBlocks) {
            foreach(var catchHandler in block.getCatchHandlers()) {
                exceptionEntries.Add(new ExceptionEntry {
                    offset = block.getOffset(),
                    length = block.getLength(),
                    target = catchHandler.catchBlock.getOffset(),
                    typeToken = catchHandler.exceptionClassToken
                });
            }
        }
    }
}

class BasicBlock {
    public bool visited = false;
    public string tag;
    UInt32 offset, length;
    // 表示offset和length是否已计算
    bool valid = false;


    public BasicBlock(string tag){
        this.tag = tag;
    }

    public struct CatchHandler {
        public BasicBlock catchBlock;
        public TokenDict.Token exceptionClassToken;
    }

    List<CatchHandler> catchHandler = new List<CatchHandler>();

    public List<Instructions.InstBase> instructions = new List<Instructions.InstBase>();

    public List<CatchHandler> getCatchHandlers() => catchHandler;
    
    public void addTryCatchHandler(BasicBlock basicBlock, TokenDict.Token exceptionClassToken) {
        catchHandler.Add(new CatchHandler {
            catchBlock = basicBlock,
            exceptionClassToken = exceptionClassToken
        });
    }

    public IEnumerable<BasicBlock> getOutBasicBlocks() {
        var ret = new List<BasicBlock>();
        foreach (var inst in instructions) {
            if (inst is Instructions.jif jifInst) {
                ret.Add(jifInst.target);
            }
            else if (inst is Instructions.br brInst) {
                ret.Add(brInst.target);
            }
        }
        return ret.Concat(catchHandler.Select(x => x.catchBlock));
    }

    public void setOffsetAndLength(UInt32 offset, UInt32 length) {
        this.offset = offset;
        this.length = length;
        valid = true;
    }

    public UInt32 getOffset() {
        Debug.Assert(valid);
        return offset;
    }

    public UInt32 getLength() {
        Debug.Assert(valid);
        return length;
    }

    public void writeToStream(BinaryWriter writer) {
        foreach (var inst in instructions) {
            if(inst is Instructions.RealInst real) {
                real.writeToStream(writer);
            }
        }
    }

    public override string ToString() {
        var ret = $" {getOffset()}_{tag}:\n";
        foreach (var inst in instructions) {
            if(inst is Instructions.RealInst real) {
                ret += "    " + real.ToString() + "\n";
            }
        }
        return ret;
    }


    public BasicBlock markNewline(UInt32 lineNumber) {
        instructions.Add(new Instructions.LineDelimiter(lineNumber));
        return this;
    }

    public BasicBlock ldsftn(Token tok) {
        instructions.Add(new Instructions.tokinst(Bytecode.ldsftn) { tok = tok });
        return this;
    }

    public BasicBlock ldvftn(Token tok) {
        instructions.Add(new Instructions.tokinst(Bytecode.ldvftn) { tok = tok });
        return this;
    }

    public BasicBlock ldftn(Token tok) {
        instructions.Add(new Instructions.tokinst(Bytecode.ldftn) { tok = tok });
        return this;
    }

    public BasicBlock ldctor(Token tok) {
        instructions.Add(new Instructions.tokinst(Bytecode.ldctor) { tok = tok });
        return this;
    }

    public BasicBlock ldforeign(Token tok) {
        instructions.Add(new Instructions.tokinst(Bytecode.ldforeign) { tok = tok });
        return this;
    }

    public BasicBlock callforeign() {
        instructions.Add(new Instructions.nopinst(Bytecode.callforeign));
        return this;
    }

    public BasicBlock callintrinsic(Token tok) {
        instructions.Add(new Instructions.tokinst(Bytecode.callintrinsic) { tok = tok });
        return this;
    }

    public BasicBlock ldflda(Token tok) {
        instructions.Add(new Instructions.tokinst(Bytecode.ldflda) { tok = tok });
        return this;
    }

    public BasicBlock ldsflda(Token tok) {
        instructions.Add(new Instructions.tokinst(Bytecode.ldsflda) { tok = tok });
        return this;
    }

    public BasicBlock enter(Token excpetionClassTok, BasicBlock target) {
        instructions.Add(new Instructions.enter { tok = excpetionClassTok, target = target });
        return this;
    }

    public BasicBlock leave(Token excpetionClassTok) {
        instructions.Add(new Instructions.tokinst(Bytecode.leave) { tok = excpetionClassTok });
        return this;
    }

    public BasicBlock instanceof(Token tok) {
        instructions.Add(new Instructions.tokinst(Bytecode.instanceof) { tok = tok });
        return this;
    }

    public BasicBlock starg(DataType typ) {
        instructions.Add(new Instructions.typinst(Bytecode.starg) { typ = typ });
        return this;
    }

    public BasicBlock ldarg(DataType typ) {
        instructions.Add(new Instructions.typinst(Bytecode.ldarg) { typ = typ });
        return this;
    }

    public BasicBlock stloc(DataType typ) {
        instructions.Add(new Instructions.typinst(Bytecode.stloc) { typ = typ });
        return this;
    }

    public BasicBlock ldloc(DataType typ) {
        instructions.Add(new Instructions.typinst(Bytecode.ldloc) { typ = typ });
        return this;
    }

    public BasicBlock stelem(DataType typ, Token tok) {
        instructions.Add(new Instructions.typtokinst(Bytecode.stelem) { typ = typ, tok = tok });
        return this;
    }

    public BasicBlock stelemr(DataType typ, Token tok) {
        instructions.Add(new Instructions.typtokinst(Bytecode.stelemr) { typ = typ, tok = tok });
        return this;
    }

    public BasicBlock ldelem(DataType typ, Token tok) {
        instructions.Add(new Instructions.typtokinst(Bytecode.ldelem) { typ = typ, tok = tok });
        return this;
    }

    public BasicBlock ldelema(Token tok) {
        instructions.Add(new Instructions.tokinst(Bytecode.ldelema) { tok = tok });
        return this;
    }

    public BasicBlock newarray(Token tok) {
        instructions.Add(new Instructions.tokinst(Bytecode.newarray) { tok = tok });
        return this;
    }

    public BasicBlock store(DataType typ) {
        instructions.Add(new Instructions.typinst(Bytecode.store) { typ = typ });
        return this;
    }

    public BasicBlock load(DataType typ) {
        instructions.Add(new Instructions.typinst(Bytecode.load) { typ = typ });
        return this;
    }

    public BasicBlock add(DataType typ) {
        instructions.Add(new Instructions.typinst(Bytecode.add) { typ = typ });
        return this;
    }

    public BasicBlock sub(DataType typ) {
        instructions.Add(new Instructions.typinst(Bytecode.sub) { typ = typ });
        return this;
    }

    public BasicBlock mul(DataType typ) {
        instructions.Add(new Instructions.typinst(Bytecode.mul) { typ = typ });
        return this;
    }

    public BasicBlock div(DataType typ) {
        instructions.Add(new Instructions.typinst(Bytecode.div) { typ = typ });
        return this;
    }

    public BasicBlock and() {
        instructions.Add(new Instructions.nopinst(Bytecode.and_));
        return this;
    }

    public BasicBlock or() {
        instructions.Add(new Instructions.nopinst(Bytecode.or_));
        return this;
    }

    public BasicBlock xor() {
        instructions.Add(new Instructions.nopinst(Bytecode.xor_));
        return this;
    }

    public BasicBlock eq(DataType typ) {
        instructions.Add(new Instructions.typinst(Bytecode.eq) { typ = typ });
        return this;
    }

    public BasicBlock ne(DataType typ) {
        instructions.Add(new Instructions.typinst(Bytecode.ne) { typ = typ });
        return this;
    }

    public BasicBlock mod(DataType typ) {
        instructions.Add(new Instructions.typinst(Bytecode.mod) { typ = typ });
        return this;
    }

    public BasicBlock lt(DataType typ) {
        instructions.Add(new Instructions.typinst(Bytecode.lt) { typ = typ });
        return this;
    }

    public BasicBlock gt(DataType typ) {
        instructions.Add(new Instructions.typinst(Bytecode.gt) { typ = typ });
        return this;
    }

    public BasicBlock le(DataType typ) {
        instructions.Add(new Instructions.typinst(Bytecode.le) { typ = typ });
        return this;
    }

    public BasicBlock ge(DataType typ) {
        instructions.Add(new Instructions.typinst(Bytecode.ge) { typ = typ });
        return this;
    }

    public BasicBlock neg(DataType typ) {
        instructions.Add(new Instructions.typinst(Bytecode.neg) { typ = typ });
        return this;
    }

    public BasicBlock not() {
        instructions.Add(new Instructions.nopinst(Bytecode.not_));
        return this;
    }

    public BasicBlock dup(DataType typ) {
        instructions.Add(new Instructions.typinst(Bytecode.dup) { typ = typ });
        return this;
    }

    public BasicBlock pop(DataType typ) {
        instructions.Add(new Instructions.typinst(Bytecode.pop) { typ = typ });
        return this;
    }

    public BasicBlock stfld(DataType typ, Token tok) {
        instructions.Add(new Instructions.typtokinst(Bytecode.stfld) { typ = typ, tok = tok });
        return this;
    }

    public BasicBlock ldfld(DataType typ, Token tok) {
        instructions.Add(new Instructions.typtokinst(Bytecode.ldfld) { typ = typ, tok = tok });
        return this;
    }

    public BasicBlock stsfld(DataType typ, Token tok) {
        instructions.Add(new Instructions.typtokinst(Bytecode.stsfld) { typ = typ, tok = tok });
        return this;
    }

    public BasicBlock ldsfld(DataType typ, Token tok) {
        instructions.Add(new Instructions.typtokinst(Bytecode.ldsfld) { typ = typ, tok = tok });
        return this;
    }

    public BasicBlock callmethod() {
        instructions.Add(new Instructions.nopinst(Bytecode.callmethod));
        return this;
    }

    public BasicBlock callvirtual() {
        instructions.Add(new Instructions.nopinst(Bytecode.callvirtual));
        return this;
    }

    public BasicBlock callstatic() {
        instructions.Add(new Instructions.nopinst(Bytecode.callstatic));
        return this;
    }

    public BasicBlock callctor() {
        instructions.Add(new Instructions.nopinst(Bytecode.callctor));
        return this;
    }

    public BasicBlock ldarga() {
        instructions.Add(new Instructions.nopinst(Bytecode.ldarga));
        return this;
    }

    public BasicBlock ldloca() {
        instructions.Add(new Instructions.nopinst(Bytecode.ldloca));
        return this;
    }

    public BasicBlock arraylength() {
        instructions.Add(new Instructions.nopinst(Bytecode.arraylength));
        return this;
    }

    public BasicBlock ret() {
        instructions.Add(new Instructions.nopinst(Bytecode.ret));
        return this;
    }

    public BasicBlock nop() {
        instructions.Add(new Instructions.nopinst(Bytecode.nop));
        return this;
    }

    public BasicBlock ldnothing() {
        instructions.Add(new Instructions.nopinst(Bytecode.ldnothing));
        return this;
    }

    public BasicBlock throw_() {
        instructions.Add(new Instructions.nopinst(Bytecode.throw_));
        return this;
    }

    public BasicBlock jif(BasicBlock target) {
        instructions.Add(new Instructions.jif(Bytecode.jif) { target = target });
        return this;
    }

    public BasicBlock br(BasicBlock target) {
        instructions.Add(new Instructions.br { target = target });
        return this;
    }

    public BasicBlock convert(DataType src, DataType dst) {
        instructions.Add(new Instructions.convert { src = src, dst = dst });
        return this;
    }

    public BasicBlock castClass(Token src, Token dst) {
        instructions.Add(new Instructions.castClass { src = src, dst = dst });
        return this;
    }

    public BasicBlock push(DataType typ, object val) {
        instructions.Add(new Instructions.push { typ = typ, val = val });
        return this;
    }

    public BasicBlock ldstr(Token tok) {
        instructions.Add(new Instructions.tokinst(Bytecode.ldstr) { tok = tok });
        return this;
    }

    public BasicBlock testopt() {
        instructions.Add(new Instructions.nopinst(Bytecode.testopt));
        return this;
    }

    public BasicBlock ldoptinfo(Token tok) {
        instructions.Add(new Instructions.tokinst(Bytecode.ldoptinfo) { tok = tok });
        return this;
    }

    public BasicBlock ldenumc(Token tok) {
        instructions.Add(new Instructions.tokinst(Bytecode.ldenumc) { tok = tok });
        return this;
    }

    public BasicBlock newobj(Token tok) {
        instructions.Add(new Instructions.tokinst(Bytecode.newobj) { tok = tok });
        return this;
    }
}

//public class ILBuilder{
//    BasicBlock insertPoint;
//    public ILBuilder(BasicBlock basicBlock){
//        insertPoint = basicBlock;
//    }

//    void setInsertPoint(BasicBlock basicBlock){
//        insertPoint = basicBlock;
//    }

//    BasicBlock getInsertPoint() {
//        return insertPoint;
//    }


//}
