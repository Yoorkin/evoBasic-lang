//
// Created by yorkin on 11/7/21.
//

#include "invoke.h"
#include<ffi.h>
#include<map>
#include"bytecode.h"
using namespace std;
using DataKind = evoBasic::vm::Data;

#ifdef linux
#include<dlfcn.h>

//namespace evoBasic::vm{
//
//    class LinuxForeignFunction : public ForeignFunction{
//        void *function = nullptr;
//        std::vector<DataKind> args_type;
//        DataKind ret_type;
//        map<DataKind,ffi_type*> type_map = {
//            {DataKind::void_,&ffi_type_void},
//            {DataKind::boolean,&ffi_type_schar},
//            {DataKind::i8,&ffi_type_schar},
//            {DataKind::i16,&ffi_type_sint16},
//            {DataKind::i32,&ffi_type_sint32},
//            {DataKind::i64,&ffi_type_sint64},
//            {DataKind::u8,&ffi_type_uchar},
//            {DataKind::u16,&ffi_type_uint16},
//            {DataKind::u32,&ffi_type_uint32},
//            {DataKind::u64,&ffi_type_uint64},
//            {DataKind::f32,&ffi_type_float},
//            {DataKind::f32,&ffi_type_double}
//        };
//        using ffi_type_ptr = ffi_type*;
//        using value_ptr = void*;
//        ffi_cif cif;
//        ffi_type_ptr *ffi_args_array = nullptr;
//        value_ptr *value_array = nullptr;
//    public:
//        LinuxForeignFunction(string name,std::vector<DataKind> args_type, DataKind ret_type,void *handle)
//            : args_type(move(args_type)),ret_type(move(ret_type)){
//            function = dlsym(handle,name.c_str());
//            ffi_cif cif;
//            using ffi_type_ptr = ffi_type*;
//            using value_ptr = void*;
//            ffi_type_ptr *ffi_args_array = new ffi_type_ptr[args_type.size()];
//            value_ptr *value_array = new value_ptr[args_type.size()];
//
//            for(auto i = args_type.size()-1;i>=0;i--){
//                auto type = args_type[i];
//                ffi_args_array[i] = type_map[type];
//            }
//            ffi_type_ptr ffi_ret = type_map[ret_type];
//
//            ffi_prep_cif(&cif,FFI_DEFAULT_ABI,args_type.size(),ffi_ret,ffi_args_array);
//        }
//
//        void invoke(Stack *operand) override {
//            value_ptr *value_array = new value_ptr[args_type.size()];
//            char *ptr = (char*)operand->getPtr();
//            for(auto i = 0;i < args_type.size();i++){
//                auto type = args_type[i];
//                ptr-=data::Data::getSize(type);
//                value_array[i] = ptr;
//            }
//            ffi_arg result;
//            ffi_call(&cif,FFI_FN(function),&result,value_array);
//
//            delete value_array;
//
//            for(auto type : args_type){
//                switch (ret_type) {
//                    case data::Data::boolean:
//                        //operand->pop<data::boolean>()
//                        break;
//                    case data::Data::i8:
//                        break;
//                    case data::Data::i16:
//                        break;
//                    case data::Data::i32:
//                        break;
//                    case data::Data::i64:
//                        break;
//                    case data::Data::f32:
//                        break;
//                    case data::Data::f64:
//                        break;
//                    case data::Data::u8:
//                        break;
//                    case data::Data::u16:
//                        break;
//                    case data::Data::u32:
//                        break;
//                    case data::Data::u64:
//                        break;
//                }
//            }
//
//
//            switch (ret_type) {
//                case data::Data::void_:
//                    break;
//                case data::Data::boolean:
//                    break;
//                case data::Data::i8:
//                    break;
//                case data::Data::i16:
//                    break;
//                case data::Data::i32:
//                    break;
//                case data::Data::i64:
//                    break;
//                case data::Data::f32:
//                    break;
//                case data::Data::f64:
//                    break;
//                case data::Data::u8:
//                    break;
//                case data::Data::u16:
//                    break;
//                case data::Data::u32:
//                    break;
//                case data::Data::u64:
//                    break;
//            }
//        }
//    };
//
//    class LinuxLibrary : public Library{
//        void *handle = nullptr;
//    public:
//        explicit LinuxLibrary(string lib){
//            handle = dlopen(lib.c_str(),RTLD_NOW);
//        }
//
//        ForeignFunction *loadFunction(std::string name, std::vector<DataKind> args_type, DataKind ret_type)override{
//            return new LinuxForeignFunction(move(name),move(args_type),move(ret_type),handle);
//        }
//    };
//
//
//
//}

#elif defined(_win32)


#endif

namespace evoBasic::vm {

    Library *Invoke::loadLibrary(std::string lib) {
//#ifdef linux
//        return new LinuxLibrary(move(lib));
//#elif defined(_win32)
//        return nullptr;
//#endif
    }
}
