Extending the runtime
=====================

See [this](CPP_LAYER.md)

There are a couple ways of doing this

- Creating a shared library (only works if using shared libs)
- Embeding the runtime
- Forking crosslang and adding functionality (I don't support pull requests at this time)

### Shared Library

This assumes you are on linux, if on windows use MyPlugin.dll on mac libMyPlugin.dylib in crosslang project (it would be a good idea to check Env.Platform)

CMakeLists.txt:
```cmake
cmake_minimum_required(VERSION 3.16)

project(MyPlugin)

find_package(TessesCrossLang REQUIRED)

add_library(MyPlugin SHARED lib.cpp)

target_link_libraries(MyPlugin PUBLIC TessesCrossLang::crosslang_shared)
```

lib.cpp:
```c++
#include <CrossLang.hpp>

using namespace Tesses::CrossLang;

void MyPlugin(GC* gc, TRootEnvironment* env)
{
    GCList ls(gc);
    TDictionary* dict = TDictionary::Create(ls,{
        TDItem("MyFunction", TExternalMethod::Create(ls, "A description for function",{"myLong","$optionalList"},[](GCList& ls, std::vector<TObject> args)->TObject {
            int64_t n;
            if(GetArgument(args,0,n))
            {
                TList* myLs;
                if(GetArgumentHeap(args,1,myLs))
                {
                    //do something with the list
                    myLs->Add(n);
                    return TList::Create(ls, {n,myLs});
                }


                return TList::Create(ls, {n});
            }

            return nullptr;
        }))
    });

    gc->BarrierBegin();
    env->SetVariable("MyPlugin",dict);
    gc->BarrierEnd();
}

CROSSLANG_PLUGIN(MyPlugin);

```

main.tcross:
```go
func main(args)
{
    Reflection.LoadNativePlugin(FS.MakeFull("libMyPlugin.so"));

    var res = MyPlugin.MyFunction(42);
    if(TypeOf(res) == "List")
    {
        Console.WriteLine(res);
    }

    var res = MyPlugin.MyFunction(42,["Hello","John"]);
    if(TypeOf(res) == "List")
    {
        Console.WriteLine(res);
    }
}
```


### Embeding the runtime

CMakeLists.txt:
```cmake
cmake_minimum_required(VERSION 3.16)

project(MyCrossLangBinary)

find_package(TessesCrossLang REQUIRED)

add_executable(MyCrossLangBinary SHARED app.cpp)

target_link_libraries(MyCrossLangBinary PUBLIC TessesCrossLang::crosslang_shared) # or crosslang_static for static linking


```

app.cpp:
```c++
#include <CrossLang.hpp>

using namespace Tesses::CrossLang;
using namespace Tesses::Framework;

void MyPlugin(GC* gc, TRootEnvironment* env)
{
    GCList ls(gc);
    TDictionary* dict = TDictionary::Create(ls,{
        TDItem("MyFunction", TExternalMethod::Create(ls, "A description for function",{"myLong","$optionalList"},[](GCList& ls, std::vector<TObject> args)->TObject {
            int64_t n;
            if(GetArgument(args,0,n))
            {
                TList* myLs;
                if(GetArgumentHeap(args,1,myLs))
                {
                    //do something with the list
                    myLs->Add(n);
                    return TList::Create(ls, {n,myLs});
                }


                return TList::Create(ls, {n});
            }

            return nullptr;
        }))
    });

    gc->BarrierBegin();
    env->SetVariable("MyPlugin",dict);
    gc->BarrierEnd();
}

int main(int argc, char** argv)
{
     TF_Init();
    if(argc < 2)
    {
        printf("USAGE: %s <filename.crvm> <args...>\n",argv[0]);
        return 1;
    }
    GC gc;
    gc.RegisterEverythingCallback(MyPlugin);
    gc.Start();
    GCList ls(gc);
    TRootEnvironment* env = TRootEnvironment::Create(ls,TDictionary::Create(ls));
    Tesses::Framework::Filesystem::LocalFilesystem fs;
    TStd::RegisterStd(&gc,env);
    env->LoadFileWithDependencies(&gc, &fs, fs.SystemToVFSPath(argv[1]));
    
    TList* args = TList::Create(ls);
    for(int arg=1;arg<argc;arg++)
        args->Add(std::string(argv[arg]));
   
    auto res = env->CallFunction(ls,"main",{args});
    int64_t iresult;
    if(GetObject(res,iresult))
        return (int)iresult;
    return 0;
}
```

main.tcross (needs to be compiled with crossc):
```go
func main(args)
{
    //no reflection necessary

    var res = MyPlugin.MyFunction(42);
    if(TypeOf(res) == "List")
    {
        Console.WriteLine(res);
    }

    var res = MyPlugin.MyFunction(42,["Hello","John"]);
    if(TypeOf(res) == "List")
    {
        Console.WriteLine(res);
    }
}
```

### Forking (implementing it in runtime library itself)
- edit include/CrossLang.hpp
- in TStd class insert, static void RegisterMyPlugin(GC* gc, TRootEnvironment* env); where MyPlugin is whatever you want
- edit src/runtime_methods/std.cpp
- in TStd::RegisterStd before register everything call TStd::RegisterMyPlugin(gc, env);
- in CMakeLists.txt within list(APPEND CROSSLANG_SOURCE **Many Source Files**) you must add src/runtime_methods/myplugin.cpp

src/runtime_methods/myplugin.cpp:
```c++
#include <CrossLang.hpp>
namespace Tesses::CrossLang 
{
    void TStd::RegisterMyPlugin(GC* gc, TRootEnvironment* env)
    {
        GCList ls(gc);
        TDictionary* dict = TDictionary::Create(ls,{
            TDItem("MyFunction", TExternalMethod::Create(ls, "A description for function",{"myLong","$optionalList"},[](GCList& ls, std::vector<TObject> args)->TObject {
                int64_t n;
                if(GetArgument(args,0,n))
                {
                    TList* myLs;
                    if(GetArgumentHeap(args,1,myLs))
                    {
                        //do something with the list
                        myLs->Add(n);
                        return TList::Create(ls, {n,myLs});
                    }


                    return TList::Create(ls, {n});
                }

                return nullptr;
            }))
        });

        gc->BarrierBegin();
        env->SetVariable("MyPlugin",dict);
        gc->BarrierEnd();
    }
}
```