//
// Created by yorkin on 11/21/21.
//

#ifndef EVOBASIC_COMPILEHELPER_H
#define EVOBASIC_COMPILEHELPER_H
#include "ir.h"
#include "lexer.h"
#include "parser.h"
#include "token.h"
#include "context.h"
#include "codeGen.h"
#include "logger.h"
#include "strstream"
//using namespace std;
//using namespace evoBasic;
//using namespace evoBasic::ir;
//
//IR *compile(std::string code){
//    Logger::debugMode = false;
//    auto source = new evoBasic::StringSource(code);
//    Lexer lexer(source);
//    Parser parser(&lexer);
//    auto ast = parser.parseGlobal();
//
//    auto context = new Context;
//    Semantic::collectSymbol(ast,context);
//    Semantic::collectDetail(ast,context);
//    Semantic::typeCheck(ast,context);
//
//    if(Logger::errorCount == 0){
//        Semantic::solveByteLengthDependencies(context);
//        IRGen gen;
//        return gen.gen(ast,context);
//    }
//    return nullptr;
//}
//
//bool is_inst_eq(Instruction *a,Instruction *b){
//    stringstream sa,sb;
//    a->toString(sa);
//    b->toString(sb);
//    return (sa.str() == sb.str());
//}
//
//bool is_block_eq(Block *a,Block *b){
//    auto al = a->getInstructions().size();
//    auto bl = a->getInstructions().size();
//    if(al!=bl)return false;
//    for(int i=0;i<al;i++){
//        auto ai = a->getInstructions()[i];
//        auto bi = b->getInstructions()[i];
//        if(!is_inst_eq(ai,bi))return false;
//    }
//    return true;
//}
//


#endif //EVOBASIC_COMPILEHELPER_H
