#include "ebffi.h"
#include "processor.h"

#ifdef _WIN32

#include<Windows.h>
std::map<void*,HMODULE> resources;
void *FFIEntry::loadEntryPoint(std::string &library, std::string &entry_name){
	HMODULE mod = LoadLibraryA(library.c_str());
	if(mod==nullptr) throw FFIModuleNotFoundException(library);
	auto fn = (void*)GetProcAddress(mod,entry_name.c_str());
	if(fn==nullptr) throw FFIEntryNotFoundException(library, entry_name);
	resources.insert({fn,mod});
	return fn;
}

void FFIEntry::freeEntryPoint(void *fn){
	auto target = resources.find(fn);
	FreeLibrary(target->second);
	resources.erase(fn);
}

void FFIEntry::call(Processor *processor){
	std::vector<std::string> temp_cstr;
	std::vector<uint8_t*> temp_ptr;
	void **values = new void*[parameters.size()];
	ffi_arg result;
	for(int i = 0; i<parameters.size(); i++){
		if(parameters[i]->getEvalKind() == runtime::EvaluationKind::Byval){
			if(runtime::instancesOf<runtime::Class>(parameters[i]->getType())){
				processor->OpRemoveRoot<interop::Instance*>();
				auto ins = processor->getOperand().pop<interop::Instance*>();
				if(ins->klass == processor->getLoader().getEBString()){
					auto str = processor->getLoader().getInteropAgent()->fetchStringFromInstance((interop::StringInstance*)ins);
					auto cstr = unicode::toPlatform(str);
					temp_cstr.push_back(cstr);
					temp_ptr.push_back((uint8_t*)temp_cstr.back().c_str());
					values[i] = &temp_ptr.back();
				}
				else if(isSubtypeOf(ins->klass,processor->getLoader().getEBArray())){
					temp_ptr.push_back((uint8_t*)ins + sizeof(interop::ArrayInstance));
					values[i] = &temp_ptr.back();
				}
			}
			else{
				values[i] = (void*)processor->getOperand().popAndGetTop(parameters[i]->getLength());
			}
		}
		else { // byref
			processor->OpRemoveRoot<interop::Instance*>();
			auto itp = processor->getOperand().pop<interop::InteriorPointer>();
			temp_ptr.push_back(itp.ptr);
			values[i] = &temp_ptr.back();
		}
	}
	ffi_call(&cif, (void(*)())entry_point, &result, values);

	bool has_result = true;
	if(auto pmt = dynamic_cast<runtime::Primitive*>(return_type)){
		has_result = pmt->getKind() != runtime::PrimitiveKind::Void;
	}
	if(has_result) processor->getOperand().pushFromPtr((uint8_t*)&result, runtime::getRuntimeSize(return_type));
	delete[] values;
}


void ForeignFunctionInterface::call(runtime::ForeignEntry *fe, Processor *processor){
	try{
		if(fe->getEntry()==nullptr){
			fe->setEntry(load(fe->getLib().value(), fe->getAlias().value_or(unicode::toPlatform(fe->name)), fe->getNormalParameters(), fe->getReturnType()));
		}
		fe->getEntry()->call(processor);
	}
	catch(FFIEntryNotFoundException &e){
		auto klass = processor->getLoader().getEBFFIEntryNotFoundException();
		auto library = processor->getLoader().getInteropAgent()->createString(unicode::fromPlatform(e.library));
		auto entry = processor->getLoader().getInteropAgent()->createString(unicode::fromPlatform(e.missing_entry));
		auto ins = processor->getLoader().getInteropAgent()->createInstance(klass, {
			interop::Value::fromRef(library), 
			interop::Value::fromRef(entry)
			});
		processor->handleException(ins);
	}
	catch(FFIModuleNotFoundException &e){
		auto klass = processor->getLoader().getEBFFIModuleNotFoundException();
		auto library = processor->getLoader().getInteropAgent()->createString(unicode::fromPlatform(e.missing_library));
		auto ins = processor->getLoader().getInteropAgent()->createInstance(klass, {
			interop::Value::fromRef(library)
			});
		processor->handleException(ins);
	}
}

#elif __linux__

//todo

#endif