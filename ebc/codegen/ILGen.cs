namespace evoBasic.IL;
using evoBasic.AST;

class ILGen{
    CompileEnv env;
    TokenDict dict;
    Symbols.ExecuableFunction function;
    DataType refTyp = new DataType{ kind = DataType.Kind.Ref },
            hdlTyp = new DataType{ kind = DataType.Kind.Hdl },
            u16Typ = new DataType{kind = DataType.Kind.U16},
            u32Typ = new DataType{kind = DataType.Kind.U32},
            i32Typ = new DataType{kind = DataType.Kind.I32},
            u8Typ = new DataType{kind = DataType.Kind.U8};

    struct LoopBlkPair{
        public BasicBlock afterBlk;
        public BasicBlock loopBlk;
        public LoopBlkPair(BasicBlock loop,BasicBlock after){
            this.loopBlk = loop;
            this.afterBlk = after;
        }
    }

    Stack<LoopBlkPair> whileBlkStack = new Stack<LoopBlkPair>(),
                    forBlkStack = new Stack<LoopBlkPair>(),
                    loopBlkStack = new Stack<LoopBlkPair>();

    public ILGen(CompileEnv env){
        this.env = env;
    }

    public Blocks FromASTBlock(Symbols.ExecuableFunction function,TokenDict dict){
        this.dict = dict;
        this.function = function;

        // 设置函数参数Index
        UInt16 index = 1;

        foreach(var parameter in function.normalParameters)
            parameter.setIndex(index++);

        foreach(var parameter in function.optionParameters)
            parameter.setIndex(index++);
        
        function.paramArray?.setIndex(index);

        var begin = new BasicBlock("begin");
        if(function is Symbols.Constructor ctor && ctor.ownerClass != env.EBObject){
            begin = VisitBaseCtorCallExpr(ctor.extendCtorAST, begin);
        }
        var end = VisitStatements(function.astBlock,begin);
        end.ret();


        var blocks = new Blocks(begin);

        //将IL转换成二进制格式
        MemoryStream memStream = new MemoryStream();
        BinaryWriter binWriter = new BinaryWriter(memStream);
        blocks.writeBytecodeToStream(binWriter);
        function.bytecodes = memStream.ToArray();

        return blocks;
    }

    BasicBlock VisitStatements(Block block,BasicBlock current){
        foreach(var stmt in block.statements){
            current.markNewline(stmt.line);
            current = VisitStatement(stmt,current);
        }
        return current;
    } 

    BasicBlock VisitStatement(Statement statement,BasicBlock current) => statement switch{
        DimStmt s => VisitDimStmt(s,current),
        Loop s => VisitLoopStmt(s,current),
        If s => VisitIfStmt(s,current),
        For s => VisitForStmt(s,current),
        Throw s => VisitThrowStmt(s,current),
        Return s => VisitReturnStmt(s,current),
        ExprStmt s => VisitExprStmt(s,current),
        Exit s => VisitExitStmt(s,current),
        Continue s => VisitContinueStmt(s,current),
        TryStmt s => VisitTryStmt(s,current),
    };

    BasicBlock VisitDimStmt(DimStmt statement,BasicBlock current){
        foreach(var (variable,initExpr) in statement.variables){
            variable.setIndex((UInt16)(function.localIndex.Count() + 1));
            var typTok = dict.QueryToken(variable.type);
            function.localIndex.Add(new Tuple<Symbols.Variable, UInt32>(variable,typTok.id));

            if(initExpr != null){
                current = LoadValueOfExpression(initExpr, current);
                current.push(u16Typ, variable.getIndex())
                       .stloc(mapType(variable.type));
            }
        }
        return current;
    }

    BasicBlock VisitTryStmt(TryStmt statement,BasicBlock current){
        // 收集异常捕获信息
        var catchInfo = new List<Tuple<BasicBlock,TokenDict.Token,CatchPart>>();
        foreach(var catchPart in statement.catchParts){
            var varIndex = (UInt16)(function.localIndex.Count() + 1);
            catchPart.exceptionVariable.setIndex(varIndex);
            function.localIndex.Add(
                new Tuple<Symbols.Variable, uint>(
                    catchPart.exceptionVariable, 
                    dict.QueryToken(catchPart.exceptionVariable.type).id
                )
            );
            
            var catchBlk = new BasicBlock("catch_blk " + catchPart.exceptionVariable.type.getFullName());
            var exceptionClassTok = dict.QueryToken(catchPart.exceptionVariable.type);
            catchInfo.Add(new Tuple<BasicBlock, TokenDict.Token, CatchPart>(catchBlk,exceptionClassTok,catchPart));
            current.addTryCatchHandler(catchBlk, exceptionClassTok);
        }

        // 处理tryPart
        var afterBlk = new BasicBlock("after_try_blk");
        
        foreach(var (catchBlk, tok, catchPart) in (catchInfo as IEnumerable<Tuple<BasicBlock, TokenDict.Token, CatchPart>>).Reverse()){
            current.enter(tok, catchBlk);
        }

        current = VisitStatements(statement.tryBlock, current);

        foreach(var (catchBlk,tok,catchPart) in catchInfo){
            current.leave(tok);
        }

        current.br(afterBlk);
        
        // 处理catchPart
        foreach(var (catchBlk, _, catchPart) in catchInfo){
            // 将虚拟机推入操作栈的异常实例指针取出，存放到本地变量exceptionVariable中
            catchBlk.push(u16Typ, catchPart.exceptionVariable.getIndex())
                    .stloc(refTyp);
            var afterCatchBlk = VisitStatements(catchPart.catchBlock, catchBlk);
            afterCatchBlk.br(afterBlk);
        }
        return afterBlk;
    }


    BasicBlock VisitLoopStmt(Loop statement,BasicBlock current){
        var loop_blk = new BasicBlock("while_loop_blk "+statement.condition.location.text);
        // 循环中发生跳转时，loop_blk指向的块被修改
        // 因此需要保存循环顶部条件判断部分的基本块，为继续循环提供跳转目标地址
        var loop_cond_blk = loop_blk; 
        var after_blk = new BasicBlock("after_while_loop_blk "+statement.condition.location.text);

        //跳转到条件判断
        current.br(loop_blk);

        //条件判断
        loop_blk = VisitExpression(statement.condition, loop_blk);
        loop_blk.not()
                .jif(after_blk);

        //循环内指令
        var loopPair = new LoopBlkPair(loop_blk, after_blk);
        whileBlkStack.Push(loopPair);
        loopBlkStack.Push(loopPair);
        loop_blk = VisitStatements(statement.block, loop_blk);
        whileBlkStack.Pop();
        loopBlkStack.Pop();
        loop_blk.br(loop_cond_blk);

        return after_blk;
    }

    BasicBlock VisitIfStmt(If statement,BasicBlock current){
        var afterBlk = new BasicBlock("after_if_blk");
        Case elseCaseAST = null;
        foreach(var caseAST in statement.cases){
            if(caseAST.condition == null){
                elseCaseAST = caseAST;
                continue;
            }
            var caseBlk = new BasicBlock("if_case_blk");
            current = LoadValueOfExpression(caseAST.condition, current);
            current.jif(caseBlk);

            caseBlk = VisitStatements(caseAST.block, caseBlk);
            caseBlk.br(afterBlk);
        }

        if(elseCaseAST!=null){
            current = VisitStatements(elseCaseAST.block, current);
        }
        current.br(afterBlk);
        
        return afterBlk;
    }

    BasicBlock LoadValueOfExpression(Expression expression,BasicBlock current){
        current = VisitExpression(expression, current);
        DataType typ;
        if(expression is Assign assign){
            typ = mapType(expression.type);
            LoadValueOfExpression(assign.lhs, current);
        }
        else if(expression is LocalAccess){
            typ = mapType(expression.type);
            current.ldloc(typ);
        }
        else if(expression is ArgumentAccess argumentAccess){
            typ = mapType(expression.type);
            if(argumentAccess.isRef){
                current.ldarg(hdlTyp)
                    .load(typ);
            }
            else{
                current.ldarg(typ);
            }
        }
        else if(expression is FldAccess fldAccess){
            typ = mapType(expression.type);
            current.ldfld(typ, dict.QueryToken(fldAccess.variable));
        }
        else if(expression is SFldAccess sfldAccess){
            typ = mapType(expression.type);
            current.ldsfld(typ, dict.QueryToken(sfldAccess.variable));
        }
        else if(expression is Subscript){
            typ = mapType(expression.type);
            current.ldelem(typ,dict.QueryToken(expression.type));
        }
        else if(expression is OptionalTest){
            throw new NotImplementedException();
        }
        return current;
    }

    BasicBlock LoadAddressOfExpression(Expression expression,BasicBlock current){
        current = VisitExpression(expression, current);

        if(expression is Assign assign){
            LoadAddressOfExpression(assign.lhs, current);
        }
        else if(expression is LocalAccess){
            current.ldloca();
        }
        else if(expression is ArgumentAccess argumentAccess){
            if(argumentAccess.isRef){
                current.ldarg(hdlTyp);
            }
            else{
                current.ldarga();
            }
        }
        else if(expression is FldAccess fldAccess){
            current.ldflda(dict.QueryToken(fldAccess.variable));
        }
        else if(expression is SFldAccess sfldAccess){
            current.ldsflda(dict.QueryToken(sfldAccess.variable));
        }
        else if(expression is Subscript subscript){
            current.ldelema(dict.QueryToken(subscript.type));
        }
        else{
            throw new ArgumentException();
        }

        return current;
    }

    BasicBlock VisitForStmt(For statement,BasicBlock current){
        UInt16 index = (UInt16)(function.localIndex.Count() + 1);
        statement.begVar.setIndex(index);
        function.localIndex.Add(new Tuple<Symbols.Variable, UInt32>(statement.begVar,dict.QueryToken(statement.begVar.type).id));
        
        index = (UInt16)(function.localIndex.Count() + 1);
        statement.endVar.setIndex(index);
        function.localIndex.Add(new Tuple<Symbols.Variable, UInt32>(statement.endVar,dict.QueryToken(statement.endVar.type).id));
        
        index = (UInt16)(function.localIndex.Count() + 1);
        statement.sepVar.setIndex(index);
        function.localIndex.Add(new Tuple<Symbols.Variable, UInt32>(statement.sepVar,dict.QueryToken(statement.sepVar.type).id));
        
        if(statement.iteratorHasDim){
            statement.iterVar.setIndex((UInt16)(function.localIndex.Count() + 1));
            var typTok = dict.QueryToken(statement.iterVar.type);
            function.localIndex.Add(new Tuple<Symbols.Variable, UInt32>(statement.iterVar,typTok.id));
        }

        var iterTyp = mapType(statement.iterator.type);

        //加载beg
        current = LoadValueOfExpression(statement.begin, current);
        current.push(u16Typ, statement.begVar.getIndex())
            .stloc(iterTyp);

        //加载end
        current = LoadValueOfExpression(statement.end, current);
        current.push(u16Typ, statement.endVar.getIndex())
            .stloc(iterTyp);

        //加载step
        if(statement.step!=null){
            current = LoadValueOfExpression(statement.step, current);
            current.push(u16Typ, statement.sepVar.getIndex())
                .stloc(iterTyp);
        }
        else{
            current.push(i32Typ, 1)
                .push(u16Typ, statement.sepVar.getIndex())
                .stloc(iterTyp);
        }
        
        // iterator = beg
        current.push(u16Typ, statement.begVar.getIndex())
            .ldloc(iterTyp);
        current = LoadAddressOfExpression(statement.iterator,current);
        current.store(iterTyp);


        var loop_blk = new BasicBlock("for_loop_blk");
        // 循环中发生跳转时，loop_blk指向的块被修改
        // 因此需要保存循环顶部条件判断部分的基本块，为继续循环提供跳转目标地址
        var loop_cond_blk = loop_blk; 
        var after_blk = new BasicBlock("for_after_blk");

        //jump to cond_blk
        current.br(loop_blk);
        
        loop_blk = LoadValueOfExpression(statement.iterator, loop_blk);
        loop_blk.push(u16Typ, statement.begVar.getIndex())
                .ldloc(i32Typ)
                .push(u16Typ, statement.endVar.getIndex())
                .ldloc(i32Typ)
                .push(u16Typ, statement.sepVar.getIndex())
                .ldloc(i32Typ);
        loop_blk.callintrinsic(dict.QueryToken("IsIteratorNotInRange"))
                .jif(after_blk);

        var loopPair = new LoopBlkPair(loop_blk,after_blk);
        forBlkStack.Push(loopPair);
        loopBlkStack.Push(loopPair);
        loop_blk = VisitStatements(statement.block, loop_blk);
        forBlkStack.Pop();
        loopBlkStack.Pop();

        loop_blk = LoadValueOfExpression(statement.iterator, loop_blk);
        loop_blk.push(i32Typ, 1)
                .add(i32Typ);

        loop_blk = LoadAddressOfExpression(statement.iterator, loop_blk);
        loop_blk.store(i32Typ);

        loop_blk.br(loop_cond_blk);

        return after_blk;
    }

    BasicBlock VisitThrowStmt(Throw statement,BasicBlock current){
        LoadValueOfExpression(statement.expression, current);
        current.throw_();
        return current;
    }

    BasicBlock VisitReturnStmt(Return statement,BasicBlock current){
        LoadValueOfExpression(statement.expression, current);
        current.ret();
        return current;
    }

    BasicBlock VisitExitStmt(Exit statement,BasicBlock current){
        switch(statement.flag){
            case Exit.Flag.While:
                current.br(whileBlkStack.Peek().afterBlk);
                break;
            case Exit.Flag.Sub:
                current.ret();
                break;
            case Exit.Flag.For:
                current.br(forBlkStack.Peek().afterBlk);
                break;
        }
        return current;
    }

    BasicBlock VisitContinueStmt(Continue statement,BasicBlock current){
        current.br(loopBlkStack.Peek().loopBlk);
        return current;
    }

    BasicBlock VisitExprStmt(ExprStmt statement,BasicBlock current){
        current = LoadValueOfExpression(statement.expression, current);
        if (!statement.expression.type.Equal(env.Void)) {
            var typ = mapType(statement.expression.type);
            current.pop(typ);
        }
        return current;
    }

    BasicBlock VisitExpression(Expression expression,BasicBlock current){
        switch(expression){
            case Unary x: return VisitUnaryExpr(x, current);
            case Binary x: return VisitBinaryExpr(x, current);
            case Assign x: return VisitAssignExpr(x, current);
            case Convert x: return VisitConvertExpr(x, current);
            case Is x: return VisitIsExpr(x, current);
            case OptionalTest x: return VisitOptionalTestExpr(x, current);
            case FtnCall x: return VisitFtnCallExpr(x, current);
            case VFtnCall x: return VisitVFtnCallExpr(x, current);
            case SFtnCall x: return VisitSFtnCallExpr(x, current);
            case ForeignCall x: return VisitForeignCallExpr(x, current);
            case CtorCall x: return VisitCtorCallExpr(x, current);
            case IntrinsicCall x: return VisitIntrinsicCallExpr(x, current);
            case NewArray x: return VisitNewArrayExpr(x, current);
            case LocalAccess x: return VisitLocalAccessExpr(x, current);
            case ArgumentAccess x: return VisitArgumentAccessExpr(x, current);
            case FldAccess x: return VisitFldAccessExpr(x, current);
            case SFldAccess x: return VisitSFldAccessExpr(x, current);
            case EnumConst x: return VisitEnumConstExpr(x, current);
            case Subscript x: return VisitSubscriptExpr(x, current);
            case ArrayLiteral x: return VisitArrayLiteral(x, current);
            case IntegerLiteral x: return VisitIntegerLiteral(x, current);
            case DoubleLiteral x: return VisitDoubleLiteral(x, current);
            case StringLiteral x: return VisitStringLiteral(x, current);
            case RuneLiteral x: return VisitRuneLiteral(x, current);
            case BooleanLiteral x: return VisitBooleanLiteral(x, current);
            case NothingLiteral x: return VisitNothingLiteral(x, current);
            default: throw new ArgumentException();
        }
    }

    BasicBlock VisitAssignExpr(Assign assign,BasicBlock current){
        current = LoadValueOfExpression(assign.rhs, current);
        current = VisitExpression(assign.lhs, current);
        var typ = mapType(assign.lhs.type);
        switch(assign.lhs){
            case LocalAccess: return current.stloc(typ);
            case ArgumentAccess argAccess: 
                if(argAccess.isRef) return current.ldarg(hdlTyp)
                                                  .store(typ);
                else return current.starg(typ);
            case FldAccess fldAccess:
                return current.stfld(typ,dict.QueryToken(fldAccess.variable));
            case SFldAccess sfldAccess:
                return current.stsfld(typ,dict.QueryToken(sfldAccess.variable));
            case Subscript subscript:
                return current.stelem(typ,dict.QueryToken(subscript.type));
        }
        throw new ArgumentException();
    }

    BasicBlock VisitConvertExpr(Convert convExpr,BasicBlock current){
        current = LoadValueOfExpression(convExpr.expression, current);

        if(convExpr.target is Symbols.Class){
            var srcTok = dict.QueryToken(convExpr.expression.type);
            var dstTok = dict.QueryToken(convExpr.target);
            current.castClass(srcTok, dstTok);
        }
        else{
            var srcTyp = mapType(convExpr.expression.type);
            var dstTyp = mapType(convExpr.target);
            current.convert(srcTyp, dstTyp);
        }

        return current;
    }

    BasicBlock VisitIsExpr(Is isExpr,BasicBlock current){
        current = LoadValueOfExpression(isExpr.expression, current);
        switch(isExpr.expression){
            case Unary:
            case Binary:
            case Assign:
            case Convert:
            case FtnCall:
            case VFtnCall:
            case SFtnCall:
            case ForeignCall:
            case CtorCall:
            case NewArray:
            case LocalAccess:
            case ArgumentAccess:
            case FldAccess:
            case SFldAccess:
            case Subscript:
                return current.instanceof(dict.QueryToken(isExpr.target));
            default:
                throw new ArgumentException();
        }
    }

    BasicBlock VisitOptionalTestExpr(OptionalTest optionalTest, BasicBlock current){
        foreach(var option in optionalTest.optionalParameters){
            current.push(u16Typ, option.getIndex());
        }
        current.push(u8Typ, (Byte)optionalTest.optionalParameters.Count)
                .testopt();
        return current;
    }

    BasicBlock VisitBinaryExpr(Binary binary,BasicBlock current){
        LoadValueOfExpression(binary.lhs, current);
        LoadValueOfExpression(binary.rhs, current);
        var dataType = mapType(binary.lhs.type);
        return binary.op switch{
            Op.And => current.and(),
            Op.Xor => current.xor(),
            Op.Or  => current.or(),
            Op.GT  => current.gt(dataType),
            Op.GE  => current.ge(dataType),
            Op.EQ  => current.eq(dataType),
            Op.LE  => current.le(dataType),
            Op.LT  => current.lt(dataType),
            Op.PLUS=> current.add(dataType),
            Op.MINUS=> current.sub(dataType),
            Op.MUL => current.mul(dataType),
            Op.DIV => current.div(dataType),
            Op.NE => current.ne(dataType),
            Op.Mod => current.mod(dataType),
            _ => throw new ArgumentException()
        };
    }

    DataType mapType(Symbols.Type type) => type switch{
        Symbols.Array => new DataType{ kind = DataType.Kind.Ref },
        Symbols.Class => new DataType{ kind = DataType.Kind.Ref },
        Symbols.Enumeration => new DataType{ kind = DataType.Kind.EmConst },
        Symbols.Record => new DataType{ kind = DataType.Kind.Record, token = dict.QueryToken(type) },
        Symbols.Primitive pmt => new DataType{ 
            kind = pmt.kind switch{
                Symbols.PrimitiveKind.Boolean => DataType.Kind.Boolean,
                Symbols.PrimitiveKind.i8 => DataType.Kind.I8,
                Symbols.PrimitiveKind.i16 => DataType.Kind.I16,
                Symbols.PrimitiveKind.i32 => DataType.Kind.I32,
                Symbols.PrimitiveKind.i64 => DataType.Kind.I64,
                Symbols.PrimitiveKind.u8 => DataType.Kind.U8,
                Symbols.PrimitiveKind.u16 => DataType.Kind.U16,
                Symbols.PrimitiveKind.u32 => DataType.Kind.U32,
                Symbols.PrimitiveKind.u64 => DataType.Kind.U64,
                Symbols.PrimitiveKind.f32 => DataType.Kind.F32,
                Symbols.PrimitiveKind.f64 => DataType.Kind.F64,
                Symbols.PrimitiveKind.rune => DataType.Kind.U32,
                Symbols.PrimitiveKind.ptr => DataType.Kind.Ptr,
                _ => throw new ArgumentException()
            }
        },
        _ => throw new ArgumentException()
    };

    BasicBlock VisitUnaryExpr(Unary unary,BasicBlock current){
        current = LoadValueOfExpression(unary.expression, current);
        switch(unary.op){
            case Op.MINUS:
                return current.neg(mapType(unary.expression.type));
            case Op.Not:
                return current.not();
        }
        throw new ArgumentException();
    }

    BasicBlock VisitArgumentPass(Call call, BasicBlock current){
        for(int i = call.args.Count()-1; i>=0;i--){
            var normalArg = call.args[i];
            if(normalArg.isRef){
                current = LoadAddressOfExpression(normalArg.expression, current);
            }
            else{
                current = LoadValueOfExpression(normalArg.expression, current);
            }
        }

        foreach(var opt in call.opts){
            current = VisitOptInit(opt, current);
        }
       
        if(call.function.optionParameters.Count()>0) current.push(u8Typ, (Byte)call.opts.Count());

        if(call.paramArrayPass!=null){
            if(call.paramArrayPass.forward!=null){
                current = LoadValueOfExpression(call.paramArrayPass.forward, current);
            }
            else if(call.paramArrayPass.expressions!=null){
                var arrayLength = call.paramArrayPass.expressions.Count();
                current.push(i32Typ, arrayLength)
                        .newarray(dict.QueryToken(env.EBObject));
                int index = 0;
                foreach(var expression in call.paramArrayPass.expressions){
                    current.dup(refTyp)
                            .push(i32Typ, index++);
                    current = LoadValueOfExpression(expression, current);
                    current.stelemr(refTyp, dict.QueryToken(env.EBObject));
                }
            }
        }

        return current;
    }

    BasicBlock VisitFtnCallExpr(FtnCall call,BasicBlock current){
        current = LoadValueOfExpression(call.reference, current);
        current = VisitArgumentPass(call, current);
        return current.ldftn(dict.QueryToken(call.function))
                    .callmethod();
    }

    BasicBlock VisitVFtnCallExpr(VFtnCall call,BasicBlock current){
        current = LoadValueOfExpression(call.reference, current);
        current = VisitArgumentPass(call, current);
        return current.ldvftn(dict.QueryToken(call.function))
                    .callvirtual();
    }

    BasicBlock VisitSFtnCallExpr(SFtnCall call,BasicBlock current){
        current = VisitArgumentPass(call, current);
        return current.ldsftn(dict.QueryToken(call.function))
                    .callstatic();
    }

    BasicBlock VisitForeignCallExpr(ForeignCall call,BasicBlock current){
        current = VisitArgumentPass(call, current);
        return current.ldforeign(dict.QueryToken(call.function))
                    .callforeign();
    }

    BasicBlock VisitCtorCallExpr(CtorCall call,BasicBlock current){
        current.newobj(dict.QueryToken(call.type))
               .dup(refTyp);
        current = VisitArgumentPass(call, current);
        return current.ldctor(dict.QueryToken(call.function))
                    .callctor();
    }

    BasicBlock VisitBaseCtorCallExpr(BaseCtorCall call,BasicBlock current){
        current.push(u16Typ, (UInt16)0)
               .ldarg(refTyp);
        current = VisitArgumentPass(call, current);
        return current.ldctor(dict.QueryToken(call.function))
                    .callctor();
    }

    BasicBlock VisitIntrinsicCallExpr(IntrinsicCall call,BasicBlock current){
        return VisitArgumentPass(call, current)
                .callintrinsic(dict.QueryToken(call.function));
    }

    BasicBlock VisitNewArrayExpr(NewArray newArrayExpr,BasicBlock current){
        current = LoadValueOfExpression(newArrayExpr.size, current);
        return current.newarray(dict.QueryToken(newArrayExpr.elementType));
    }

    BasicBlock VisitLocalAccessExpr(LocalAccess localAccess,BasicBlock current){
        return current.push(u16Typ, localAccess.local.getIndex());
    }

    BasicBlock VisitArgumentAccessExpr(ArgumentAccess argAccess,BasicBlock current){
        return current.push(u16Typ, argAccess.parameter.getIndex());
    }

    BasicBlock VisitOptInit(OptInit optInit,BasicBlock current){
        if(optInit.isRef){
            current = LoadAddressOfExpression(optInit.expression, current);
        }
        else{
            current = LoadValueOfExpression(optInit.expression, current);
        }
        return current.ldoptinfo(dict.QueryToken(optInit.parameter));
    }

    BasicBlock VisitFldAccessExpr(FldAccess fldAccess,BasicBlock current){
        if (fldAccess.reference.type is Symbols.Record) {
            var expression = fldAccess.reference;
            current = VisitExpression(expression, current);
            if (expression is Assign assign) {
                current = LoadAddressOfExpression(assign.lhs, current);
                current.push(u8Typ, 2);
            }
            else if(expression is FtnCall || expression is VFtnCall 
                || expression is SFtnCall || expression is ForeignCall) {
                current.push(u8Typ, 3);
            }
            else if (expression is LocalAccess) {
                current.ldloca();
                current.push(u8Typ, 2);
            }
            else if (expression is ArgumentAccess argumentAccess) {
                if (argumentAccess.isRef) {
                    current.ldarg(hdlTyp);
                }
                else {
                    current.ldarga();
                }
                current.push(u8Typ, 2);
            }
            else if (expression is FldAccess inside_fldAccess) {
                current.ldflda(dict.QueryToken(inside_fldAccess.variable));
                current.push(u8Typ, 2);
            }
            else if (expression is SFldAccess sfldAccess) {
                current.ldsflda(dict.QueryToken(sfldAccess.variable));
                current.push(u8Typ, 2);
            }
            else if (expression is Subscript) {
                current.ldelema(dict.QueryToken(expression.type));
                current.push(u8Typ, 2);
            }
            else if (expression is OptionalTest) {
                throw new NotImplementedException();
            }
            return current;
        }
        else {
            current = LoadValueOfExpression(fldAccess.reference, current);
            return current.push(u8Typ, 1);
        }
    }

    BasicBlock VisitSFldAccessExpr(SFldAccess sfldAccess,BasicBlock current){
        return current;
    }

    BasicBlock VisitEnumConstExpr(EnumConst enumConst,BasicBlock current){
        return current.ldenumc(dict.QueryToken(enumConst.constant));
    }

    BasicBlock VisitSubscriptExpr(Subscript subscript,BasicBlock current){
        current = LoadValueOfExpression(subscript.reference, current);
        current = LoadValueOfExpression(subscript.subscript, current);
        return current;
    }

    BasicBlock VisitArrayLiteral(ArrayLiteral arrayLiteral,BasicBlock current){
        current.push(i32Typ, arrayLiteral.expressions.Count());
        TokenDict.Token eleTok = null;
        DataType eleTyp = null;
        if(arrayLiteral.expressions.Count()>0){
            eleTyp = mapType(arrayLiteral.expressions[0].type);
            eleTok = dict.QueryToken(arrayLiteral.expressions[0].type);
        }
        else{
            eleTyp = i32Typ;
            eleTok = dict.QueryToken(env.Integer);
        }
        current.newarray(eleTok);
        
        int index = 0;
        foreach(var expression in arrayLiteral.expressions){
            current.dup(refTyp)
                    .push(i32Typ, index);
            current = LoadValueOfExpression(expression, current);
            current.stelemr(eleTyp,eleTok);
            index++;
        }

        return current;
    }

    BasicBlock VisitIntegerLiteral(IntegerLiteral literal,BasicBlock current){
        return current.push(
            new DataType{ kind = DataType.Kind.I32 },
            literal.value
        );
    }

    BasicBlock VisitDoubleLiteral(DoubleLiteral literal,BasicBlock current){
        return current.push(
            new DataType{ kind = DataType.Kind.F64 },
            literal.value
        );
    }   

    BasicBlock VisitStringLiteral(StringLiteral literal,BasicBlock current){
        var tok = dict.QueryToken(literal.value);
        return current.ldstr(tok);
    }

    BasicBlock VisitRuneLiteral(RuneLiteral literal,BasicBlock current){
        return current.push(
            new DataType{ kind = DataType.Kind.U32 },
            literal.value
        );
    }

    BasicBlock VisitBooleanLiteral(BooleanLiteral literal,BasicBlock current){
        return current.push(
            new DataType{ kind = DataType.Kind.Boolean },
            literal.value
        );
    }

    BasicBlock VisitNothingLiteral(NothingLiteral nothingLiteral,BasicBlock current){
        return current.ldnothing();
    }
}