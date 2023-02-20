#ifndef EVM_LOADER
#define EVM_LOADER
#include <exception>
#include <stdexcept>
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <queue>
#include <typeinfo>
#include <optional>

#include "backage.pb.h"
#include "dependencies.h"
#include "gc.h"
#include "interop.h"
#include "runtime.h"
#include "utils.h"
#include "unicode.h"
#include "ebffi.h"

class Loader;

class Token{
    uint32_t token_id;
public:
    inline explicit Token(uint32_t id) : token_id(id){}
    virtual unicode::string toString() = 0;
};

class TextToken : public Token{
    unicode::string text;
public:
    inline unicode::string &getText(){ return text; }
    inline unicode::string toString()override{
        return text;
    }
    inline TextToken(uint32_t id, unicode::string text) : Token(id), text(text){}
};

class ConstructedToken : public Token{
    std::vector<uint32_t> subtoken_id;
public:
    inline int getSubTokenCount(){ return subtoken_id.size(); }
    inline uint32_t getSubToken(int index){ return subtoken_id[index]; }
    inline unicode::string toString()override{
        unicode::string ret;
        for(auto i : subtoken_id){
            ret += unicode::to_string(i);
        }
        return ret;
    }
    inline ConstructedToken(uint32_t id, std::vector<uint32_t> subtoken_id) : Token(id), subtoken_id(subtoken_id){}
};

class ArrayToken : public Token{
    uint32_t element_type_token_id;
public:
    inline uint32_t getElementTypeTokenID(){ return element_type_token_id; }
    inline unicode::string toString()override{
        return "arrayToken"_utf32;
    }
    inline ArrayToken(uint32_t id, uint32_t element_type_token_id) : Token(id), element_type_token_id(element_type_token_id){}
};


class TokenTable{
protected:
    std::vector<runtime::Symbol*> cache;

private:
    Loader &loader;
    std::vector<Token*> tokens;

    runtime::Symbol *search(Token *token);
public:
    runtime::Symbol *query(uint32_t token_id){
        if(token_id == 0)throw std::invalid_argument("token cannot be zero");

        if(cache[token_id-1]==nullptr)
            return cache[token_id-1] = search(tokens[token_id-1]);
        else
            return cache[token_id-1];
    }

    inline Token *getToken(uint32_t token_id){ return tokens[token_id-1]; }

    TokenTable(Loader &loader,std::vector<Token*> tokens)
        : loader(loader), cache(tokens.size() + 1,nullptr), tokens(tokens){}
};

class SpecializedArrayPool{
    std::map<runtime::Symbol*,runtime::SpecializedArray*> array_map;
    runtime::Class *array_base_class = nullptr;
    Loader &loader;
    bool instant_complete = false;
public:
    runtime::SpecializedArray *query(runtime::Symbol* type);

    inline void setArrayBaseClass(runtime::Class *array_base){
        if(array_base==nullptr || !runtime::isNamedArray(array_base)) 
            throw std::invalid_argument("invalid array base class");
        array_base_class = array_base;
    }

    inline void completeAll(){
        instant_complete = true;
        for(auto [_,ary] : array_map) ary->complete();
    }

    explicit SpecializedArrayPool(Loader &loader) : loader(loader){}

    ~SpecializedArrayPool(){
        for(auto [_,ary] : array_map) delete ary;
    }
};

class Loader{
    unicode::string package_folder;

    std::map<unicode::string,Backage::Package*> packages;
    std::map<Backage::Package*,TokenTable*> token_tables;

    interop::Agent *interop_agent;
    GarbageCollector *gc;
    ForeignFunctionInterface *ffi;

    SpecializedArrayPool specialized_array_pool;
    runtime::Global *global = nullptr;
    runtime::Class *eb_object = nullptr,
                    *eb_string = nullptr,
                    *eb_array = nullptr,
                    *eb_null_pointer_exception = nullptr,
                    *eb_conversion_exception = nullptr,
                    *eb_out_of_range_exception = nullptr,
                    *eb_option_missing_exception = nullptr,
                    *eb_evm_internal_exception = nullptr,
                    *eb_divide_by_zero_exception = nullptr,
                    *eb_object_unpinned_exception = nullptr,
                    *eb_ffi_entry_not_found_exception = nullptr,
                    *eb_ffi_module_not_found_exception = nullptr;

    runtime::Symbol *createSymbol(Backage::Declaration &decl, TokenTable &table);
    
public:

    inline SpecializedArrayPool *getSpecilizedArrayPool(){
        return &specialized_array_pool;
    }

    inline runtime::Global *getGlobal(){
        return global;
    }

    inline interop::Agent *getInteropAgent(){
        return interop_agent;
    }

    inline GarbageCollector *getGC(){ 
        return gc; 
    }

    inline ForeignFunctionInterface *getFFI(){
        return ffi;
    }

    inline runtime::Class *getEBObject(){ return eb_object; }
    inline runtime::Class *getEBString(){ return eb_string; }
    inline runtime::Class *getEBArray(){ return eb_array; }
    inline runtime::Class *getEBNullPointerException(){ return eb_null_pointer_exception; }
    inline runtime::Class *getEBConverstionException(){ return eb_conversion_exception; }
    inline runtime::Class *getEBOutOfRangeException(){ return eb_out_of_range_exception; }
    inline runtime::Class *getEBOptionMissingException(){ return eb_option_missing_exception; }
    inline runtime::Class *getEBEvmInternalException(){ return eb_evm_internal_exception; }
    inline runtime::Class *getEBDivideByZeroException(){ return eb_divide_by_zero_exception; }
    inline runtime::Class *getEBObjectUnpinnedException(){ return eb_object_unpinned_exception; }
    inline runtime::Class *getEBFFIModuleNotFoundException(){ return eb_ffi_module_not_found_exception; }
    inline runtime::Class *getEBFFIEntryNotFoundException(){ return eb_ffi_entry_not_found_exception; }

    void fromPath(unicode::string package_path);
    void fromPackageFolder(unicode::string package_name);

    void load();

    explicit Loader(unicode::string package_folder);

};

#endif