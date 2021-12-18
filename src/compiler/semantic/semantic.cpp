//
// Created by yorkin on 11/1/21.
//

#include "semantic.h"
#include "nullSafe.h"
#include "logger.h"
#include "utils.h"
#include "symbolCollector.h"
#include "detailCollector.h"
#include "typeAnalyzer.h"

using namespace std;
using namespace evoBasic::type;
using namespace evoBasic::parseTree;
using namespace evoBasic::parseTree::expr;
namespace evoBasic{


    ExpressionType *ExpressionType::Error = new ExpressionType(new type::Error,error,il::empty);
    ExpressionType *ExpressionType::Void = new ExpressionType(new type::Error,void_,il::empty);

    void Semantic::collectSymbol(AST *ast, Context *context) {
        SymbolCollector collector;
        SymbolCollectorArgs args;
        args.context = context;
        args.domain = context->getGlobal();
        collector.visitGlobal(ast,args);
    }

    void Semantic::collectDetail(AST *ast, Context *context) {
        DetailCollector collector;
        DetailArgs args;
        args.domain = context->getGlobal();
        args.context = context;
        collector.visitGlobal(ast,args);
    }

    void Semantic::typeCheck(AST *ast, Context *context) {
        TypeAnalyzer analyzer;
        TypeAnalyzerArgs args;
        args.domain = context->getGlobal();
        args.context = context;
        analyzer.visitGlobal(ast,args);
    }

    bool Semantic::solveTypeInferenceDependencies(Context *context) {
        return false;
    }

    bool Semantic::solveByteLengthDependencies(Context *context) {
        if(context->byteLengthDependencies.solve()){
            Logger::dev("update memory layout topo order: ");
            for(auto &domain : context->byteLengthDependencies.getTopologicalOrder()){
                domain->updateMemoryLayout();
                Logger::dev(format() << " -> " << domain->mangling() << "{" << domain->getByteLength() << "}");
            }
            Logger::dev("\n");
            return true;
        }
        else{
            format msg;
            msg<<"Recursive declaration in Type.The recursive paths is \n";
            for(auto & circle : context->byteLengthDependencies.getCircles()){
                msg<<"\t";
                for(auto & t : circle){
                    msg<<"'"<<t->getName()<<"'";
                    if(&t != &circle.back())msg<<"->";
                }
                msg<<"\n";
            }
            Logger::error(msg);
            return false;
        }
    }

    bool Semantic::solveInheritDependencies(Context *context) {
        list<string> order;
        if(context->inheritDependencies.solve()){
            for(auto &cls : context->inheritDependencies.getTopologicalOrder()){
                cls->generateClassInfo();
                order.push_back(cls->mangling('.'));
            }
            Logger::dev("inherit topo order: ");
            for(auto name:order){
                Logger::dev(" -> ");
                Logger::dev(name);
            }
            Logger::dev("\n");
            return true;
        }
        else{
            PANIC;
            return false;
        }
    }

}
