#include "CrossLang.hpp"



namespace Tesses::CrossLang
{
    class ProcessObject : public TNativeObject {
        public:
            ProcessObject()
            {
                arguments=nullptr;
                environment=nullptr;
                
            }
            ProcessObject(GCList& ls)
            {
                arguments = TList::Create(ls);
                environment = TList::Create(ls);
                process.includeThisEnv=true;
                process.redirectStdIn=false;

                process.redirectStdOut=false;

                process.redirectStdErr=false;
            }
            TList* arguments;
            TList* environment;
            Tesses::Framework::Platform::Process process;
            std::string TypeName()
            {
                return "Process";
            }
            void Mark()
            {
                if(this->marked) return;
                this->marked=true;
                if(arguments != nullptr) arguments->Mark();
                if(environment != nullptr) environment->Mark();
            }
            TObject CallMethod(GCList& ls, std::string key, std::vector<TObject> args)
            {
                if(key == "setFileName" && GetArgument(args,0,process.name))
                {
                    return process.name;
                }
                if(key == "getFileName")
                {
                    return process.name;
                }
                
                if(key == "setRedirectStandardInput" && GetArgument(args,0,process.redirectStdIn))
                {
                    return process.redirectStdIn;
                }
                if(key == "getRedirectStandardInput")
                {
                    return process.redirectStdIn;
                }
                if(key == "setRedirectStandardOutput" && GetArgument(args,0,process.redirectStdOut))
                {
                    return process.redirectStdOut;
                }
                if(key == "getRedirectStandardOutput")
                {
                    return process.redirectStdOut;
                }
                if(key == "setRedirectStandardError" && GetArgument(args,0,process.redirectStdErr))
                {
                    return process.redirectStdErr;
                }
                if(key == "getRedirectStandardError")
                {
                    return process.redirectStdErr;
                }
                if(key == "setWorkingDirectory" && GetArgument(args,0,process.workingDirectory))
                {
                    return process.workingDirectory;
                }
                if(key == "getWorkingDirectory")
                {
                    return process.workingDirectory;
                }
                if(key == "setInheritParentEnvironment" && GetArgument(args,0,process.includeThisEnv))
                {
                    return process.includeThisEnv;
                }
                if(key == "getInheritParentEnvironment")
                {
                    return process.includeThisEnv;
                }
                if(key == "getArguments")
                {
                    if(arguments == nullptr) return nullptr;
                    return arguments;
                }
                if(key == "setArguments")
                {
                    if(GetArgumentHeap(args,0,arguments))
                    {
                        return arguments;
                    }
                }
                if(key == "getEnvironment")
                {
                    if(environment == nullptr) return nullptr;
                    return environment;
                }
                if(key == "getEnvironment")
                {
                    if(GetArgumentHeap(args,0,environment))
                    {
                        return environment;
                    }
                }
                if(key == "Start")
                {
                    this->process.args.push_back(process.name);
                    if(arguments != nullptr)
                    {
                        for(int64_t i = 0; i < arguments->Count(); i++)
                        {
                            TObject argVal=arguments->Get(i);
                            std::string argValStr;
                            if(GetObject(argVal,argValStr))
                                this->process.args.push_back(argValStr);
                        }
                    }
                    if(environment != nullptr)
                    {
                        for(int64_t i = 0; i < environment->Count(); i++)
                        {
                            TObject arg = environment->Get(i);
                            std::string argstr;
                            if(GetObject(arg,argstr))
                            {
                                auto kvp = Tesses::Framework::Http::HttpUtils::SplitString(argstr,"=",2);
                                if(kvp.size()==2)
                                {
                                    process.env.push_back(std::pair<std::string,std::string>(kvp[0],kvp[1]));
                                }
                                else if(kvp.size()==1)
                                {
                                    process.env.push_back(std::pair<std::string,std::string>(kvp[0],""));
                                }
                            }
                        }
                    }
                    return process.Start();
                }
                if(key == "Join" || key == "WaitForExit")
                {
                    return (int64_t)process.WaitForExit();
                }
                if(key == "getHasExited")
                {
                    return process.HasExited();
                }
                if(key == "Terminate")
                {
                    process.Kill(SIGTERM);
                }
                if(key == "CloseStdInNow")
                {
                    process.CloseStdInNow();
                    
                }
                int64_t k;
                if(key == "Kill" && GetArgument(args,0,k))
                {
                    process.Kill((int)k);
                }
                if(key == "getStandardInput")
                {
                    return TStreamHeapObject::Create(ls,process.GetStdinStream());
                }
                if(key == "getStandardOutput")
                {
                    return TStreamHeapObject::Create(ls,process.GetStdoutStream());
                }
                if(key == "getStandardError")
                {
                    return TStreamHeapObject::Create(ls,process.GetStderrStream());
                }
                return Undefined();
            }
    };
    static TObject Process_Start(GCList& ls, std::vector<TObject> args)
    {
        
        //Process.Start({
        // FileName = "git",
        // Arguments = ["clone","https://gitea.site.tesses.net/tesses50/crosslang.git"],
        // Environment = [],
        // InheritParentEnvironment=true
        //})

        TDictionary* dict;

        if(GetArgumentHeap(args,0,dict))
        {
            auto process = TNativeObject::Create<ProcessObject>(ls);
            auto name = dict->GetValue("FileName");
            auto inh = dict->GetValue("InheritParentEnvironment");
            auto rStdIn = dict->GetValue("RedirectStandardInput");
            auto rStdOut = dict->GetValue("RedirectStandardOutput");

            auto rStdErr = dict->GetValue("RedirectStandardError");
            auto workingDirectory = dict->GetValue("WorkingDirectory");
            process->process.includeThisEnv=true;
            process->process.redirectStdIn=false;

            process->process.redirectStdOut=false;

            process->process.redirectStdErr=false;
            GetObject(inh,process->process.includeThisEnv);
            GetObject(rStdIn,process->process.redirectStdIn);
            GetObject(rStdOut,process->process.redirectStdOut);
            GetObject(rStdErr,process->process.redirectStdErr);
            
            GetObject(name,process->process.name);

            Tesses::Framework::Filesystem::VFSPath wdPath;

            if(GetObject(workingDirectory,wdPath))
            {
                process->process.workingDirectory= wdPath.MakeAbsolute().ToString();
            }

            GetObject(workingDirectory,process->process.workingDirectory);
            


            process->process.args.push_back(process->process.name);
            auto argumentsO = dict->GetValue("Arguments");
            TList* arguments;
            if(GetObjectHeap(argumentsO,arguments))
            {
                for(int64_t i = 0; i < arguments->Count(); i++)
                {
                    TObject arg = arguments->Get(i);
                    std::string argstr;
                    if(GetObject(arg,argstr))
                    {
                        process->process.args.push_back(argstr);
                    }
                }
            }
            argumentsO = dict->GetValue("Environment");
            if(GetObjectHeap(argumentsO,arguments))
            {
                for(int64_t i = 0; i < arguments->Count(); i++)
                {
                    TObject arg = arguments->Get(i);
                    std::string argstr;
                    if(GetObject(arg,argstr))
                    {
                        auto kvp = Tesses::Framework::Http::HttpUtils::SplitString(argstr,"=",2);
                        if(kvp.size()==2)
                        {
                            process->process.env.push_back(std::pair<std::string,std::string>(kvp[0],kvp[1]));
                        }
                        else if(kvp.size()==1)
                        {
                            process->process.env.push_back(std::pair<std::string,std::string>(kvp[0],""));
                        }
                    }
                }
            }
            if(process->process.Start())
            {
                return process;
            }
        }

        return nullptr;
    }
    static TObject New_Process(GCList& ls, std::vector<TObject> args)
    {
        return TNativeObject::Create<ProcessObject>(ls,ls);
    }

    void TStd::RegisterProcess(GC* gc,TRootEnvironment* env)
    {

        env->permissions.canRegisterProcess=true;
        GCList ls(gc);
        TDictionary* dict = TDictionary::Create(ls);
        dict->DeclareFunction(gc,"Start","Start a process",{"process_object"},Process_Start);
       
        gc->BarrierBegin();
        env->SetVariable("Process",dict);
        auto process = env->EnsureDictionary(gc,"New");
        process->DeclareFunction(gc, "Process", "Create process",{},New_Process);
        gc->BarrierEnd();
    }
}
