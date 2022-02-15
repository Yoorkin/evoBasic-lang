//
// Created by yorkin on 1/12/22.
//

#include "runtime.h"
#include <execution/intrinsic.h>

namespace evoBasic::vm{
    Runtime *NameSpace::find(std::string name) {
        auto target = childs.find(name);
        if(target == childs.end())return nullptr;
        else return target->second;
    }

    NameSpace::NameSpace(TokenTable *table) : Runtime(table) {}

    const std::map<std::string, Runtime *> &NameSpace::getChilds() {
        return childs;
    }

    void TokenTable::loadCache(data::u64 idx) {
        Runtime *target = context;
        auto fullname = tokens[idx]->getFullName();
        auto first = fullname.front();
        fullname.pop_front();
        if(first == "array"){
            auto size = stoi(fullname.front());
            fullname.pop_front();
            fullname.pop_front(); // pop "global"
            for(auto name : fullname){
                target = ((NameSpace*)target)->find(name);
            }
            cache[idx] = new Array(target,size);
        }
        else if(first == "global"){
            for(auto name : fullname){
                target = ((NameSpace*)target)->find(name);
            }
            cache[idx] = target;
        }
        NotNull(cache[idx]);
    }

    TokenTable::TokenTable(evoBasic::vm::RuntimeContext *context, std::vector<il::TokenDef*> ls)
        : context(context),tokens(ls.begin(),ls.end()),cache(ls.size(),nullptr){}


    std::optional<RuntimeContext::NameRuntimePair> RuntimeContext::collectSymbolRecursively(TokenTable *table, il::Member *member){
        auto name = member->getNameToken()->getDef()->getName();
        switch(member->getKind()){
            case il::MemberKind::Class:{
                auto cls = dynamic_cast<il::Class*>(member);
                auto ret = new vm::Class(table, cls);
                for(auto sub_member : cls->getMembers()){
                    auto sub_pair = collectSymbolRecursively(table,sub_member);
                    if(sub_pair.has_value()) ret->childs.insert(sub_pair.value());
                }
                return {{name,ret}};
            }
            case il::MemberKind::Module:{
                auto mod = dynamic_cast<il::Module*>(member);
                auto ret = new vm::Module(table, mod);
                for(auto sub_member : mod->getMembers()){
                    auto sub_pair = collectSymbolRecursively(table,sub_member);
                    if(sub_pair.has_value()) ret->childs.insert(sub_pair.value());
                }
                return {{name,ret}};
            }
            case il::MemberKind::Enum:{
                auto em = dynamic_cast<il::Enum*>(member);
                auto ret = new vm::Enum(table, em);
                return {{name,ret}};
            }
            case il::MemberKind::Record:{
                auto record = dynamic_cast<il::Record*>(member);
                auto ret = new vm::Record(table, record);
                return {{name,ret}};
            }
            case il::MemberKind::SFtn:
                return {{name,new Function(table, dynamic_cast<il::FunctionDefine*>(member))}};
            case il::MemberKind::Ftn:
                return {{name,new Function(table, dynamic_cast<il::FunctionDefine*>(member))}};
            case il::MemberKind::Ctor:
                return {{"#ctor",new Function(table, dynamic_cast<il::FunctionDefine*>(member))}};
            case il::MemberKind::Ext:{
                auto ext = dynamic_cast<il::Ext*>(member);
                auto library = ext->getLibraryToken()->getDef()->getName();
                auto alias = ext->getAliasToken()->getDef()->getName();
                Runtime *ret = nullptr;
                if(library != "intrinsic"){
                    ret = new ForeignFunction(library,alias,ext);
                    return {{name,ret}};
                }
                else{
                    return {};
                }
            }
            case il::MemberKind::Interface:
            case il::MemberKind::InterfaceFunction:
            case il::MemberKind::Fld:
            case il::MemberKind::SFld:
            case il::MemberKind::VFtn:
                return {};
        }
    }

    void RuntimeContext::collectDependencies(Runtime *parent, Runtime *current, Dependencies<Class*> &inherit, Dependencies<Record*> &include){
        switch(current->getKind()){
            case RuntimeKind::Class: {
                auto cls = dynamic_cast<vm::Class*>(current);
                auto base = cls->table->getRuntime<Class>(cls->il_info->getExtendClassToken()->getID());
                cls->base_class = base;
                inherit.addDependent(cls,base);
                break;
            }
            case RuntimeKind::Module:
                for(auto [_,child] : ((Module*)current)->childs){
                    collectDependencies(current,child,inherit,include);
                }
                break;
            case RuntimeKind::Record: {
                auto record = dynamic_cast<Record*>(current);
                if(parent->getKind() == RuntimeKind::Record){
                    include.addDependent(dynamic_cast<Record*>(parent),record);
                }
                else{
                    include.addIsolate(record);
                }

                for(auto [_,child] : dynamic_cast<Record*>(current)->childs){
                    collectDependencies(record,child,inherit,include);
                }
                break;
            }
            case RuntimeKind::Enum:
            case RuntimeKind::Interface:
            case RuntimeKind::Function:
            case RuntimeKind::VirtualFtnSlot:
            case RuntimeKind::FieldSlot:
            case RuntimeKind::StaticFieldSlot:
                break;
        }
    }

    void RuntimeContext::recordFieldsResolution(Record *record) {
        for(auto member : record->il_info->getMembers()){
            auto fld = dynamic_cast<il::Fld*>(member);
            auto fld_slot = new FieldSlot(record->size,fld);
            record->childs.insert({fld->getNameToken()->getDef()->getName(),fld_slot});
            auto fld_size = record->table->getRuntime<Sizeable>(fld->getTypeToken()->getID());
            record->size += fld_size->getByteLength();
        }
    }

    void RuntimeContext::classFieldsAndVTableResolution(Class *cls) {
        cls->vtable = cls->base_class->vtable;
        for(auto member : cls->il_info->getMembers()){
            auto name = member->getNameToken()->getDef()->getName();
            Runtime *runtime = nullptr;
            switch(member->getKind()){
                case il::MemberKind::SFld: {
                    auto sfld = dynamic_cast<il::SFld*>(member);
                    runtime = new StaticFieldSlot(cls,cls->size,sfld);
                    cls->size += cls->table->getRuntime<Sizeable>(sfld->getTypeToken()->getID())->getByteLength();
                    break;
                }
                case il::MemberKind::Fld: {
                    auto fld = dynamic_cast<il::Fld*>(member);
                    runtime = new FieldSlot(cls->size,fld);
                    cls->size += cls->table->getRuntime<Sizeable>(fld->getTypeToken()->getID())->getByteLength();
                    break;
                }
                case il::MemberKind::VFtn: {
                    auto vftn = dynamic_cast<il::VFtn*>(member);
                    auto vftn_slot = dynamic_cast<VirtualFtnSlot*>(cls->base_class->find(name));
                    if(vftn_slot){
                        runtime = new VirtualFtnSlot(vftn_slot->offset,vftn);
                        cls->vtable[vftn_slot->offset] = new Function(cls->table, vftn);
                    }
                    else{
                        runtime = new VirtualFtnSlot(cls->vtable.size(),vftn);
                        cls->vtable.push_back(new Function(cls->table, vftn));
                    }
                    break;
                }
                case il::MemberKind::SFtn:
                case il::MemberKind::Ftn: {
                    auto ftn = dynamic_cast<il::FunctionDefine*>(member);
                    runtime = new Function(cls->table, ftn);
                    break;
                }
            }
            cls->childs.emplace(name,runtime);
        }
    }

    void RuntimeContext::fillModuleStaticField(Runtime *runtime) {
        if(runtime->getKind() == RuntimeKind::Module){
            auto mod = dynamic_cast<Module*>(runtime);
            data::u64 static_field_size = 0;
            for(auto member : mod->il_info->getMembers()) {
                switch (member->getKind()) {
                    case il::MemberKind::SFld: {
                        auto sfld = dynamic_cast<il::SFld*>(member);
                        auto slot = new StaticFieldSlot(mod, static_field_size, sfld);
                        auto sfld_name = sfld->getNameToken()->getDef()->getName();
                        mod->childs.insert({sfld_name,slot});
                        static_field_size += mod->table->getRuntime<Sizeable>(sfld->getTypeToken()->getID())->getByteLength();
                        break;
                    }
                }
            }

            mod->static_field_memory = (char*)malloc(static_field_size);

            for(auto [_,child] : mod->childs){
                fillModuleStaticField(child);
            }
        }
    }
    void RuntimeContext::collectDetailRecursively(Runtime *runtime) {
        switch(runtime->getKind()){
            case RuntimeKind::Module: {
                for(auto [_,child] : dynamic_cast<Module*>(runtime)->childs){
                    collectDetailRecursively(child);
                }
                break;
            }
            case RuntimeKind::Class: {
                for(auto [_,child] : dynamic_cast<Class*>(runtime)->childs){
                    collectDetailRecursively(child);
                }
                break;
            }
            case RuntimeKind::Context:
                for(auto [_,child] : dynamic_cast<RuntimeContext*>(runtime)->childs){
                    collectDetailRecursively(child);
                }
                break;
            case RuntimeKind::Function:{
                auto function = dynamic_cast<Function*>(runtime);
                //TODO ?
                break;
            }
            case RuntimeKind::VirtualFtnSlot:
                break;
            case RuntimeKind::ForeignFunction:
                break;


            case RuntimeKind::Enum:
                break;
            case RuntimeKind::Interface:
                break;
            case RuntimeKind::FieldSlot:
                break;
            case RuntimeKind::StaticFieldSlot:
                break;
            case RuntimeKind::Record:
                break;
        }
    }

    RuntimeContext::RuntimeContext(std::list<il::Document*> &documents) : NameSpace(nullptr) {

        std::vector<std::pair<std::string,vm::BuiltIn*>> builtin_list = {
            {"boolean",new BuiltIn(BuiltInKind::boolean)},
            {"byte",new BuiltIn(BuiltInKind::i8)},
            {"short",new BuiltIn(BuiltInKind::i16)},
            {"integer",new BuiltIn(BuiltInKind::i32)},
            {"long",new BuiltIn(BuiltInKind::i64)},
            {"single",new BuiltIn(BuiltInKind::f32)},
            {"double",new BuiltIn(BuiltInKind::f64)},
            {"u8",new BuiltIn(BuiltInKind::u8)},
            {"u32",new BuiltIn(BuiltInKind::u32)},
            {"u64",new BuiltIn(BuiltInKind::u64)}
        };

        for(auto builtin : builtin_list) childs.insert(builtin);

        std::list<std::pair<TokenTable*,il::Document*>> tmp;
        for(auto document : documents){
            auto t = new TokenTable(this,document->getTokens());
            token_tables.push_back(t);
            tmp.emplace_back(t,document);
        }

        for(auto [token_table,document] : tmp){
            for(auto member : document->getMembers()){
                auto child = collectSymbolRecursively(token_table,member);
                if(child.has_value()){
                    this->childs.insert(child.value());
                }
            }
        }

        Dependencies<Class*> inherit_dependencies;
        Dependencies<Record*> include_dependencies;

        for(auto [_,runtime] : childs){
            collectDependencies(this,runtime,inherit_dependencies,include_dependencies);
        }

        include_dependencies.solve();
        for(auto record : include_dependencies.getTopologicalOrder()){
            recordFieldsResolution(record);
        }

        for(auto [_,runtime] : childs){
            fillModuleStaticField(runtime);
        }

        inherit_dependencies.solve();
        for(auto cls : inherit_dependencies.getTopologicalOrder()){
            classFieldsAndVTableResolution(cls);
        }

        collectDetailRecursively(this);
    }


    Runtime::Runtime(TokenTable *table) : table(table) {}

    TokenTable *Runtime::getTokenTable() {
        return table;
    }

    data::u64 Sizeable::getByteLength() {
        return size;
    }

    Function::Function(TokenTable *table, il::FunctionDefine *info)
        : Runtime(table),il_info(info) {}

    data::Byte *Function::getBlock() {
        return il_info->getBlocksMemory();
    }


    const std::vector<Runtime*> &Function::getParams() {
        if(params_frame_size==-1){
            params_frame_size = 0;
            for(auto param : il_info->getParams()){
                auto param_rt = table->getRuntime<Runtime>(param->getTypeToken()->getDef()->getID());
                params_offset.push_back(params_frame_size);
                //判断是否是引用参数
                auto param_length = param->isRef() ? sizeof(char*) : dynamic_cast<Sizeable*>(param_rt)->getByteLength();
                params_frame_size += param_length;
                params_length.push_back(param_length);
                params.push_back(param_rt);
            }
        }
        return params;
    }

    const std::vector<Runtime *> &Function::getLocals() {
        auto base = getParamsStackFrameLength();
        if(locals_frame_size==-1){
            locals_frame_size = 0;
            for(auto local : il_info->getLocals()){
                auto local_rt = table->getRuntime<Runtime>(local->getTypeToken()->getDef()->getID());
                locals_offset.push_back(locals_frame_size + base);
                locals_frame_size += dynamic_cast<Sizeable*>(local_rt)->getByteLength();
                locals.push_back(local_rt);
            }
        }
        return locals;
    }

    data::u64 Function::getParamsStackFrameLength() {
        if(params_frame_size==-1)getParams();
        return params_frame_size;
    }

    data::u64 Function::getLocalsStackFrameLength() {
        if(locals_frame_size==-1)getLocals();
        return locals_frame_size;
    }

    data::u64 Function::getParamOffset(data::u16 index) {
        if(params_frame_size==-1)getParams();
        return params_offset[index];
    }

    data::u64 Function::getLocalOffset(data::u16 index) {
        if(locals_frame_size==-1)getLocals();
        return locals_offset[index];
    }

    data::u64 Function::getStackFrameLength() {
        return getLocalsStackFrameLength() + getParamsStackFrameLength();
    }

    const std::vector<data::u16> &Function::getParamsLength() {
        if(params_frame_size==-1)getParams();
        return params_length;
    }

    ForeignFunction::ForeignFunction(std::string library, std::string name, il::Ext *info)
        : Runtime(nullptr),name(name),il_info(info){}

    VirtualFtnSlot::VirtualFtnSlot(data::u64 offset, il::VFtn *info)
        : Runtime(nullptr),offset(offset),il_info(info){}

    data::u64 VirtualFtnSlot::getOffset() {
        return offset;
    }

    FieldSlot::FieldSlot(data::u64 offset, il::Fld *info)
        : Runtime(nullptr),offset(offset),il_info(info){}

    data::u64 FieldSlot::getOffset() {
        return offset;
    }

    StaticFieldSlot::StaticFieldSlot(Module *owner,data::u64 offset, il::SFld *info)
            : Runtime(nullptr),offset(offset),il_info(info),owner(owner){}

    StaticFieldSlot::StaticFieldSlot(Class *owner,data::u64 offset, il::SFld *info)
        : Runtime(nullptr),offset(offset),il_info(info),owner(owner){}

    data::Byte *StaticFieldSlot::getAddress() {
        switch(owner->getKind()){
            case RuntimeKind::Module:
                return dynamic_cast<Module*>(owner)->address<data::Byte*>(this);
            case RuntimeKind::Class:
                return dynamic_cast<Class*>(owner)->address<data::Byte*>(this);
        }
    }

    Module::Module(TokenTable *table, il::Module *info)
        : NameSpace(table), il_info(info){}

    Module::~Module() {
        if(static_field_memory)
            free(static_field_memory);
    }

    Record::Record(TokenTable *table, il::Record *info)
        : NameSpace(table),il_info(info){}


    Class::Class(TokenTable *table, il::Class *info)
        : NameSpace(table),il_info(info){}

    Runtime *Class::find(std::string name) {
         auto target = NameSpace::find(name);
         if(target)return target;
         else return base_class->find(name);
    }

    Function *Class::virtualFunctionDispatch(VirtualFtnSlot slot) {
        return vtable[slot.getOffset()];
    }

    Class::~Class() {
        if(static_field_memory)
            free(static_field_memory);
    }

    Enum::Enum(TokenTable *table, il::Enum *info)
        : NameSpace(table),il_info(info){}

    BuiltInKind BuiltIn::getBuiltInKind() {
        return kind;
    }

    data::u64 getBuiltInSize(BuiltInKind kind){
        switch(kind){
            case BuiltInKind::boolean: return sizeof(data::boolean);
            case BuiltInKind::u8:      return sizeof(data::u8);
            case BuiltInKind::u16:     return sizeof(data::u16);
            case BuiltInKind::u32:     return sizeof(data::u32);
            case BuiltInKind::u64:     return sizeof(data::u64);
            case BuiltInKind::i8:      return sizeof(data::i8);
            case BuiltInKind::i16:     return sizeof(data::i16);
            case BuiltInKind::i32:     return sizeof(data::i32);
            case BuiltInKind::i64:     return sizeof(data::i64);
            case BuiltInKind::f32:     return sizeof(data::f32);
            case BuiltInKind::f64:     return sizeof(data::f64);
        }
    }

    BuiltIn::BuiltIn(BuiltInKind kind)
        : Runtime(nullptr),kind(kind){
        size = getBuiltInSize(kind);
    }

    Array::Array(Runtime *element, data::u64 count)
            : Runtime(nullptr),element_type(element),count(count){
        size = count * dynamic_cast<Sizeable*>(element)->getByteLength();
    }

    Runtime *Array::getElementRuntime() {
        return element_type;
    }

    data::u64 Array::getElementCount() {
        return count;
    }
}

