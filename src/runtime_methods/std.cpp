#include "CrossLang.hpp"
#if defined(CROSSLANG_ENABLE_SHARED)
#if defined(CROSSLANG_ENABLE_FFI)
#include <ffi.h>
#endif
#if defined(_WIN32)
#include <windows.h>

#include <time.h>
#undef min
#undef max
#else
#include <dlfcn.h>
#include <unistd.h>
#endif
#else
#if defined(_WIN32)
#include <windows.h>

#include <time.h>
#undef min
#undef max
#else
#include <unistd.h>
#endif
#endif




namespace Tesses::CrossLang
{
    class DocumentationParser : public TNativeObject
    {
        public:

        std::vector<std::pair<std::string,TObject>> items;
            DocumentationParser(std::string doc)
            {
                size_t i=0;
                int state=-1;
                std::string key={};
                std::string value={};

                auto add = [&]()->void{
                    if(value == "true") {
                        items.push_back(std::pair<std::string,TObject>(key,true));
                    } else if(value == "false") {
                        items.push_back(std::pair<std::string,TObject>(key,true));
                    } else if(value == "null") {
                        items.push_back(std::pair<std::string,TObject>(key,nullptr));
                    } else {
                        if(value[0] >= '0' && value[0] <= '9')
                        {
                            try {
                                if(value.find('.') != std::string::npos)
                                {
                                
                                    items.push_back(std::pair<std::string,TObject>(key,std::stod(value)));
                                }
                                else {
                                    items.push_back(std::pair<std::string,TObject>(key,std::stoll(value)));
                                }
                            }catch(...) {}
                        }
                        else {
                            items.push_back(std::pair<std::string,TObject>(key,value));
                        }
                    }
                };
                
        auto ReadChr = [&]() -> std::pair<int,bool> {
             int read=i < doc.size() ? (int)doc[i++] : -1;
                
            if(read == -1) 
            {
                
                return std::pair<int,bool>(-1,false);
            }
            


            if(read == '\\')
            {
                read = i < doc.size() ? (int)doc[i++] : -1;
                
                if(read == -1)
                {
                    return std::pair<int,bool>(-1,true);
                }
                else if(read == 'n')
                {
                    return std::pair<int,bool>('\n',true);
                }
                else if(read == 'r')
                {
                    return std::pair<int,bool>('\r',true);
                }
                else if(read == 'f')
                {
                    return std::pair<int,bool>('\f',true);
                }
                else if(read == 'b')
                {
                    return std::pair<int,bool>('\b',true);
                }
                else if(read == 'a')
                {
                    return std::pair<int,bool>('\a',true);
                }
                else if(read == '0')
                {
                    return std::pair<int,bool>('\0',true);
                }
                else if(read == 'v')
                {
                    return std::pair<int,bool>('\v',true);
                }
                else if(read == 'e')
                {
                    return std::pair<int,bool>('\x1B',true);
                }
                else if(read == 't')
                {
                    return std::pair<int,bool>('\t',true);
                }
                else if(read == 'x')
                {
                    int r1 = i < doc.size() ? (int)doc[i++] : -1;
                    
                    if(r1 == -1)
                    {
                        return std::pair<int,bool>(-1,true);
                    }
                    int r2 = i < doc.size() ? (int)doc[i++] : -1;
                   
                    if(r2 == -1)
                    {
                        return std::pair<int,bool>(-1,true);
                    }
                    
                    uint8_t c = (uint8_t)std::stoi(std::string{(char)r1,(char)r2},nullptr,16);

                    return std::pair<int,bool>(c,true);
                }
                else
                {
                    return std::pair<int,bool>(read,true);
                }
            }
            else
            {
                return std::pair<int,bool>(read,false);
            }
        };




                for(; i < doc.size(); i++)
                {
                    if(doc[i] == '@')
                    {
                         state = 0;
                         key={};
                         value={};
                        if(i + 1 < doc.size())
                        {
                            if(doc[i+1] == '@') 
                            {
                                i++;
                                continue;
                            }
                            else {
                                i++;
                                for(; i < doc.size(); i++)
                                {
                                    switch(doc[i])
                                    {
                                        case '\'':
                                        {
                                            i++;
                                            auto c = ReadChr();
                                            i++;
                                            if(c.first != -1)
                                            {
                                                if(state == 0)
                                                    key+={(char)c.first};
                                                else {
                                                    this->items.push_back(std::pair<std::string,TObject>(key,(char)c.first));
                                                    state = -1;
                                                    goto outer;
                                                }
                                            }
                                        }
                                        break;
                                        case '\"':
                                        {
                                            if(state == 0)
                                            {
                                                i++;
                                                auto rChr = ReadChr();
                                                while(rChr.first != '\"' || rChr.second)
                                                {
                                                    if(rChr.first == -1) break;
                                                    key += (char)rChr.first;
                                                    rChr = ReadChr();
                                                }
                                                i--;
                                            }
                                            else {
                                                i++;
                                                auto rChr = ReadChr();
                                                
                                                while(rChr.first != '\"' || rChr.second)
                                                {
                                                    if(rChr.first == -1) break;
                                                    value += (char)rChr.first;
                                                    rChr = ReadChr();
                                                }
                                                this->items.push_back(std::pair<std::string,TObject>(key,value));
                                                state = -1;
                                                goto outer;
                                            }
                                        }
                                            break;
                                        case ' ':
                                        case '\n':
                                        case '\t':
                                        case '\r':
                                            if(state == 0 && !key.empty())
                                                state=1;
                                            else
                                            if(state == 1 && !value.empty())
                                            {
                                                add();
                                                state = -1;
                                                goto outer;
                                            }
                                            break;
                                        default:
                                            if(state == 0) key += doc[i];
                                            else value += doc[i];
                                            break;
                                    }
                                }
                            }
                            outer:;
                        }
                        
                    }
                }
                if(state == 1 && !value.empty())
                {
                    add();
                }
            }
            TObject CallMethod(GCList& ls, std::string key, std::vector<TObject> args);
            std::string TypeName()
            {
                return "DocumentationParser";
            }
    };
    class DocumentationParserEnumerator : public TEnumerator
    {
        int index;
        DocumentationParser* dict;
        public:
            static DocumentationParserEnumerator* Create(GCList& ls, DocumentationParser* dict)
            {
                auto dpe=new DocumentationParserEnumerator();
                auto gc = ls.GetGC();
                ls.Add(dpe);
                gc->Watch(dpe);
                dpe->dict = dict;
                dpe->index=-1;
                return dpe;
            }

            bool MoveNext(GC* ls)
            {
                ls->BarrierBegin();
                bool r = ++index < this->dict->items.size();
                   
                ls->BarrierEnd();
                return r;
            }
            TObject GetCurrent(GCList& ls)
            {
                std::pair<std::string,TObject> item;
                ls.GetGC()->BarrierBegin();
                if(this->index > -1 && this->index < this->dict->items.size())
                {
                    item = this->dict->items[(size_t)this->index];
                }
                ls.GetGC()->BarrierEnd();

                return TDictionary::Create(ls,{
                    TDItem("Key", item.first),
                    TDItem("Value", item.second)
                });
            }
            void Mark()
            {
                if(this->marked) return;
                this->marked=true;
                dict->Mark();
            }
    };


    TObject DocumentationParser::CallMethod(GCList& ls, std::string key, std::vector<TObject> args)
    {
        std::string myKey;
        if(key == "GetAt" && GetArgument(args,0,myKey))
        {
            for(auto item : this->items)
                if(item.first == myKey) return item.second;
        }
        if(key == "ToString")
        {
            std::string n={};
            for(auto item : this->items)
            {
                n.push_back('@');
                n.append(item.first);
                n.push_back(' ');
                std::string str;
                if(GetObject(item.second,str))
                {
                    n.append(EscapeString(str,true));
                }
                else n.append(ToString(ls.GetGC(), item.second));
                n.push_back('\n');
            }
            return n;
        }
        if(key == "GetEnumerator")
        {
            return DocumentationParserEnumerator::Create(ls,this);
        }
        return Undefined();
            
    }
    TObject New_DocumentationParser(GCList& ls, std::vector<TObject> args)
    {
        std::string doc;

        if(GetArgument(args,0,doc))
        {
            return TNativeObject::Create<DocumentationParser>(ls, doc);
        }

        return Undefined();
    }


    #if defined(CROSSLANG_ENABLE_SHARED)
    class DL {
        void* handle;
        public:
            DL(Tesses::Framework::Filesystem::VFSPath p)
            {
                Tesses::Framework::Filesystem::LocalFilesystem lfs;
                std::string str = lfs.VFSPathToSystem(p);
                #if defined(_WIN32)
                handle = LoadLibraryExA(str.c_str() , NULL, LOAD_WITH_ALTERED_SEARCH_PATH );
                #else
                handle = dlopen(str.c_str(), RTLD_LAZY);
                #endif
            }
            template<typename T>
            T Resolve(std::string name)
            {
                #if defined(_WIN32)
                return (T)GetProcAddress((HMODULE)handle,name.c_str());
                #else
                return (T)dlsym(handle,name.c_str());
                
                #endif
            }
            ~DL()
            {
                #if defined(_WIN32)
                FreeLibrary((HMODULE)handle);
                #else
                dlclose(handle);
                #endif
            }
    };
    #endif
    #if defined(CROSSLANG_ENABLE_FFI)
    typedef union {
        ffi_arg v_arg;
        ffi_sarg v_sarg;
        uint64_t v_u64;
        int64_t v_i64;
        double v_f64;
        void* v_ptr;

    } ffi_tmp_type_t;

    template<typename T>
    class ffi_my_struct_t {
        char ffi_my_struct_chr;
        T ffi_my_struct_val;

        public:
            static int64_t GetPadding()
            {
                ffi_my_struct_t<T> s;
                return ((int64_t)&s.ffi_my_struct_val - (int64_t)&s.ffi_my_struct_chr);
            }
    };



    ffi_tmp_type_t FFI_ConvertTObjectToFFIType(TObject& arg,std::string t)
    {
        
        if(std::holds_alternative<char>(arg))
        {
            if(t == "char" || t == "i8" || t == "i16" || t == "i32")
            {
                return (ffi_tmp_type_t){.v_sarg = std::get<char>(arg)};
            }
            else if(t == "u8" || t == "u16" || t == "u32")
            {
                return (ffi_tmp_type_t){.v_arg = (uint8_t)std::get<char>(arg)};
            }
            else if(t == "u64")
            {
                return (ffi_tmp_type_t){.v_u64 = (uint8_t)std::get<char>(arg)};
            }
            else if(t == "i64")
            {
                return (ffi_tmp_type_t){.v_i64 = std::get<char>(arg)};
            }
        }
        else if(std::holds_alternative<int64_t>(arg))
        {
            if(t == "u8" || t == "u16" || t == "u32")
            {
                return (ffi_tmp_type_t){.v_arg = (ffi_arg)(uint64_t)std::get<int64_t>(arg)};
            }
            else if(t == "char" || t == "i8" || t == "i16" || t == "i32")
            {
                return (ffi_tmp_type_t){.v_sarg = (ffi_sarg)std::get<int64_t>(arg)};
            }
            else if(t == "u64")
            {
                return (ffi_tmp_type_t){.v_u64 = (uint64_t)std::get<int64_t>(arg)};
            }
            else if(t == "i64")
            {
                return (ffi_tmp_type_t){.v_i64 =std::get<int64_t>(arg)};
            }
            else if(t == "pointer" || t == "string")
            {
                return (ffi_tmp_type_t){.v_ptr = (void*)std::get<int64_t>(arg)};
            }
        }
        else if(std::holds_alternative<double>(arg))
        {
            if(t == "f64")
            {
                return (ffi_tmp_type_t){.v_f64 =std::get<double>(arg)};
            }
        }
        else if(std::holds_alternative<std::string>(arg))
        {
            if(t == "pointer" || t == "string")
            {
                return (ffi_tmp_type_t){.v_ptr = (void*)std::get<std::string>(arg).c_str()};
            }
        }
        else if(std::holds_alternative<THeapObjectHolder>(arg))
        {
            auto obj = std::get<THeapObjectHolder>(arg).obj;
            auto bA = dynamic_cast<TByteArray*>(obj);
            auto nat = dynamic_cast<TNative*>(obj);
            if(bA != nullptr)
            {
                if(t == "pointer" || t == "string")
                {
                    return (ffi_tmp_type_t){.v_ptr = (void*)bA->data.data() };
                }
            }
            if(nat != nullptr)
            {
                if(t == "pointer" || t == "string")
                {
                    return (ffi_tmp_type_t){.v_ptr = nat->GetPointer() };
                }
            }
        }


        return {0};
    }

    ffi_type* FFI_ConvertStringToFFIType(std::string t)
    {
        if(t == "char")
        {
            return &ffi_type_schar;
        }
        else if(t == "i8")
        {
            return &ffi_type_sint8;
        }
        else if(t == "u8")
        {
            return &ffi_type_uint8;
        }
        else if(t == "i16")
        {
            return &ffi_type_sint16;
        }
        else if(t == "u16")
        {
            return &ffi_type_uint16;
        }
        else if(t == "i32")
        {
            return &ffi_type_sint32;
        }
        else if(t == "u32")
        {
            return &ffi_type_uint32;
        }
        else if(t == "i64")
        {
            return &ffi_type_sint64;
        }
        else if(t == "u64")
        {
            return &ffi_type_uint64;
        }
        else if(t == "float")
        {
            return &ffi_type_float;
        }
        else if(t == "double")
        {
            return &ffi_type_double;
        }
        else if(t == "void")
        {
            return &ffi_type_void;
        }
        else if(t == "pointer" || t == "string")
        {
            return &ffi_type_pointer;
        }
        return nullptr;
    }
    TExternalMethod* FFI_CreateFunction(GCList& ls,TNative* native,std::string retVal,std::vector<std::string> listArgs,ffi_abi abi,bool freeRet)
    {
        ffi_type* retTypeT = FFI_ConvertStringToFFIType(retVal);
        std::vector<ffi_type*> argTypes;

        argTypes.resize(listArgs.size());
        for(size_t i = 0; i < listArgs.size();i++)
        {
            argTypes[i] = FFI_ConvertStringToFFIType(listArgs[i]);
        }
        auto cb = TExternalMethod::Create(ls,"FFI Generated function, returns: " + retVal,listArgs, [retVal,listArgs,abi,native,freeRet,argTypes,retTypeT](GCList& ls, std::vector<TObject> args)->TObject {
                    if(args.size() != listArgs.size()) throw VMException("Args must be " + std::to_string(listArgs.size()) + "but got " + std::to_string(args.size()) + ".");
                    ffi_cif cif;
                    
                    std::vector<void*> argVals;
                    std::vector<ffi_tmp_type_t> tmpTypes;
                    argVals.resize(listArgs.size());
                    tmpTypes.resize(listArgs.size());
                    for(size_t i = 0; i < argTypes.size(); i++)
                    {
                        
                        tmpTypes[i] = FFI_ConvertTObjectToFFIType(args[i],listArgs[i]);
                        if(listArgs[i] == "char" || listArgs[i] == "i8" || listArgs[i] == "i16" || listArgs[i] == "i32" || listArgs[i] == "u8" || listArgs[i] == "u16" || listArgs[i] == "u32")
                        {
                            argVals[i] = (void*)&(tmpTypes[i].v_arg);
                        }
                       
                        else if(listArgs[i] == "u64" || listArgs[i] == "i64")
                        {
                            argVals[i] = (void*)&(tmpTypes[i].v_u64);
                        }
                        else if(listArgs[i] == "pointer" || listArgs[i] == "string")
                        {
                            argVals[i] = (void*)&(tmpTypes[i].v_ptr);
                        }
                        else if(listArgs[i] == "f64")
                        {
                            argVals[i] = (void*)&(tmpTypes[i].v_f64);
                        }
                    }
                    ffi_prep_cif(&cif, abi,(unsigned int)listArgs.size(),retTypeT,(ffi_type**)argTypes.data());

                    ffi_tmp_type_t retObj;
                    void* retPtr=NULL;
                    if(retVal == "char" || retVal == "i8" || retVal == "u8" || retVal == "i16" || retVal == "u16" || retVal == "i32" || retVal == "u32")
                    {
                        retPtr = (void*)&(retObj.v_arg);
                    }
                    else if(retVal == "i64" || retVal == "u64")
                    {
                        retPtr = (void*)&(retObj.v_u64);
                    }
                    else if(retVal == "f64")
                    {
                        retPtr = (void*)&(retObj.v_f64);
                    }
                    else if(retVal == "pointer" || retVal == "string")
                    {
                        retPtr = (void*)&(retObj.v_ptr);
                    }
                    

                    ffi_call(&cif, (void(*)())native->GetPointer() , retPtr, argVals.data());

                    

                    if(retVal == "char") return (char)retObj.v_sarg;
                    if(retVal == "i8" || retVal == "i16" || retVal == "i32") return (int64_t)retObj.v_sarg;
                    if(retVal == "u8" || retVal == "u16" || retVal == "u32") return (int64_t)retObj.v_arg;
                    if(retVal == "i64" || retVal == "u64") return retObj.v_i64;
                    if(retVal == "f64") return retObj.v_f64;
                    if(retVal == "string")
                    {
                        std::string res = (char*)retObj.v_ptr;
                        if(freeRet)
                            free(retObj.v_ptr);
                        return res;
                    }
                    if(retVal == "pointer")
                    {
                        TNative* native3 = TNative::Create(ls,retObj.v_ptr,[freeRet](void* _ptr)->void {
                            if(freeRet) free(_ptr);
                        });
                        native3->other = native;
                        return native3;
                    }
                    return nullptr;
                });
                cb->watch.push_back(native);
                return cb;
    }

    
    

    void RegisterFFI(GC* gc, TDictionary* dict)
    {
        dict->SetValue("SizeOfChar",(int64_t)sizeof(char));
        dict->SetValue("SizeOfShort",(int64_t)sizeof(short));
        dict->SetValue("SizeOfInt",(int64_t)sizeof(int));
        dict->SetValue("SizeOfLong",(int64_t)sizeof(long));
        dict->SetValue("SizeOfLongLong",(int64_t)sizeof(long long));
        dict->SetValue("SizeOfInt8",(int64_t)sizeof(int8_t));
        dict->SetValue("SizeOfInt16",(int64_t)sizeof(int16_t));
        dict->SetValue("SizeOfInt32",(int64_t)sizeof(int32_t));
        dict->SetValue("SizeOfInt64",(int64_t)sizeof(int64_t));
        dict->SetValue("SizeOfFloat",(int64_t)sizeof(float));
        dict->SetValue("SizeOfDouble",(int64_t)sizeof(double));
        dict->SetValue("SizeOfPointer",(int64_t)sizeof(void*));
        dict->SetValue("PaddingOfShort",ffi_my_struct_t<short>::GetPadding());
        dict->SetValue("PaddingOfInt",ffi_my_struct_t<int>::GetPadding());
        dict->SetValue("PaddingOfLong",ffi_my_struct_t<long>::GetPadding());
        dict->SetValue("PaddingOfLongLong",ffi_my_struct_t<long long>::GetPadding());
        dict->SetValue("PaddingOfInt16",ffi_my_struct_t<int16_t>::GetPadding());
        dict->SetValue("PaddingOfInt32",ffi_my_struct_t<int32_t>::GetPadding());
        dict->SetValue("PaddingOfInt64",ffi_my_struct_t<int64_t>::GetPadding());
        dict->SetValue("PaddingOfFloat",ffi_my_struct_t<float>::GetPadding());
        dict->SetValue("PaddingOfDouble",ffi_my_struct_t<double>::GetPadding());
        dict->SetValue("PaddingOfPointer",ffi_my_struct_t<void*>::GetPadding());

        union {
            uint8_t c[2];
            uint16_t num;
        } endian;
        endian.c[0] = 0x01;
        endian.c[1] = 0xA4;

        dict->SetValue("IsLittleEndian", (bool)(endian.num != 420));


        dict->DeclareFunction(gc, "Open","Open a shared object",{"path"},[](GCList& ls, std::vector<TObject> args)->TObject {
            Tesses::Framework::Filesystem::VFSPath path;
            if(GetArgumentAsPath(args,0,path))
            {
                return TNative::Create(ls,static_cast<void*>(new DL(path)),[](void* ptr)->void {
                    delete static_cast<DL*>(ptr);
                });
            }
            return nullptr;
        });
        dict->DeclareFunction(gc, "Symbol","Get current symbol",{"sharedObj","name"},[](GCList& ls, std::vector<TObject> args)->TObject {
            TNative* native;
            std::string name;
            if(GetArgumentHeap(args,0,native) && GetArgument(args,1,name)) {
                if(native->GetDestroyed()) return nullptr;
                DL* dl = static_cast<DL*>(native->GetPointer());
                auto native2= TNative::Create(ls,dl->Resolve<void*>(name), [](void* _e)->void {});
                native2->other = native;
                return native2;
            }
            return nullptr;
        });
        dict->DeclareFunction(gc, "Close", "Closes the shared object",{"sharedObj"},[](GCList& ls, std::vector<TObject> args)->TObject {
            TNative* native;
            if(GetArgumentHeap(args,0,native)) native->Destroy();

            return nullptr;
        });
        

        dict->DeclareFunction(gc, "CreateFunction","Create a function", {"functionPtr","returnVal","args","$abi","$freeRet"},[](GCList& ls, std::vector<TObject> args)->TObject {
            TNative* native;
            std::string retVal;
            TList* listArgsO;
            int64_t abi=(int64_t)FFI_DEFAULT_ABI;
            bool freeRet=false;
            if(GetArgumentHeap(args,0,native) && GetArgument(args,1,retVal) && GetArgumentHeap(args,2,listArgsO))
            {
                GetArgument(args,3,abi);
                GetArgument(args,4,freeRet);
                std::vector<std::string> listArgs;
                for(int64_t i = 0; i < listArgsO->Count(); i++)
                {
                    auto item = listArgsO->Get(i);
                    std::string typ;
                    if(GetObject(item, typ)) listArgs.push_back(typ);
                }

                
                return FFI_CreateFunction(ls,native,retVal,listArgs,(ffi_abi)abi,freeRet);
            }
            return nullptr;
        });
    }
    #endif
    
    void LoadPlugin(GC* gc, TRootEnvironment* env, Tesses::Framework::Filesystem::VFSPath sharedObjectPath)
    {
        #if defined(CROSSLANG_ENABLE_SHARED)
        auto ptr = std::make_shared<DL>(sharedObjectPath);
        auto cb = ptr->Resolve<PluginFunction>("CrossLangPluginInit");
        if(cb == nullptr) return;
        gc->RegisterEverythingCallback([ptr,cb](GC* gc, TRootEnvironment* env)-> void{
            cb(gc,env);
        });
        cb(gc,env);
        #endif
    }
    static TObject TypeIsClass(GCList& ls, std::vector<TObject> args)
    {
        if(args.empty()) return nullptr;
        if(args.empty()) return nullptr;
        TClassObject* co;
        return GetArgumentHeap(args,0,co);
    }

    static TObject TypeIsDefined(GCList& ls,std::vector<TObject> args)
    {
        if(args.empty()) return nullptr;
        return (!std::holds_alternative<std::nullptr_t>(args[0]) && !std::holds_alternative<Undefined>(args[0]));
    }
    static TObject TypeIsHeap(GCList& ls, std::vector<TObject> args)
    {
        if(args.empty()) return nullptr;
        return std::holds_alternative<THeapObjectHolder>(args[0]);
    }
    static TObject TypeIsNumber(GCList& ls, std::vector<TObject> args)
    {
        if(args.empty()) return nullptr;
        return std::holds_alternative<int64_t>(args[0]) || std::holds_alternative<double>(args[0]);
    }
    static TObject TypeIsLong(GCList& ls, std::vector<TObject> args)
    {
        if(args.empty()) return nullptr;
        return std::holds_alternative<int64_t>(args[0]);
    }
    static TObject TypeIsDouble(GCList& ls, std::vector<TObject> args)
    {
        if(args.empty()) return nullptr;
        return std::holds_alternative<double>(args[0]);
    }
    static TObject TypeIsString(GCList& ls, std::vector<TObject> args)
    {
        if(args.empty()) return nullptr;
        return std::holds_alternative<std::string>(args[0]);
    }
    static TObject TypeIsCallable(GCList& ls, std::vector<TObject> args)
    {
        if(args.empty()) return nullptr;
        TCallable* call;
        return GetArgumentHeap(args,0,call);
    }
    static TObject TypeIsDictionary(GCList& ls, std::vector<TObject> args)
    {
        if(args.empty()) return nullptr;
        TDictionary* dict;
        TDynamicDictionary* dynDict;
        return GetArgumentHeap(args,0,dict) || GetArgumentHeap(args,0,dynDict);
    }
    static TObject TypeIsList(GCList& ls, std::vector<TObject> args)
    {
        if(args.empty()) return nullptr;
        TList* list;
        TDynamicList* dynList;
        return GetArgumentHeap(args,0,list) || GetArgumentHeap(args,0, dynList);
    }
    static TObject TypeIsStream(GCList& ls, std::vector<TObject> args)
    {
        if(args.empty()) return nullptr;
        TStreamHeapObject* strm;
        return GetArgumentHeap(args,0,strm);
    }
    static TObject TypeIsVFS(GCList& ls, std::vector<TObject> args)
    {
        if(args.empty()) return nullptr;
        TVFSHeapObject* vfs;
        return GetArgumentHeap(args,0,vfs);
    }

    static TObject TypeIsDateTime(GCList& ls, std::vector<TObject> args)
    {
        if(args.empty()) return nullptr;
        TDateTime* dt;
        return GetArgumentHeap(args,0,dt);
    }
    static TObject New_SubdirFilesystem(GCList& ls, std::vector<TObject> args)
    {
        TVFSHeapObject* vfsho;

        Tesses::Framework::Filesystem::VFSPath path;

        if(GetArgumentHeap(args,0,vfsho) && GetArgumentAsPath(args,1,path))
        {
            return TVFSHeapObject::Create(ls,new Tesses::Framework::Filesystem::SubdirFilesystem(new TObjectVFS(ls.GetGC(),vfsho),path,true));
        }
        return nullptr;
    }
    static TObject New_MountableFilesystem(GCList& ls, std::vector<TObject> args)
    {
        TVFSHeapObject* vfsho;

        if(GetArgumentHeap(args,0,vfsho))
        {
            return TVFSHeapObject::Create(ls,new Tesses::Framework::Filesystem::MountableFilesystem(new TObjectVFS(ls.GetGC(),vfsho),true));
        }
        return nullptr;
    }
    static TObject New_MemoryStream(GCList& ls, std::vector<TObject> args)
    {
        bool writable;
        if(GetArgument(args,0,writable))
        {
            return TStreamHeapObject::Create(ls,new Tesses::Framework::Streams::MemoryStream(writable));
        }
        return nullptr;
    }
    static TObject New_MemoryFilesystem(GCList& ls, std::vector<TObject> args)
    {
        return TVFSHeapObject::Create(ls, new Tesses::Framework::Filesystem::MemoryFilesystem());
    }

    static TObject New_Filesystem(GCList& ls, std::vector<TObject> args)
    {
        TDictionary* dict;
        if(GetArgumentHeap(args,0,dict))
        {
            return TVFSHeapObject::Create(ls, new TObjectVFS(ls.GetGC(),dict));
        }
        return nullptr;
    }
    static TObject New_Stream(GCList& ls, std::vector<TObject> args)
    {
        TDictionary* dict;
        if(GetArgumentHeap(args,0,dict))
        {
            return TStreamHeapObject::Create(ls, new TObjectStream(ls.GetGC(),dict));
        }
        return nullptr;
    }
    static TObject New_DateTime(GCList& ls, std::vector<TObject> args)
    {
        int64_t year;
        if(GetArgument(args,0,year))
        {
            if(args.size()==1)
            {
               Tesses::Framework::Date::DateTime dt(year);
               return dt;
            }
            else
            {
                int64_t month=1;
                int64_t day=1;
                int64_t hour=0;
                int64_t minute=0;
                int64_t second=0;
                bool isLocal=true;
                GetArgument(args,1,month);
                GetArgument(args,2,day);
                GetArgument(args,3,hour);
                GetArgument(args,4,minute);
                GetArgument(args,5,second);
                GetArgument(args,6,isLocal);

                
                Tesses::Framework::Date::DateTime dt((int)year,(int)month,(int)day,(int)hour,(int)minute,(int)second,isLocal);
                return dt;
            }
        }
        return nullptr;
    }
  
    static TObject TypeOf(GCList& ls, std::vector<TObject> args)
    {
        if(args.size() < 1) return "Undefined";
        if(std::holds_alternative<std::regex>(args[0])) return "Regex";
        if(std::holds_alternative<Undefined>(args[0])) return "Undefined";
        if(std::holds_alternative<std::nullptr_t>(args[0])) return "Null";
        if(std::holds_alternative<bool>(args[0])) return "Boolean";
        if(std::holds_alternative<int64_t>(args[0])) return "Long";
        if(std::holds_alternative<double>(args[0])) return "Double";
        if(std::holds_alternative<char>(args[0])) return "Char";
        if(std::holds_alternative<MethodInvoker>(args[0])) return "MethodInvoker";
        if(std::holds_alternative<std::string>(args[0])) return "String";
        
        if(std::holds_alternative<Tesses::Framework::Filesystem::VFSPath>(args[0])) return "Path";
        if(std::holds_alternative<TDateTime>(args[0])) return "DateTime";
        if(std::holds_alternative<THeapObjectHolder>(args[0]))
        {
            auto obj = std::get<THeapObjectHolder>(args[0]).obj;
            auto dict = dynamic_cast<TDictionary*>(obj);
            auto dynDict = dynamic_cast<TDynamicDictionary*>(obj);
            
            auto list = dynamic_cast<TList*>(obj);
            auto dynList = dynamic_cast<TDynamicList*>(obj);
            auto argWrapper = dynamic_cast<TArgWrapper*>(obj);
            auto closure = dynamic_cast<TClosure*>(obj);
            auto externalMethod = dynamic_cast<TExternalMethod*>(obj);
            auto byteArray = dynamic_cast<TByteArray*>(obj);
            auto native = dynamic_cast<TNative*>(obj);
            auto any = dynamic_cast<TAny*>(obj);
            auto vfs = dynamic_cast<TVFSHeapObject*>(obj);
            auto strm = dynamic_cast<TStreamHeapObject*>(obj);
            auto svr = dynamic_cast<TServerHeapObject*>(obj);
            auto cse = dynamic_cast<CallStackEntry*>(obj);
            auto rootEnv = dynamic_cast<TRootEnvironment*>(obj);
            auto subEnv = dynamic_cast<TSubEnvironment*>(obj);
            auto env = dynamic_cast<TEnvironment*>(obj);
            auto natObj = dynamic_cast<TNativeObject*>(obj);
            auto cobj = dynamic_cast<TClassObject*>(obj);
            auto aarray = dynamic_cast<TAssociativeArray*>(obj);

            if(rootEnv != nullptr) return "RootEnvironment";
            if(subEnv != nullptr) return "SubEnvironment";
            if(env != nullptr) return "Environment";
            if(cobj != nullptr) return cobj->TypeName();

            if(cse != nullptr) return "YieldedClosure";
            if(dynDict != nullptr) return "DynamicDictionary";
            if(dynList != nullptr) return "DynamicList";
            if(aarray != nullptr) return "AssociativeArray";
            if(natObj != nullptr) return natObj->TypeName();
            if(strm != nullptr)
            {
                auto netStrm = dynamic_cast<Tesses::Framework::Streams::NetworkStream*>(strm->stream);
                if(netStrm != nullptr)
                {
                    return "NetworkStream";
                }
                

                return "Stream";
            }
            if(svr != nullptr)
            {
                auto fileServer = dynamic_cast<Tesses::Framework::Http::FileServer*>(svr->server);
                auto mountableServer = dynamic_cast<Tesses::Framework::Http::MountableServer*>(svr->server);
                if(fileServer != nullptr)
                {
                    return "FileServer";
                }
                if(mountableServer != nullptr)
                {
                    return "MountableServer";
                }
                return "HttpServer";
            }
            if(vfs != nullptr)
            {
                auto localVFS = dynamic_cast<Tesses::Framework::Filesystem::LocalFilesystem*>(vfs->vfs);

                auto mountableVFS = dynamic_cast<Tesses::Framework::Filesystem::MountableFilesystem*>(vfs->vfs);


                auto subFS = dynamic_cast<Tesses::Framework::Filesystem::SubdirFilesystem*>(vfs->vfs);

                if(localVFS != nullptr) return "LocalFilesystem";
                if(subFS != nullptr) return "SubdirFilesystem";
                if(mountableVFS != nullptr) return "MountableFilesystem";
                
                return "VFS";
            }
            if(dict != nullptr) return "Dictionary";
            if(list != nullptr) return "List";
            if(argWrapper != nullptr) return "ArgWrapper";
            if(closure != nullptr) return "Closure";
            if(externalMethod != nullptr) return "ExternalMethod";
            if(byteArray != nullptr) return "ByteArray";
            if(native != nullptr) return "Native";
            if(any != nullptr) return "Any";

            return "HeapObject";
        }

        return "Object";
    }
    
     TObject ByteArray(GCList& ls, std::vector<TObject> args)
    {
        auto res= TByteArray::Create(ls);
        if(args.size() == 1){
                
            if(std::holds_alternative<int64_t>(args[0]))
            res->data.resize((size_t)std::get<int64_t>(args[0]));
            else if(std::holds_alternative<std::string>(args[0]))
            {
                std::string& txt = std::get<std::string>(args[0]);
                res->data.insert(res->data.end(), txt.begin(),txt.end());
            }
                
        }
        return res;

    }
    TObject ParseLong(GCList& ls, std::vector<TObject> args)
    {
        
        if(args.size() >= 1)
        {
            int base = 10;
            
            if(args.size() == 2 && std::holds_alternative<int64_t>(args[1]))
            {
                base = (int)std::get<int64_t>(args[1]);
            }

            size_t pos;


            std::string str = ToString(ls.GetGC(),args[0]);
            
            try {
                int64_t v =  std::stoll(str,&pos,base);
                if(pos < str.size()) return nullptr;
                return v;
            }catch(std::exception& ex)
            {
                return nullptr;
            }

        }
        return nullptr;

    }
 TObject ParseDouble(GCList& ls, std::vector<TObject> args)
    {
        
        if(args.size() == 1)
        {
            

            size_t pos;


            std::string str = ToString(ls.GetGC(),args[0]);
            
            try {
                double v =  std::stod(str,&pos);
                if(pos < str.size()) return nullptr;
                return v;
            }catch(std::exception& ex)
            {
                return nullptr;
            }

        }
        return nullptr;

    }

    

    EnvironmentPermissions::EnvironmentPermissions()
    {
        this->canRegisterConsole=false;
        this->canRegisterCrypto=false;
        this->canRegisterDictionary=false;
        this->canRegisterEnv=false;
        this->canRegisterIO=false;
        this->canRegisterJSON=false;
        this->canRegisterLocalFS=false;
        this->canRegisterNet=false;
        this->canRegisterOGC=false;
        this->canRegisterPath=false;
        this->canRegisterProcess=false;
        this->canRegisterRoot=false;
        this->canRegisterSqlite=false;
        this->canRegisterVM = false;
        this->locked=false;
        this->sqlite3Scoped=false;
    
    }

    static TObject YieldEnumerableFunc(GCList& ls, std::vector<TObject> args)
    {
        TClosure* closure;
        if(GetArgumentHeap(args,0,closure))
        {
            TDictionary* enumerableItem = TDictionary::Create(ls);
            ls.GetGC()->BarrierBegin();

            auto fn = TExternalMethod::Create(ls,"Get Enumerator for yield",{},[closure](GCList& ls2, std::vector<TObject> args)->TObject {
                return TYieldEnumerator::Create(ls2,closure);
            });
            fn->watch.push_back(closure);
                
            enumerableItem->SetValue("GetEnumerator", fn);
                
            ls.GetGC()->BarrierEnd();

            return enumerableItem;
            
        }

        return Undefined();
    }

    
    static TObject DateTime_Sleep(GCList& ls, std::vector<TObject> args)
    {
        int64_t msec;
        if(GetArgument(args,0,msec))
        {
            #if defined(_WIN32)
            Sleep((int)msec);
            #else
            usleep(1000*msec);   
            #endif
        }
        return nullptr;
    }
    static TObject DateTime_getNow(GCList& ls, std::vector<TObject> args)
    {
        
        return Tesses::Framework::Date::DateTime::Now();
    }
    static TObject DateTime_getNowUTC(GCList& ls, std::vector<TObject> args)
    {
        return Tesses::Framework::Date::DateTime::NowUTC();
    }
    static TObject DateTime_getNowEpoch(GCList& ls, std::vector<TObject> args)
    {
        
        return (int64_t)time(NULL);
    }
    static TObject DateTime_TryParseHttpDate(GCList& ls, std::vector<TObject> args)
    {
        std::string d;
        Tesses::Framework::Date::DateTime dt;
        if(GetArgument(args,0,d) && Tesses::Framework::Date::DateTime::TryParseHttpDate(d,dt))
        {
            return dt;
        }
        return nullptr;
    }
    void TStd::RegisterRoot(GC* gc, TRootEnvironment* env)
    {
        GCList ls(gc);      
        
        gc->BarrierBegin();  
        
        env->permissions.canRegisterRoot=true;
       
        auto date =env->EnsureDictionary(gc,"DateTime");
        date->DeclareFunction(gc, "Sleep","Sleep for a specified amount of milliseconds (multiply seconds by 1000 to get milliseconds)", {"ms"},DateTime_Sleep);
        date->DeclareFunction(gc, "getNow", "Get the current time",{},DateTime_getNow);
        date->DeclareFunction(gc, "getNowUTC","Get the current time in UTC",{},DateTime_getNowUTC);
        date->DeclareFunction(gc, "getNowEpoch","Get the time_t time now",{},DateTime_getNowEpoch);
        date->DeclareFunction(gc, "TryParseHttpDate","Parse the http date",{},DateTime_TryParseHttpDate);
        
        
	
        date->SetValue("Zone", (int64_t)Tesses::Framework::Date::GetTimeZone());
        date->SetValue("SupportsDaylightSavings",Tesses::Framework::Date::TimeZoneSupportDST());
	
        auto task = env->EnsureDictionary(gc,"Task");

        task->DeclareFunction(gc,"AsyncClosure","Create async closure (internal for compiler to generate calls to)",{"closure"},[](GCList& ls, std::vector<TObject> args)->TObject {
            TClosure* closure;
            if(GetArgumentHeap(args,0,closure))
            return TTask::FromClosure(ls,closure);
            return nullptr;
        });

        task->DeclareFunction(gc,"Run","Run code async",{"callable"},[](GCList& ls, std::vector<TObject> args)->TObject {
             TCallable* closure;
            if(GetArgumentHeap(args,0,closure))
            return TTask::Run(ls,closure);
            return nullptr;
        });


        TDictionary* newTypes = env->EnsureDictionary(gc, "New");
       
        newTypes->DeclareFunction(gc, "DateTime","Create a DateTime object, if only one arg is provided year is epoch, isLocal defaults to true unless epoch",{"year","$month","$day","$hour","$minute","$second","$isLocal"},New_DateTime);

        newTypes->DeclareFunction(gc, "MountableFilesystem","Create a mountable filesystem",{"root"}, New_MountableFilesystem);
        newTypes->DeclareFunction(gc, "SubdirFilesystem","Create a subdir filesystem",{"fs","subdir"}, New_SubdirFilesystem);
        newTypes->DeclareFunction(gc, "MemoryStream","Create a memory stream",{"writable"}, New_MemoryStream);
        newTypes->DeclareFunction(gc, "Stream","Create stream", {"strm"},New_Stream);
        newTypes->DeclareFunction(gc, "Filesystem","Create filesystem", {"fs"},New_Filesystem);

        newTypes->DeclareFunction(gc, "MemoryFilesystem","Create in memory filesystem", {},New_MemoryFilesystem);

    
        newTypes->DeclareFunction(gc,"Version","Create a version object",{"$major","$minor","$patch","$build","$stage"},[](GCList& ls, std::vector<TObject> args)->TObject{
            int64_t major=1;
            int64_t minor=0;
            int64_t patch=0;
            int64_t build=0;
            std::string stageS="prod";
            TVMVersionStage stage=TVMVersionStage::ProductionVersion;
            GetArgument(args, 0,major);
            GetArgument(args, 1,minor);
            GetArgument(args, 2,patch);
            GetArgument(args, 3,build);
            GetArgument(args, 4,stageS);
            if(stageS == "dev")
                stage = TVMVersionStage::DevVersion;
            else if(stageS == "alpha")
                stage = TVMVersionStage::AlphaVersion;
            else if(stageS == "beta")
                stage = TVMVersionStage::BetaVersion;

            return TVMVersion((uint8_t)major,(uint8_t)minor,(uint8_t)patch,(uint16_t)build,stage);
        });
        
       
        env->DeclareFunction(gc, "ParseLong","Parse Long from String",{"arg","$base"},ParseLong);
        env->DeclareFunction(gc, "ParseDouble","Parse Double from String",{"arg"},ParseDouble);
        env->DeclareFunction(gc, "YieldEmumerable","Turn yield in function into enumerable",{"closure"},YieldEnumerableFunc);
        env->DeclareFunction(gc, "TypeOf","Get type of object",{"object"},TypeOf);
        env->DeclareFunction(gc, "TypeIsDefined","Get whether object is not null or undefined",{"object"},TypeIsDefined);
        env->DeclareFunction(gc, "TypeIsHeap","Get whether object is susceptible to garbage collection",{"object"},TypeIsHeap);
        env->DeclareFunction(gc, "TypeIsNumber","Get whether object is a number",{"object"},TypeIsNumber);
        env->DeclareFunction(gc, "TypeIsLong","Get whether object is a long (not a double)",{"object"},TypeIsLong);
        env->DeclareFunction(gc, "TypeIsDouble","Get whether object is a double (not a long)",{"object"},TypeIsDouble);
        env->DeclareFunction(gc, "TypeIsString","Get whether object is a string",{"object"},TypeIsString);
        env->DeclareFunction(gc, "TypeIsCallable","Get whether object is callable",{"object"},TypeIsCallable);

        env->DeclareFunction(gc, "TypeIsClass","Get whether object is class instance",{"object"},TypeIsClass);
        env->DeclareFunction(gc, "TypeIsDictionary","Get whether object is a dictionary or dynamic dictionary",{"object"},TypeIsDictionary);
        env->DeclareFunction(gc, "TypeIsList","Get whether object is a list or dynamic list",{"object"},TypeIsList);
        env->DeclareFunction(gc, "TypeIsStream","Get whether object is a stream",{"object"},TypeIsStream);
        env->DeclareFunction(gc, "TypeIsVFS","Get whether object is a virtual filesystem",{"object"},TypeIsVFS);
        env->DeclareFunction(gc, "TypeIsDateTime","Get whether object is a DateTime",{"object"},TypeIsDateTime);
        
        
        
        newTypes->DeclareFunction(gc, "Regex", "Create regex object",{"regex"},[](GCList& ls,std::vector<TObject> args)->TObject {
            std::string str;
            if(GetArgument(args,0,str))
            {
                std::regex regex(str);
                return regex;
            }
            return nullptr;
        });
        newTypes->DeclareFunction(gc, "Mutex", "Create mutex",{}, [](GCList& ls,std::vector<TObject> args)->TObject {
            ls.GetGC()->BarrierBegin();
            auto mtx = TDictionary::Create(ls);
            auto native = TNative::Create(ls, new Tesses::Framework::Threading::Mutex(),[](void* ptr)->void{
                delete static_cast<Tesses::Framework::Threading::Mutex*>(ptr);
            });
            auto lock = TExternalMethod::Create(ls,"Lock the mutex",{},[native](GCList& ls, std::vector<TObject> args)->TObject {
                if(native->GetDestroyed()) return nullptr;
                auto r = static_cast<Tesses::Framework::Threading::Mutex*>(native->GetPointer());
                r->Lock();
                return nullptr;
            });
            lock->watch.push_back(native);
            mtx->SetValue("Lock",lock);

            auto unlock = TExternalMethod::Create(ls,"Unlock the mutex",{},[native](GCList& ls, std::vector<TObject> args)->TObject {
                if(native->GetDestroyed()) return nullptr;
                auto r = static_cast<Tesses::Framework::Threading::Mutex*>(native->GetPointer());
                r->Unlock();
                return nullptr;
            });
            unlock->watch.push_back(native);
            mtx->SetValue("Unlock",unlock);


            auto trylock = TExternalMethod::Create(ls,"Try to lock the mutex, returns true if we aquire the lock, false if we can't due to another thread owning it",{},[native](GCList& ls, std::vector<TObject> args)->TObject {
                if(native->GetDestroyed()) return true;
                auto r = static_cast<Tesses::Framework::Threading::Mutex*>(native->GetPointer());
                return r->TryLock();
            });
            trylock->watch.push_back(native);
            mtx->SetValue("TryLock",trylock);
            ls.GetGC()->BarrierEnd();


            auto close = TExternalMethod::Create(ls,"Try to lock the mutex, returns true if we aquire the lock, false if we can't due to another thread owning it",{},[native](GCList& ls, std::vector<TObject> args)->TObject {
                native->Destroy();
                return nullptr;
            });
            close->watch.push_back(native);
            mtx->SetValue("Close",close);
            ls.GetGC()->BarrierEnd();
            return mtx;
        });
        newTypes->DeclareFunction(gc, "Thread","Create thread",{"callback"},[](GCList& ls, std::vector<TObject> args)-> TObject
        {
            if(args.size() == 1 && std::holds_alternative<THeapObjectHolder>(args[0]))
            {
                auto cb = dynamic_cast<TCallable*>(std::get<THeapObjectHolder>(args[0]).obj);
                if(cb != nullptr)
                {
                    return CreateThread(ls,cb,false);
                }
            }
            return Undefined();
        });
        newTypes->DeclareFunction(gc,"DynamicList","Create a dynamic list",{},[](GCList& ls,std::vector<TObject> args)->TObject {
            TCallable* callable;
            if(GetArgumentHeap(args,0,callable))
            return TDynamicList::Create(ls,callable);
            return nullptr;
        });
        newTypes->DeclareFunction(gc,"DynamicDictionary","Create a dynamic dictionary",{},[](GCList& ls, std::vector<TObject> args)->TObject {
            TCallable* callable;
            if(GetArgumentHeap(args,0,callable))
            return TDynamicDictionary::Create(ls,callable);
            return nullptr;
        });
        newTypes->DeclareFunction(gc,"AssociativeArray","Create a new AssociativeArray",{},[](GCList& ls, std::vector<TObject> args)->TObject {
            return TAssociativeArray::Create(ls);
        });
        newTypes->DeclareFunction(gc,"AArray","alias for new AssociativeArray",{},[](GCList& ls, std::vector<TObject> args)->TObject {
            return TAssociativeArray::Create(ls);
        });
        newTypes->DeclareFunction(gc,"DocumentationParser","Parse documentation blocks",{"documentationString"},New_DocumentationParser);
        newTypes->DeclareFunction(gc,"ByteArray","Create bytearray, with optional either size (to size it) or string argument (to fill byte array)",{"$data"},ByteArray);
        
        env->DeclareVariable("Version", TDictionary::Create(ls,{
            TDItem("Parse",TExternalMethod::Create(ls,"Parse version from string",{"versionStr"},[](GCList& ls, std::vector<TObject> args)->TObject{
                std::string str;
                if(GetArgument(args, 0, str))
                {
                    TVMVersion version;
                    if(TVMVersion::TryParse(str,version))
                    {
                        return version;
                    }
                }
                return nullptr;
            })),
            TDItem("FromByteArray",TExternalMethod::Create(ls,"Create from ByteArray",{"byteArray","$offset"},[](GCList& ls,std::vector<TObject> args)->TObject {
                TByteArray* ba;
                if(GetArgumentHeap(args,0, ba))
                {
                    int64_t offset=0;
                    GetArgument(args, 1, offset);
                    if(ba->data.size() < 5) throw VMException("ByteArray too small");
                    size_t o = (size_t)offset;
                    if((o + 5) > ba->data.size() || (o + 5) < 5) throw VMException("ByteArray too small");
                    
                    return TVMVersion(ba->data.data()+o);
                }
                return nullptr;
            })),
            TDItem("FromLong", TExternalMethod::Create(ls, "Create from long",{"longBasedVersion"},[](GCList& ls, std::vector<TObject> args)->TObject {
                int64_t num;
                if(GetArgument(args,0,num))
                {
                    return TVMVersion((uint64_t)num);
                }
                return nullptr;
            }))
        }));
        env->DeclareVariable("InvokeMethod",MethodInvoker());


        
        gc->BarrierEnd();
    }
    void TStd::RegisterStd(GC* gc, TRootEnvironment* env)
    {
        env->permissions.canRegisterEverything=true;
        RegisterEnv(gc, env);
        RegisterRoot(gc,env);
        RegisterPath(gc,env);
        RegisterConsole(gc, env);
        RegisterIO(gc, env);
        RegisterNet(gc, env);
        RegisterSqlite(gc, env);
        RegisterVM(gc, env);
        RegisterJson(gc, env);
        RegisterDictionary(gc, env);
        RegisterCrypto(gc,env);
        RegisterOGC(gc, env);
        RegisterProcess(gc,env);
        RegisterClass(gc,env);

        gc->RegisterEverything(env);

        GCList ls(gc);

        TDictionary* dict = TDictionary::Create(ls);
         TDictionary* gc_dict = TDictionary::Create(ls);
         #if defined(CROSSLANG_ENABLE_FFI)
         TDictionary* ffi = TDictionary::Create(ls);
         RegisterFFI(gc,ffi);
         #endif
        dict->DeclareFunction(gc,"LoadNativePlugin","Load a native plugin, requires a dynamic linker and shared build of libcrosslang",{"path"},[gc,env](GCList& ls, std::vector<TObject> args)->TObject {
            Tesses::Framework::Filesystem::VFSPath path;
            if(GetArgumentAsPath(args,0,path))
            {
                LoadPlugin(gc,env,path);
            }
            return nullptr;
        });
        gc_dict->DeclareFunction(gc,"Collect","Collect garbage",{},[](GCList& ls, std::vector<TObject> args)->TObject{
            ls.GetGC()->Collect();
            return nullptr;
        });
        gc_dict->DeclareFunction(gc,"BarrierBegin","Lock globally",{},[](GCList& ls,std::vector<TObject> args)->TObject {
            ls.GetGC()->BarrierBegin();
            return nullptr;
        });
        gc_dict->DeclareFunction(gc, "BarrierEnd","Unlock globally",{},[](GCList& ls, std::vector<TObject> args)->TObject {
            ls.GetGC()->BarrierEnd();
            return  nullptr;
        });
        gc->BarrierBegin();
        #if defined(CROSSLANG_ENABLE_FFI)
        env->SetVariable("FFI", ffi);
        #endif
        env->SetVariable("Reflection",dict);
        env->SetVariable("GC", gc_dict);
        gc->BarrierEnd();
        env->permissions.locked=true;
    }
}
