#include "CrossLang.hpp"
#include <iostream>
#include <sstream>
namespace Tesses::CrossLang {
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
        

        Tesses::Framework::Streams::Stream* file;
        if(vfs->RegularFileExists(filename) && (file = vfs->OpenFile(filename,"rb")) != nullptr)
        {
            
            GCList ls(gc);
            TFile* f = TFile::Create(ls);
            f->Load(gc, file);
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
    TObject TEnvironment::LoadFile(GC* gc, TFile* file)
    {
          for(auto fn : file->functions)
        {
            
            std::string name;
            
            
            if(fn.first.size() < 2) throw VMException("No function name.");

            std::vector<std::string> items(fn.first.begin()+1, fn.first.end());

            if(fn.second >= file->chunks.size()) throw VMException("ChunkId out of bounds.");
            TFileChunk* chunk = file->chunks[fn.second];
            

            if(items.size() == 0)
                throw VMException("Function name can't be empty.");

            else if(items.size() == 1)
            {
                GCList ls(gc);
                TClosure* closure=TClosure::Create(ls,this,file,fn.second);
                closure->documentation = fn.first[0];
                gc->BarrierBegin();
                this->DeclareVariable(items[0],closure);
                gc->BarrierEnd();
            }
            else
            {
                GCList ls(gc);
                TClosure* closure=TClosure::Create(ls,this,file,fn.second);
                closure->documentation = fn.first[0];
                TObject v = this->GetVariable(items[0]);
                TDictionary* dict=nullptr;
                if(std::holds_alternative<THeapObjectHolder>(v))
                {
                    dict=dynamic_cast<TDictionary*>(std::get<THeapObjectHolder>(v).obj);
                    if(dict == nullptr)
                    {
                        dict = TDictionary::Create(ls);
                        gc->BarrierBegin();
                        this->SetVariable(items[0],dict);
                        gc->BarrierEnd();
                    }
                }
                else
                {
                    dict = TDictionary::Create(ls);
                    gc->BarrierBegin();
                    this->DeclareVariable(items[0],dict);
                    gc->BarrierEnd();
                }

                for(size_t i = 1; i < items.size()-1; i++)
                {
                    gc->BarrierBegin();
                    auto v = dict->GetValue(items[i]);
                    gc->BarrierEnd();
                    if(std::holds_alternative<THeapObjectHolder>(v))
                    {
                        auto dict2=dynamic_cast<TDictionary*>(std::get<THeapObjectHolder>(v).obj);
                        if(dict2 == nullptr)
                        {
                            dict2 = TDictionary::Create(ls);
                            gc->BarrierBegin();
                            dict->SetValue(items[i],dict2);
                            gc->BarrierEnd();
                        }
                        dict = dict2;
                    }
                    else
                    {
                        auto dict2 = TDictionary::Create(ls);
                        gc->BarrierBegin();
                        dict->SetValue(items[i],dict2);
                        gc->BarrierEnd();
                        dict = dict2;
                    }
                }
                gc->BarrierBegin();
                dict->SetValue(items[items.size()-1],closure);
                gc->BarrierEnd();
            }
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
        Tesses::Framework::Streams::Stream* file;

        if(vfs->RegularFileExists(path) && (file = vfs->OpenFile(path,"rb")) != nullptr)
        {
            
            GCList ls(gc);
            TFile* f = TFile::Create(ls);
            f->Load(gc, file);
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
        return this->dict->SetValue(key,value);
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
