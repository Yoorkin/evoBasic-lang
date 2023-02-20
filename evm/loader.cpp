#include "loader.h"
#include "gc.h"
#include "interop.h"
#include "processor.h"
#include "backage.pb.h"
#include "dependencies.h"
#include "runtime.h"
#include "unicode.h"
#include <fstream>
#include <stdexcept>
#include <vector>


namespace fs = std::filesystem;


runtime::SpecializedArray *SpecializedArrayPool::query(runtime::Symbol* type){
    auto target = array_map.find(type);
    if(target != array_map.end()) return target->second;
    auto spec_ary = new runtime::SpecializedArray(*type, array_base_class);
    array_map.insert({type, spec_ary});
    if(instant_complete) spec_ary->complete();
    return spec_ary;
}


runtime::Symbol *TokenTable::search(Token *token){
    if(auto text_token = dynamic_cast<TextToken*>(token)){
        auto target = loader.getGlobal()->find(text_token->getText());
        if(target==nullptr) {
            std::cout<<unicode::toPlatform(text_token->getText()) + " not found"<<std::endl;
            throw std::invalid_argument(unicode::toPlatform(text_token->getText()) + " not found");
        }
        return target;
    }
    else if(auto ctd_token = dynamic_cast<ConstructedToken*>(token)){
        runtime::Symbol *prv_sym = loader.getGlobal();
        for(int i=0; i<ctd_token->getSubTokenCount(); i++){
            auto sub_token_id = ctd_token->getSubToken(i);
            auto next_name = dynamic_cast<TextToken*>(tokens[sub_token_id-1])->getText();
            prv_sym = dynamic_cast<runtime::Scope*>(prv_sym)->find(next_name);
        }
        return prv_sym;
    }
    else if(auto ary_token = dynamic_cast<ArrayToken*>(token)){
        auto ret = search(tokens[ary_token->getElementTypeTokenID()-1]);
        return loader.getSpecilizedArrayPool()->query(ret);
    }
    
    throw std::invalid_argument("unexpected token definition");
}



Loader::Loader(unicode::string package_folder) : package_folder(package_folder), specialized_array_pool(*this) {
    global = new runtime::Global();
    interop_agent = new interop::Agent(this);
    gc = new GarbageCollector(2048);
    ffi = new ForeignFunctionInterface();

    using enum runtime::PrimitiveKind;
    for(auto x : {Void,Boolean,Byte,Short,UShort,Rune,Integer,UInteger,Long,ULong,Single,Double})
        global->add(new runtime::Primitive(x));
}

runtime::Symbol *Loader::createSymbol(Backage::Declaration &decl, TokenTable &table){
    if(decl.has_moduledecl()){
        auto name = dynamic_cast<TextToken*>(table.getToken(decl.moduledecl().nametoken()))->getText();
        std::list<runtime::Symbol*> childern;
        for(auto child : decl.moduledecl().declarations())
            childern.push_back(createSymbol(child,table));
        return new runtime::Module(name,table,childern);
    }
    else if(decl.has_classdecl()){
        auto name = dynamic_cast<TextToken*>(table.getToken(decl.classdecl().nametoken()))->getText();
        std::list<runtime::Symbol*> childern;
        for(auto child : decl.classdecl().declarations())
            childern.push_back(createSymbol(child,table));
        return new runtime::PackageDefinedClass(name,table,decl.classdecl(),childern);
    }
    else if(decl.has_enumdecl()){
        auto name = dynamic_cast<TextToken*>(table.getToken(decl.enumdecl().nametoken()))->getText();
        std::list<runtime::Symbol*> childern;
        for(auto constant : decl.enumdecl().constants()){
            auto constant_name = dynamic_cast<TextToken*>(table.getToken(constant.nametoken()))->getText();
            auto constant_sym = new runtime::EnumConstant(constant_name,table,constant);
            childern.push_back(constant_sym);
        }
        return new runtime::Enumeration(name,table,decl.enumdecl(),childern);
    }
    else if(decl.has_recorddecl()){
        auto name = dynamic_cast<TextToken*>(table.getToken(decl.recorddecl().nametoken()))->getText();
        std::list<runtime::Symbol*> childern;
        for(auto child : decl.recorddecl().declarations())
            childern.push_back(createSymbol(child,table));
        return new runtime::Record(name,table,decl.recorddecl(),childern);
    }
    else if(decl.has_varaibledecl()){
        auto name = dynamic_cast<TextToken*>(table.getToken(decl.varaibledecl().nametoken()))->getText();
        return new runtime::Variable(name,table,decl.varaibledecl());
    }
    else if(decl.has_methoddecl()){
        auto name = dynamic_cast<TextToken*>(table.getToken(decl.methoddecl().nametoken()))->getText();
        return new runtime::Method(name,table,decl.methoddecl());
    }
    else if(decl.has_foreignentrydecl()){
        auto name = dynamic_cast<TextToken*>(table.getToken(decl.foreignentrydecl().nametoken()))->getText();
        LOG(Loader,"ffi:"<<name<<std::endl);
        return new runtime::ForeignEntry(name,table,decl.foreignentrydecl());
    }
    else if(decl.has_ctordecl()){
        return new runtime::Ctor(table,decl.ctordecl());
    }

    throw std::invalid_argument("unexpected declaration in Loader::createSymbol");
}

void Loader::fromPath(unicode::string package_target) {
    LOG(Loader, "load bkg:" << package_target << std::endl);
    std::fstream stream(unicode::toPlatform(package_target), std::ios::in | std::ios::binary);
    auto package = new Backage::Package;
    if (stream.is_open() && package->ParseFromIstream(&stream)) {
        auto identity = unicode::fromPlatform(package->identity());
        if (!packages.contains(identity)) {
            packages.insert(std::make_pair(identity, package));
            for (auto depend : package->dependencies()) {
                std::cout << depend.text() << std::endl;
                fromPackageFolder(unicode::fromPlatform(depend.text()));
            }
        }
        else {
            //todo
        }
    }
    else {
        throw std::invalid_argument("failed to load package '" + unicode::toPlatform(package_target) + "'");
    }
}

void Loader::fromPackageFolder(unicode::string package_name){
    unicode::string target = package_folder + "/"_utf32 + package_name + ".bkg"_utf32;
    fromPath(target);
}

Token *loadTokenFromTokenDecl(const Backage::TokenDefinition &def){
    if(def.has_texttoken()){
        return new TextToken(def.id(), unicode::fromUTF8(def.texttoken().text()));
    }
    else if(def.has_constructedtoken()){
        std::vector<uint32_t> sub_token_ids;
        for(auto i : def.constructedtoken().tokens()){
            sub_token_ids.push_back(i);
        }
        return new ConstructedToken(def.id(), sub_token_ids);
    }
    else if(def.has_arraytoken()){
        return new ArrayToken(def.id(), def.arraytoken().typetoken());
    }
    else {
        throw std::invalid_argument("unexpected token type");
    }
}

void Loader::load(){
    using namespace runtime;
    for(auto [_,package] : packages){
        std::vector<Token*> tokens;
        for(auto token_decl : package->tokens()){
            tokens.push_back(loadTokenFromTokenDecl(token_decl));
        }

        auto table = new TokenTable(*this, tokens);
        for(auto child : package->declarations()){
            global->add(createSymbol(child, *table));
        }
    }

    auto array_base_class = dynamic_cast<runtime::Class*>(global->find("Array"_utf32));
    specialized_array_pool.setArrayBaseClass(array_base_class);

    TopologicalSort<Symbol*> inherit_dependencies;
    TopologicalSort<Record*> size_dependencies;

    std::queue<Symbol*> sym_queue;
    sym_queue.push(global);
    while(!sym_queue.empty()){
        auto sym = sym_queue.front();
        sym_queue.pop();
        LOG(Loader,"SYM: "<<sym->name<<std::endl);
        if(auto gbl = dynamic_cast<Global*>(sym)){
            for(auto [_,child] : gbl->getChildern())
                sym_queue.push(child);
        }
        else if(auto mod = dynamic_cast<Module*>(sym)){
            for(auto [_,child] : mod->getChildern())
                sym_queue.push(child);
        }
        else if(auto cls = dynamic_cast<Class*>(sym)){
            for(auto child : cls->getDependencies()){
                inherit_dependencies.addEdge(child, sym);
            }
        }
        else if(auto rcd = dynamic_cast<Record*>(sym)){
            size_dependencies.addVex(rcd);
            for(auto [_,child] : rcd->getChildern()){
                auto variable = dynamic_cast<Variable*>(child);
                if(auto child_rcd = dynamic_cast<Record*>(variable->getType())){
                    size_dependencies.addEdge(child_rcd,rcd);
                }
            }
        }
    } 

    eb_object = dynamic_cast<runtime::Class*>(global->find("Object"_utf32));
    eb_string = dynamic_cast<runtime::Class*>(global->find("String"_utf32));
    eb_array = dynamic_cast<runtime::Class*>(global->find("Array"_utf32));
    eb_null_pointer_exception = dynamic_cast<runtime::Class*>(global->find("NullPointerException"_utf32));
    eb_conversion_exception = dynamic_cast<runtime::Class*>(global->find("ConversionException"_utf32));
    eb_out_of_range_exception = dynamic_cast<runtime::Class*>(global->find("OutOfRangeException"_utf32));
    eb_option_missing_exception = dynamic_cast<runtime::Class*>(global->find("OptionMissingException"_utf32));
    eb_evm_internal_exception = dynamic_cast<runtime::Class*>(global->find("EvmInternalException"_utf32));
    eb_divide_by_zero_exception = dynamic_cast<runtime::Class*>(global->find("DivideByZeroException"_utf32));
    eb_object_unpinned_exception = dynamic_cast<runtime::Class*>(global->find("ObjectUnpinned"_utf32));
    eb_ffi_entry_not_found_exception = dynamic_cast<runtime::Class*>(global->find("FFIEntryNotFoundException"_utf32));
    eb_ffi_module_not_found_exception = dynamic_cast<runtime::Class*>(global->find("FFIModuleNotFoundException"_utf32));

    if(!inherit_dependencies.solve()){
        throw "inherit resolve error";
    }

    if(!size_dependencies.solve()){
        throw "size resolve error";
    }

    //complete class
    LOG(Loader,"class:"<<std::endl);
    for(auto cls : inherit_dependencies.getOrder()){
        
        cls->complete();
        LOG(Loader, cls->name << '(' << dynamic_cast<Class*>(cls)->getInstanceMemorySize() << "," 
                << dynamic_cast<Class*>(cls)->getStaticMemorySize()
                << ") -> "<<std::endl); 
    }

    //complete record
    LOG(Loader,"record:"<<std::endl);
    for(auto rcd : size_dependencies.getOrder()){
        rcd->complete();
        LOG(Loader, rcd->name << "(" << rcd->getMemorySize() << ") -> "<<std::endl);
    }

    //complete specialized array
    getSpecilizedArrayPool()->completeAll();

    //complete module\enum\global\static method
    sym_queue.push(global);
    while(!sym_queue.empty()){
        auto sym = sym_queue.front();
        sym_queue.pop();
        if(instancesOf<Module>(sym) || instancesOf<Enumeration>(sym) || instancesOf<Global>(sym)){
            sym->complete();
            for(auto [_,child] : dynamic_cast<Scope*>(sym)->getChildern()){
                sym_queue.push(child);
            }
        }
        else if(instancesOf<Function>(sym)){
            sym->complete();
        }
    }
}