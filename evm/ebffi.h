#ifndef EVM_FFI
#define EVM_FFI
#include "runtime.h"
#include "utils.h"

#include <ffi.h>
#include <string>
#include <stdexcept>

class Processor;

class ForeignFunctionInterface;

class FFIModuleNotFoundException : public std::runtime_error{
public: 
	std::string missing_library;
	explicit FFIModuleNotFoundException(std::string lib) 
		: std::runtime_error(lib + " not found"), missing_library(lib){}
};

class FFIEntryNotFoundException : public std::runtime_error{
public:
	std::string missing_entry,library;
	explicit FFIEntryNotFoundException(std::string lib,std::string entry)
		: std::runtime_error(entry + " not found"), library(library), missing_entry(entry){}
};

class FFIEntry {
public:
	friend ForeignFunctionInterface;
	using Key = std::pair<std::string,std::string>;
private:
	Key key;
	ffi_cif cif;
	void *entry_point;
	const std::vector<runtime::Parameter*> &parameters;
	runtime::Symbol *return_type;

	void *loadEntryPoint(std::string &library, std::string &entry_name);
	void freeEntryPoint(void *fn);

	inline ffi_type *map_type(runtime::EvaluationKind eval,const runtime::Symbol *type){
		if(eval == runtime::EvaluationKind::Byref){
			return &ffi_type_pointer;
		}
		else{ // byref
			if(auto klass = dynamic_cast<const runtime::Class*>(type)){
				return &ffi_type_pointer;
			}
			else if(auto pmt = dynamic_cast<const runtime::Primitive*>(type)){
				using enum runtime::PrimitiveKind;
				switch(pmt->getKind()){
				case Boolean:   return &ffi_type_uint8;
				case Byte:      return &ffi_type_uint8;
				case Short:     return &ffi_type_sint16;
				case UShort:    return &ffi_type_uint16;
				case Rune:		return &ffi_type_uint32;
				case Integer:   return &ffi_type_sint32;
				case UInteger:  return &ffi_type_uint32;
				case Long:      return &ffi_type_sint64;
				case ULong:     return &ffi_type_uint64;
				case Single:    return &ffi_type_float;
				case Double:    return &ffi_type_double;
				case Void:		return &ffi_type_void;
				default:        throw "";
				}
			}
			else if(auto em = dynamic_cast<const runtime::Enumeration*>(type)){
				return &ffi_type_sint32;
			}
			else{
				throw "";
			}
		}
	}

	explicit inline FFIEntry(Key key, const std::vector<runtime::Parameter*> &parameters, runtime::Symbol *return_type) 
		: cif(cif),parameters(parameters),return_type(return_type){

		entry_point = loadEntryPoint(key.first, key.second);

		ffi_type **args_type = new ffi_type*[parameters.size()];
		for(int i = 0; i<parameters.size(); i++){
			args_type[i] = map_type(parameters[i]->getEvalKind(), parameters[i]->getType());
		}

		auto ret_type = map_type(runtime::EvaluationKind::Byval, return_type);
		if(ffi_prep_cif(&cif, FFI_DEFAULT_ABI, parameters.size(), ret_type, args_type) != FFI_OK){
			throw "";
		}

		delete[] args_type;
	}

	~FFIEntry(){
		freeEntryPoint(entry_point);
	}

public:

	inline Key getKey(){ return key; }

	void call(Processor *processor);
	
};

class ForeignFunctionInterface {
	std::map<FFIEntry::Key,std::pair<int,FFIEntry*>> entries;
public:
	inline FFIEntry *load(std::string library, std::string entry_name, const std::vector<runtime::Parameter*> &parameters, runtime::Symbol *return_type){
		auto key = std::make_pair(library,entry_name);
		auto target = entries.find(key);
		if(target!=entries.end()){
			target->second.first++;
			return target->second.second;
		}
		else{
			auto entry = new FFIEntry(std::make_pair(library, entry_name), parameters, return_type);
			entries.insert({key,{0, entry}});
			return entry;
		}
	}

	void call(runtime::ForeignEntry *fe, Processor *processor);

	inline void free(FFIEntry *entry){
		auto target = entries.find(entry->getKey());
		if(target == entries.end()) throw "";
		target->second.first--;
		if(target->second.first==0){
			delete entry;
		}
	}
};



#endif