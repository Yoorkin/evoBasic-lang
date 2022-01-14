//
// Created by yorkin on 11/1/21.
//

#include "semantic.h"
#include "symbolCollector.h"
#include "detailCollector.h"
#include "typeAnalyzer.h"
#include "context.h"

#include <utils/nullSafe.h>
#include <utils/logger.h>

using namespace std;
using namespace evoBasic::type;
using namespace evoBasic::parseTree;
using namespace evoBasic::parseTree::expr;
namespace evoBasic{


    ExpressionType *ExpressionType::Error = new ExpressionType(new type::Error,error);
    ExpressionType *ExpressionType::Void = new ExpressionType(new type::Error,void_);

    void Semantic::collectSymbol(ParseTree *parse_tree, CompileTimeContext *context) {
        SymbolCollector collector;
        SymbolCollectorArgs args;
        args.context = context;
        args.domain = context->getGlobal();
        collector.visitGlobal(parse_tree, args);
    }

    void Semantic::collectDetail(ParseTree *parse_tree, CompileTimeContext *context) {
        DetailCollector collector;
        DetailArgs args;
        args.domain = context->getGlobal();
        args.context = context;
        collector.visitGlobal(parse_tree, args);
    }

    ast::AST *Semantic::typeCheck(ParseTree *parse_tree, CompileTimeContext *context) {
        TypeAnalyzer analyzer;
        TypeAnalyzerArgs args;
        args.domain = context->getGlobal();
        args.context = context;
        return any_cast<ast::Global*>(analyzer.visitGlobal(parse_tree, args));
    }

    bool Semantic::solveTypeInferenceDependencies(CompileTimeContext *context) {
        return false;
    }

    bool Semantic::solveByteLengthDependencies(CompileTimeContext *context) {
        if(context->byteLengthDependencies.solve()){
            Logger::print(Channel::ByteLengthTopologicalOrder,"update memory layout topological order: ");
            for(auto &domain : context->byteLengthDependencies.getTopologicalOrder()){
                domain->updateMemoryLayout();
                Logger::print(Channel::ByteLengthTopologicalOrder,
                              Format() << " -> " << domain->mangling() << "{" << domain->getByteLength() << "}");
            }
            Logger::print(Channel::ByteLengthTopologicalOrder,"\n");
            return true;
        }
        else{
            Format msg;
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

    bool Semantic::solveInheritDependencies(CompileTimeContext *context) {
        list<string> order;
        if(context->inheritDependencies.solve()){
            for(auto &cls : context->inheritDependencies.getTopologicalOrder()){
                cls->generateClassInfo();
                order.push_back(cls->mangling('.'));
            }
            Logger::print(Channel::InheritTopologicalOrder,"inherit topo order: ");
            for(auto name:order){
                Logger::print(Channel::InheritTopologicalOrder," -> ");
                Logger::print(Channel::InheritTopologicalOrder,name);
            }
            Logger::print(Channel::InheritTopologicalOrder,"\n");
            return true;
        }
        else{
            PANIC;
            return false;
        }
    }

}