
#include "CrossLang.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

namespace Tesses::CrossLang
{
    static TObject VM_Eval(GCList& ls, std::vector<TObject> args)
    {
        std::string str;
        if(GetArgument(args,0,str))
        {
            if(current_function != nullptr)
            {
                return current_function->env->Eval(ls,str);
            }
        }
        return nullptr;
    }
    static TObject Failure(GCList& ls, std::string reason)
    {
        TDictionary* dict = TDictionary::Create(ls);
        ls.GetGC()->BarrierBegin();
        dict->SetValue("Success",false);
        dict->SetValue("Reason", reason);
        ls.GetGC()->BarrierEnd();
        return dict;
    }
    static TObject Success(GCList& ls)
    {
        TDictionary* dict = TDictionary::Create(ls);
        ls.GetGC()->BarrierBegin();
        dict->SetValue("Success",true);
        ls.GetGC()->BarrierEnd();
        return dict;
    }
    static TObject VM_getCurrentEnvironment(GCList& ls2,std::vector<TObject> args)
    {
        if(current_function != nullptr) return current_function->env;
        return Undefined();
    }
    static TObject VM_Compile(GCList& ls, std::vector<TObject> args)
    {
        try {
        TDictionary* dict;
        if(!GetArgumentHeap<TDictionary*>(args,0,dict)) return Undefined();
        /*
            VM.Compile
            {
                Name = "My name",
                Info = "",
                Sources = [],
                Version = "",
                Dependencies = [{Name="Name", Version}],
                ResourceDirectory = "",
                
            }   
        */

        std::string name = "Out";
        std::vector<std::pair<std::string,std::string>> sources;
        TVMVersion version(TVM_MAJOR,TVM_MINOR,TVM_PATCH,TVM_BUILD,TVM_VERSIONSTAGE);
        std::vector<std::pair<std::string,TVMVersion>> dependencies;
        std::string info;
        TVFSHeapObject* vfsHO =nullptr;

        ls.GetGC()->BarrierBegin();
        TObject _name = dict->GetValue("Name");
        TObject _version = dict->GetValue("Version");
        TObject _sources = dict->GetValue("Sources");
        TObject _dependencies = dict->GetValue("Dependencies");
        TObject _info = dict->GetValue("Info");
        TObject _resourceFileSystem = dict->GetValue("ResourceFileSystem");
        
        TObject _out = dict->GetValue("Output");
        TList* _depList; TList* srcLst;
        GetObject<std::string>(_name,name);
        GetObject<std::string>(_info,info);
        GetObjectHeap(_resourceFileSystem, vfsHO);
        std::string v2;
        if(GetObject<std::string>(_version,v2))
            TVMVersion::TryParse(v2, version);

        if(GetObjectHeap<TList*>(_dependencies,_depList))
        {
            for(int64_t i = 0; i < _depList->Count(); i++)
            {
                TObject _dep = _depList->Get(i);
                TDictionary* _depD;
                if(GetObjectHeap<TDictionary*>(_dep, _depD))
                {
                    TObject _name2 = _depD->GetValue("Name");
                    TObject _version2 = _depD->GetValue("Version");
                    std::string name2;
                    std::string version2;
                    TVMVersion version02;

                    if(GetObject<std::string>(_name2,name2) && GetObject<std::string>(_version2,version2) && TVMVersion::TryParse(version2,version02))
                    {
                        dependencies.push_back(std::pair<std::string,TVMVersion>(name2, version02));
                    }
                }
            }
            
        }
        if(GetObjectHeap<TList*>(_sources,srcLst))
        {
            for(int64_t i = 0; i < srcLst->Count(); i++)
            {
                TObject _src = srcLst->Get(i);
                TDictionary* _srcD;
                if(GetObjectHeap<TDictionary*>(_src, _srcD))
                {
                    TObject _sourceTxt = _srcD->GetValue("Source");
                    TObject _filename = _srcD->GetValue("FileName");
                    

                    std::string srctxt = "";
                    std::string filename = "memory_" + std::to_string(i) + ".tcross";
                    bool fromFile = false;

                    GetObject<std::string>(_sourceTxt,srctxt);
                    GetObject<std::string>(_filename,filename);
                   
                    
                    sources.push_back(std::pair<std::string,std::string>(srctxt,filename));
                }
            }
            
        }
       
        ls.GetGC()->BarrierEnd();

        std::vector<LexToken> tokens;
        for(auto source : sources)
        {
            std::stringstream strm(source.first);

            int res = Lex(source.second, strm, tokens);
            if(res != 0)
            {
                    return Failure(ls, "Lex error in file \"" + source.second + "\":" + std::to_string(res));
            }
        }
        Parser parser(tokens);
        SyntaxNode n = parser.ParseRoot();
        CodeGen gen;
        gen.GenRoot(n);
        gen.dependencies = dependencies;
        gen.info = info;
        gen.name = name;
        gen.version = version;
        std::string outpath;
        TStreamHeapObject* stream;
        if(GetObjectHeap<TStreamHeapObject*>(_out, stream))
        {
            gen.Save(vfsHO != nullptr ? vfsHO->vfs : nullptr, stream->stream);
        }

        

        return Success(ls);
        } catch(std::exception& ex)
        {
            return Failure(ls, ex.what());
        }
    }
    void TStd::RegisterVM(GC* gc,TRootEnvironment* env)
    {

        env->permissions.canRegisterVM=true;
        GCList ls(gc);
        TDictionary* dict = TDictionary::Create(ls);
        dict->DeclareFunction(gc, "getRootEnvironmentAsDictionary","Get root environment as a dictionary",{},[env](GCList& ls, std::vector<TObject> args)-> TObject{
            return env->GetDictionary();
        });
        dict->DeclareFunction(gc, "getRootEnvironment","Get root environment, for reflection purposes",{},[env](GCList& ls2,std::vector<TObject> args)->TObject {return env;});
        dict->DeclareFunction(gc, "getCurrentEnvironment","Get current environment, for reflection purposes",{},VM_getCurrentEnvironment);
        dict->DeclareFunction(gc, "CreateEnvironment","Create root environment",{"$dict"},[](GCList& ls,std::vector<TObject> args)->TObject{
            TDictionary* dict;
            if(GetArgumentHeap(args,0,dict))
            {
                return TRootEnvironment::Create(ls,dict);
            }
            else
            {
                return TRootEnvironment::Create(ls,TDictionary::Create(ls));
            }
        });
        dict->DeclareFunction(gc, "LoadExecutable", "Load a crossvm executable",{"stream"},[](GCList& ls,std::vector<TObject> args)->TObject{
            TStreamHeapObject* strm;
            if(GetArgumentHeap(args,0,strm))
            {
                TFile* f =TFile::Create(ls);
                f->Load(ls.GetGC(),strm->stream);
                return f;
            }
            return nullptr;
        });
        dict->DeclareFunction(gc, "Eval", "Eval source code",{"source"}, VM_Eval);
        dict->DeclareFunction(gc, "Compile", "Compile Source",{"dict"},VM_Compile);
        
        
        
        gc->BarrierBegin();
        env->DeclareVariable("VM", dict);
        gc->BarrierEnd();
    }
    
}