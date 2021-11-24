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
using namespace evoBasic::ast;
using namespace evoBasic::ast::expr;
namespace evoBasic{


    ExpressionType *ExpressionType::Error = new ExpressionType(make_shared<type::Error>(),error);

    void Semantic::collectSymbol(AST *ast, std::shared_ptr<Context> context) {
        SymbolCollector collector;
        SymbolCollectorArgs args;
        args.domain = context->getGlobal();
        collector.visitGlobal(ast,args);
    }

    void Semantic::collectDetail(AST *ast, std::shared_ptr<Context> context) {
        DetailCollector collector;
        BaseArgs args;
        args.domain = context->getGlobal();
        args.context = context;
        collector.visitGlobal(ast,args);
    }

    void Semantic::typeCheck(AST *ast, std::shared_ptr<Context> context) {
        TypeAnalyzer analyzer;
        BaseArgs args;
        args.domain = context->getGlobal();
        args.context = context;
        analyzer.visitGlobal(ast,args);
    }

    bool Semantic::solveTypeInferenceDependencies(std::shared_ptr<Context> context) {
        return false;
    }

    bool Semantic::solveByteLengthDependencies(std::shared_ptr<Context> context) {
        if(context->byteLengthDependencies.solve()){
            Logger::dev("update memory layout topo order: ");
            for(auto &domain : context->byteLengthDependencies.getTopologicalOrder()){
                domain->updateMemoryLayout();
                Logger::dev(format()<<" -> "<<domain->mangling()<<"{"<<domain->getByteLength()<<"}");
            }
            Logger::dev("\n");
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
        }

        return false;
    }


//    std::any visitID(ast::expr::ID *id_node, BaseArgs args){
//        auto name = getID(id_node);
//
//        if(!args.dot_expression_context){
//            auto target = args.domain->lookUp(name)->as_shared<Prototype>();
//            if(!target){
//                Logger::error(id_node->location,"object not find");
//                return ExpressionType::Error;
//            }
//            return new ExpressionType(target,ExpressionType::lvalue);
//        }
//        else{
//            auto domain = args.dot_expression_context->as_shared<Domain>();
//            shared_ptr<Prototype> target;
//            if(domain && (target = domain->find(name)->as_shared<Prototype>())){
//                return new ExpressionType(target,ExpressionType::lvalue);
//            }
//            else{
//                Logger::error(id_node->location,"object not find");
//                return ExpressionType::Error;
//            }
//        }
//    }



}
