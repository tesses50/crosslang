#include "CrossLang.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

namespace Tesses::CrossLang
{

    static TObject AstToTObject(GCList& ls,SyntaxNode node)
    {
        if(std::holds_alternative<std::nullptr_t>(node))
        return nullptr;
    if(std::holds_alternative<int64_t>(node))
    {
        return std::get<int64_t>(node);
    }
    if(std::holds_alternative<double>(node))
    {
        return std::get<double>(node);
    }
    if(std::holds_alternative<bool>(node))
    {
        return std::get<bool>(node);
    }
    if(std::holds_alternative<std::string>(node))
    {
        std::string str = std::get<std::string>(node);
        return str;
    }
    if(std::holds_alternative<char>(node))
    {
        char c = std::get<char>(node);
        return c;
    }
    if(std::holds_alternative<Undefined>(node))
    {
        auto r = TDictionary::Create(ls);
        ls.GetGC()->BarrierBegin();
        r->SetValue("Type",std::string(UndefinedExpression));
        ls.GetGC()->BarrierEnd();
        return r;
        
    }


    if(std::holds_alternative<AdvancedSyntaxNode>(node))
    {
        auto asn = std::get<AdvancedSyntaxNode>(node);
        auto r = TDictionary::Create(ls);
        ls.GetGC()->BarrierBegin();
        r->SetValue("Type",asn.nodeName);
        r->SetValue("IsExpression",asn.isExpression);
        TList* ls2 = TList::Create(ls);
        for(auto item : asn.nodes)
        {
            ls2->Add(AstToTObject(ls,item));
        }
        r->SetValue("Arguments", ls2);
        ls.GetGC()->BarrierEnd();
        return r;
    }
        return nullptr;
    }
    static TObject VM_SourceToAst(GCList& ls, std::vector<TObject> args)
    {
        std::string str;
        if(GetArgument(args,0,str))
        {
            std::stringstream strm(str);
            std::vector<LexToken> tokens;
            int res = Lex("memory.tcross",strm,tokens);
            if(res != 0)
            {
                throw VMException("Lex error at line: " + std::to_string(res));
            }
            Parser p(tokens);
            auto res2 =p.ParseRoot();
            return AstToTObject(ls,res2);
        }
        return nullptr;
    }
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
        std::vector<std::pair<std::string,TVMVersion>> tools;
        std::string info;
        std::string icon;
        bool debug=false;
        std::shared_ptr<Tesses::Framework::Filesystem::VFS> vfs;

        ls.GetGC()->BarrierBegin();
        TObject _name = dict->GetValue("Name");
        TObject _version = dict->GetValue("Version");
        TObject _sources = dict->GetValue("Sources");
        TObject _dependencies = dict->GetValue("Dependencies");
        TObject _tools = dict->GetValue("Tools");
        TObject _info = dict->GetValue("Info");
        TObject _icon = dict->GetValue("Icon");
        TObject _comptime = dict->GetValue("CompTime");
        TObject _resourceFileSystem = dict->GetValue("ResourceFileSystem");
        
        TObject _out = dict->GetValue("Output");
        TObject _dbg = dict->GetValue("Debug");
        TList* _toolList;
        TList* _depList; TList* srcLst;
        TRootEnvironment* comptimeEnv=nullptr;
        GetObject<std::string>(_name,name);
        GetObject<std::string>(_info,info);
        GetObject<std::string>(_icon,icon);
        GetObject(_resourceFileSystem, vfs);
        GetObjectHeap(_comptime,comptimeEnv);
        GetObject(_dbg,debug);
        std::string v2;
        if(GetObject<std::string>(_version,v2))
            TVMVersion::TryParse(v2, version);
        else
            GetObject(_version,version);

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
                    else if(GetObject<std::string>(_name2,name2) && GetObject(_version2,version02))
                    {
                        dependencies.push_back(std::pair<std::string,TVMVersion>(name2, version02));
                    }
                }
            }
            
        }
        if(GetObjectHeap<TList*>(_tools,_toolList))
        {
            for(int64_t i = 0; i < _toolList->Count(); i++)
            {
                TObject _dep = _toolList->Get(i);
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
                        tools.push_back(std::pair<std::string,TVMVersion>(name2, version02));
                    }
                    else if(GetObject<std::string>(_name2,name2) && GetObject(_version2,version02))
                    {
                        tools.push_back(std::pair<std::string,TVMVersion>(name2, version02));
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
        Parser parser(tokens,ls.GetGC(),comptimeEnv);
        parser.debug = debug;
        SyntaxNode n = parser.ParseRoot();
        CodeGen gen;
        gen.embedFS = vfs;
        gen.GenRoot(n);
        gen.dependencies = dependencies;
        gen.tools = tools;
        gen.info = info;
        gen.name = name;
        gen.version = version;
        gen.icon = icon;
        std::string outpath;
        std::shared_ptr<Tesses::Framework::Streams::Stream> stream;
        if(GetObject(_out, stream))
        {
            gen.Save(stream);
        }

        

        return Success(ls);
        } catch(std::exception& ex)
        {
            return Failure(ls, ex.what());
        }
    }
    static TObject VM_GetStacktrace(GCList& ls, std::vector<TObject> args)
    {
        if(current_function != nullptr)
        {
            if(current_function->thread != nullptr)
            {
                TList* list = TList::Create(ls);
                ls.GetGC()->BarrierBegin();
                for(auto item : current_function->thread->call_stack_entries)
                {
                    auto dict = TDictionary::Create(ls);
                    if(item->callable->closure->name)
                        dict->SetValue("Name", *item->callable->closure->name);
                    
                    dict->SetValue("Closure", item->callable->closure);
                    dict->SetValue("FileName", item->callable->file->name + "-" + item->callable->file->version.ToString() + ".crvm");
                    dict->SetValue("IP",(int64_t)item->ip);
                    if(item->srcline >= 1)
                    {
                        dict->SetValue("SourceFile", item->srcfile);
                        dict->SetValue("SourceLine",item->srcline);
                    }
                    list->Add(dict);
                }
                ls.GetGC()->BarrierEnd();
                return list;
            }
        }
        return nullptr;
    }
    void TStd::RegisterVM(GC* gc,TRootEnvironment* env)
    {
        env->permissions.canRegisterVM=true;
        GCList ls(gc);

        TDictionary* dict = TDictionary::Create(ls);
        dict->DeclareFunction(gc, "GetStacktrace","Get the current stack trace", {}, VM_GetStacktrace);
        dict->DeclareFunction(gc, "getRootEnvironmentAsDictionary","Get root environment as a dictionary",{},[env](GCList& ls, std::vector<TObject> args)-> TObject{
            return env->GetDictionary();
        });
        dict->DeclareFunction(gc, "getRootEnvironment","Get root environment, for reflection purposes",{},[env](GCList& ls2,std::vector<TObject> args)->TObject {return env;});
        dict->DeclareFunction(gc, "getCurrentEnvironment","Get current environment, for reflection purposes",{},VM_getCurrentEnvironment);
        dict->DeclareFunction(gc, "CreateEnvironment","Create root environment",{"$dict"},[env](GCList& ls,std::vector<TObject> args)->TObject{
            TDictionary* dict;
            if(GetArgumentHeap(args,0,dict))
            {
                auto renv =  TRootEnvironment::Create(ls,dict);
                renv->permissions.customConsole = env->permissions.customConsole;
                return renv;
            }
            else
            {
                auto renv = TRootEnvironment::Create(ls,TDictionary::Create(ls));
                renv->permissions.customConsole = env->permissions.customConsole;
                return renv;
            }
        });
        dict->DeclareFunction(gc, "LoadExecutable", "Load a crossvm executable",{"stream"},[](GCList& ls,std::vector<TObject> args)->TObject{
            std::shared_ptr<Tesses::Framework::Streams::Stream> strm;
            if(GetArgument(args,0,strm))
            {
                TFile* f =TFile::Create(ls);
                f->Load(ls.GetGC(),strm);
                return f;
            }
            return nullptr;
        });
        dict->DeclareFunction(gc, "Eval", "Eval source code",{"source"}, VM_Eval);
        dict->DeclareFunction(gc, "Compile", "Compile Source",{"dict"},VM_Compile);
        
        dict->DeclareFunction(gc, "SourceToAst", "Convert source to ast", {"source"}, VM_SourceToAst);
        dict->DeclareFunction(gc, "getRuntimeVersion","Get the runtime version",{},[](GCList& ls,std::vector<TObject> args)->TObject {
            return TVMVersion(TVM_MAJOR,TVM_MINOR,TVM_PATCH,TVM_BUILD,TVM_VERSIONSTAGE);
        });
        dict->DeclareFunction(gc, "getIsRunning","Is the program still running",{},[](GCList& ls, std::vector<TObject> args)->TObject {
            return Tesses::Framework::TF_IsRunning();
        });
        dict->DeclareFunction(gc, "RunEventLoopItteration","Run Event Loop Itteration",{},[](GCList& ls, std::vector<TObject> args)->TObject {
            Tesses::Framework::TF_RunEventLoopItteration();
            return Undefined();
        });
        dict->DeclareFunction(gc, "RunEventLoop","Run Event Loop",{},[](GCList& ls, std::vector<TObject> args)->TObject {
            Tesses::Framework::TF_RunEventLoop();
            return Undefined();
        });
        dict->DeclareFunction(gc, "Merge", "Merge crvm files", {"srcVFS","sourcePath","destVFS"},[](GCList& ls, std::vector<TObject> args)->TObject {
            std::shared_ptr<Tesses::Framework::Filesystem::VFS> srcVFS;
            std::shared_ptr<Tesses::Framework::Filesystem::VFS> destVFS;
            Tesses::Framework::Filesystem::VFSPath sourcePath;
            if(GetArgument(args,0, srcVFS) && GetArgumentAsPath(args,1,sourcePath) && GetArgument(args,2,destVFS))
            return Merge(srcVFS, sourcePath, destVFS);
            return Undefined();
        });
        dict->DeclareFunction(gc, "Disassemble","Disassemble crvm file",{"strm","vfs","$generateJSON","$extractResources"},[](GCList& ls, std::vector<TObject> args)->TObject {
            std::shared_ptr<Tesses::Framework::Streams::Stream> strm;
            std::shared_ptr<Tesses::Framework::Filesystem::VFS> vfs;
            bool generateJSON=true;
            bool extractResources=true;
            if(GetArgument(args,0,strm) && GetArgument(args,1, vfs))
            {
                GetArgument(args,2,generateJSON);
                GetArgument(args,3,extractResources);
                Disassemble(strm,vfs, generateJSON, extractResources);
            }
            return Undefined();
        });
        dict->DeclareFunction(gc, "Assemble", "Assemble crvm file",{"vfs"},[](GCList& ls, std::vector<TObject> args)->TObject {
            std::shared_ptr<Tesses::Framework::Filesystem::VFS> vfs;
            if(GetArgument(args,0, vfs))
                return Assemble(vfs);
            return Undefined();
        });
        
        gc->BarrierBegin();
       
        

        env->DeclareVariable("VM", dict);
      
        gc->BarrierEnd();
    }
    
}