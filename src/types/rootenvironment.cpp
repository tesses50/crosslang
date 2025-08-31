#include "CrossLang.hpp"
#include <iostream>
#include <sstream>
namespace Tesses::CrossLang {
    bool TRootEnvironment::TryFindClass(std::vector<std::string>& name, size_t& index)
    {
        for(size_t i = 0; i < this->classes.size(); i++)
        {
            if(classes[i].first->classes.at(classes[i].second).name.size() != name.size()) continue;
            for(size_t j = 0; j < name.size(); j++)
                if(classes[i].first->classes.at(classes[i].second).name[j] != name[j]) continue;
            index=i;
            return true;
        }
        return false;
    }
    bool TRootEnvironment::HasVariableOrFieldRecurse(std::string key,bool setting)
    {
        std::string property=(setting? "set":"get") + key;
        if(this->HasVariable(property))
        {
            auto res = this->GetVariable(property);
            TCallable* callable;
            if(GetObjectHeap(res,callable)) return true;
        }

        return this->HasVariable(key);
    }
    TObject TRootEnvironment::GetVariable(GCList& ls, std::string key)
    {
        ls.GetGC()->BarrierBegin();
        if(this->HasVariable("get" + key))
        {
            auto item = this->GetVariable("get"+key);
            TCallable* callable;
            if(GetObjectHeap(item,callable))
            {
                ls.GetGC()->BarrierEnd();
                return callable->Call(ls,{});
            }
        }
        
        auto item = this->GetVariable(key);
        ls.GetGC()->BarrierEnd();

        return item;
    }
    TObject TRootEnvironment::SetVariable(GCList& ls, std::string key, TObject value)
    {
        ls.GetGC()->BarrierBegin();
        if(this->HasVariable("set" + key))
        {
            auto item = this->GetVariable("set"+key);
            TCallable* callable;
            if(GetObjectHeap(item,callable))
            {
                ls.GetGC()->BarrierEnd();
                return callable->Call(ls,{value});
            }
        }
        
        this->SetVariable(key,value);
        ls.GetGC()->BarrierEnd();

        return value;
    }
            
    void TRootEnvironment::LoadDependency(GC* gc,Tesses::Framework::Filesystem::VFS* vfs, std::pair<std::string,TVMVersion> dep)
    {
         for(auto item : this->dependencies)
            if(item.first == dep.first && item.second.CompareTo(dep.second) >= 0) return;
        std::string name = {};
        name.append(dep.first);
        name.push_back('-');
        name.append(dep.second.ToString());
        name.append(".crvm");
        std::string filename="/" + name;
        
        if(vfs->RegularFileExists(filename))
        {
            Tesses::Framework::Streams::Stream* file = vfs->OpenFile(filename,"rb");
            GCList ls(gc);
            TFile* f = TFile::Create(ls);
            f->Load(gc, file);
            delete file;
            LoadFileWithDependencies(gc, vfs, f);
        }
        else throw VMException("Could not open file: \"" + name + "\".");
    }
    TObject TEnvironment::Eval(GCList& ls,std::string code)
    {
        std::stringstream strm(code);
            std::vector<LexToken> tokens;
            int res =Lex("eval.tcross",strm,tokens);
            if(res != 0)
            {
                throw VMException("Lex error at line: " + std::to_string(res));
            }
            Parser parser(tokens);
            SyntaxNode n = parser.ParseRoot();
            CodeGen gen;
            gen.GenRoot(n);
            Tesses::Framework::Streams::MemoryStream ms(true);
            gen.Save(nullptr, &ms);
            ms.Seek(0,Tesses::Framework::Streams::SeekOrigin::Begin);
            TFile* f = TFile::Create(ls);
            f->Load(ls.GetGC(),&ms);
        return this->LoadFile(ls.GetGC(), f);
    }
    TDictionary* TEnvironment::EnsureDictionary(GC* gc, std::string key)
    {
        TObject item = this->GetVariable(key);
        TDictionary* dict;
        if(GetObjectHeap(item,dict)) return dict;
        GCList ls(gc);
        dict = TDictionary::Create(ls);
        this->DeclareVariable(key, dict);
        return dict;
    }
    void TEnvironment::DeclareVariable(GC* gc, std::vector<std::string> name, TObject o)
    {
        if(name.size() == 0)
                throw VMException("name can't be empty.");

            else if(name.size() == 1)
            {
                GCList ls(gc);
               
                gc->BarrierBegin();
                this->DeclareVariable(name[0],o);
                gc->BarrierEnd();
            }
            else
            {
                GCList ls(gc);
               
                TObject v = this->GetVariable(name[0]);
                TDictionary* dict=nullptr;
                if(std::holds_alternative<THeapObjectHolder>(v))
                {
                    dict=dynamic_cast<TDictionary*>(std::get<THeapObjectHolder>(v).obj);
                    if(dict == nullptr)
                    {
                        dict = TDictionary::Create(ls);
                        gc->BarrierBegin();
                        this->SetVariable(name[0],dict);
                        gc->BarrierEnd();
                    }
                }
                else
                {
                    dict = TDictionary::Create(ls);
                    gc->BarrierBegin();
                    this->DeclareVariable(name[0],dict);
                    gc->BarrierEnd();
                }

                for(size_t i = 1; i < name.size()-1; i++)
                {
                    gc->BarrierBegin();
                    auto v = dict->GetValue(name[i]);
                    gc->BarrierEnd();
                    if(std::holds_alternative<THeapObjectHolder>(v))
                    {
                        auto dict2=dynamic_cast<TDictionary*>(std::get<THeapObjectHolder>(v).obj);
                        if(dict2 == nullptr)
                        {
                            dict2 = TDictionary::Create(ls);
                            gc->BarrierBegin();
                            dict->SetValue(name[i],dict2);
                            gc->BarrierEnd();
                        }
                        dict = dict2;
                    }
                    else
                    {
                        auto dict2 = TDictionary::Create(ls);
                        gc->BarrierBegin();
                        dict->SetValue(name[i],dict2);
                        gc->BarrierEnd();
                        dict = dict2;
                    }
                }
                gc->BarrierBegin();
                dict->SetValue(name[name.size()-1],o);
                gc->BarrierEnd();
            }
    }

    TObject TEnvironment::LoadFile(GC* gc, TFile* file)
    {
        file->EnsureCanRunInCrossLang();
        for(size_t i = 0; i < file->classes.size(); i++)
        {
            this->GetRootEnvironment()->classes.push_back(std::pair<TFile*,uint32_t>(file,(uint32_t)i));
            std::vector<std::string> clsPart={"New"};
            clsPart.insert(clsPart.end(),file->classes[i].name.begin(),file->classes[i].name.end());
            GCList ls(gc);
            std::vector<std::string> name=file->classes[i].name;
            auto rootEnv = this->GetRootEnvironment();
            this->DeclareVariable(gc, clsPart, TExternalMethod::Create(ls,"Create instance of the class",{"$$args"},[rootEnv,file,i](GCList& ls, std::vector<TObject> args)->TObject{
                 return TClassObject::Create(ls, file,i,rootEnv,args);
            }));
            for(auto meth : file->classes[i].entry)
            {

                if(meth.isFunction && meth.modifier == TClassModifier::Static)
                {
                    std::vector<std::string> method=file->classes[i].name;
                    method.push_back(meth.name);
                    auto clo = TClosure::Create(ls,this,file,meth.chunkId);
                    clo->documentation = meth.documentation;
                    this->DeclareVariable(gc, method, clo);
                }
            }
            
        }
        for(auto fn : file->functions)
        {
            
            std::string name;
            
            
            if(fn.first.size() < 2) throw VMException("No function name.");

            std::vector<std::string> items(fn.first.begin()+1, fn.first.end());

            if(fn.second >= file->chunks.size()) throw VMException("ChunkId out of bounds.");
            TFileChunk* chunk = file->chunks[fn.second];
            GCList ls(gc);
            TClosure* closure=TClosure::Create(ls,this,file,fn.second);
            closure->documentation = fn.first[0];
            this->DeclareVariable(gc,items,closure);
            
        }
        if(!file->chunks.empty())
        {
            GCList ls(gc);
            TClosure* closure=TClosure::Create(ls,this,file,0);          
            return closure->Call(ls,{});
        }
        return nullptr;
    }
    void TRootEnvironment::LoadFileWithDependencies(GC* gc,Tesses::Framework::Filesystem::VFS* vfs, TFile* file)
    {
        this->dependencies.push_back(std::pair<std::string,TVMVersion>(file->name,file->version));
        for(auto item : file->dependencies)
        {
            LoadDependency(gc,vfs,item);
        }
        LoadFile(gc, file);
      
    }
    void TRootEnvironment::LoadFileWithDependencies(GC* gc,Tesses::Framework::Filesystem::VFS* vfs, Tesses::Framework::Filesystem::VFSPath path)
    {
       

        if(vfs->RegularFileExists(path))
        {
            Tesses::Framework::Streams::Stream* file=vfs->OpenFile(path,"rb");
            GCList ls(gc);
            TFile* f = TFile::Create(ls);
            f->Load(gc, file);
            delete file;
            Tesses::Framework::Filesystem::SubdirFilesystem dir(vfs,path.GetParent(),false);
            LoadFileWithDependencies(gc,&dir,f);
        }
        else throw VMException("Could not open file: \"" + path.GetFileName() + "\".");
        
    }
    TDictionary* TRootEnvironment::GetDictionary()
    {
        return this->dict;
    }
    TObject TRootEnvironment::GetVariable(std::string key)
    {
        return this->dict->GetValue(key);
    }
    void TRootEnvironment::SetVariable(std::string key, TObject value)
    {
        this->dict->SetValue(key,value);
    }
     void TRootEnvironment::DeclareVariable(std::string key, TObject value)
    {
        return this->dict->SetValue(key,value);
    }
    bool TRootEnvironment::HasVariable(std::string key)
    {
        return this->dict->HasValue(key);
    }
    bool TRootEnvironment::HasVariableRecurse(std::string key)
    {
        return this->dict->HasValue(key);
    }
    TEnvironment* TRootEnvironment::GetParentEnvironment()
    {
        return this;
    }
    TRootEnvironment* TRootEnvironment::GetRootEnvironment()
    {
        return this;
    }

    TRootEnvironment::TRootEnvironment(TDictionary* dict)
    {
        this->dict = dict;
    }

    void TRootEnvironment::Mark()
    {
        if(this->marked) return;
        this->marked = true;
        this->dict->Mark();
        if(this->permissions.customConsole != nullptr) this->permissions.customConsole->Mark();
        for(auto defer : this->defers) defer->Mark();
        if(this->error != nullptr) this->error->Mark();
    }
    TRootEnvironment* TRootEnvironment::Create(GCList* gc,TDictionary* dict)
    {
        TRootEnvironment* env=new TRootEnvironment(dict);
        GC* _gc = gc->GetGC();
        gc->Add(env);
        _gc->Watch(env);
        return env;
    }
    TRootEnvironment* TRootEnvironment::Create(GCList& gc,TDictionary* dict)
    {
        TRootEnvironment* env=new TRootEnvironment(dict);
        GC* _gc = gc.GetGC();
        gc.Add(env);
        _gc->Watch(env);
        return env;
    }

    bool TRootEnvironment::HandleException(GC* gc,TEnvironment* env, TObject err)
    {
        if(error != nullptr)
        {
            GCList ls(gc);
            return ToBool(error->Call(ls, {
                TDictionary::Create(ls,{
                    TDItem("IsBreakpoint",false),
                                    TDItem("Exception",err),
                                    TDItem("Environment", env)
                })
            }));
        }
        return false;
    }
    bool TRootEnvironment::HandleBreakpoint(GC* gc,TEnvironment* env, TObject err)
    {
        if(error != nullptr)
        {
            GCList ls(gc);
            return ToBool(error->Call(ls, {
                TDictionary::Create(ls,{
                        TDItem("IsBreakpoint",true),
                        TDItem("Breakpoint",err),
                        TDItem("Environment", env)
                })
            }));
        }
        return true;
    }
    void TRootEnvironment::RegisterOnError(TCallable* call)
    {
        this->error = call;
    }
};
