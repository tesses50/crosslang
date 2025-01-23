#include "CrossLang.hpp"
#if defined(CROSSLANG_ENABLE_SQLITE)
#include "../sqlite/sqlite3.h"
#endif
#include <iostream>
namespace Tesses::CrossLang {
    #if defined(CROSSLANG_ENABLE_SQLITE)
    static int sqlcollector(void* user, int count,char** vals, char** keys)
    {
        
        std::pair<GCList*,TList*>* ls2 = static_cast<std::pair<GCList*,TList*>*>(user);
        TDictionary* dict = TDictionary::Create(ls2->first);
        for(int i = 0; i < count; i++)
        {
            std::string key = keys[i] == nullptr ? "" : keys[i];
            
            if(vals[i] == nullptr)
            {
                dict->SetValue(key,nullptr);
            }
            else
            {
                dict->SetValue(key,vals[i]);
            }
        }
        ls2->second->Add(dict);

        return 0;
    }
    TObject Sqlite_Open(GCList& ls, std::vector<TObject> args,TRootEnvironment* env)
    {
        Tesses::Framework::Filesystem::VFSPath p;
        if(GetArgumentAsPath(args,0,p))
        {
            if(env->permissions.sqlite3Scoped)
            {
                p = env->permissions.sqliteOffsetPath / p.CollapseRelativeParents();
            }
            std::string name = p.ToString();
            
            sqlite3* sqlite;
            int rc =sqlite3_open(name.c_str(),&sqlite);
            if(rc)
            {
                std::string error = sqlite3_errmsg(sqlite);
                sqlite3_close(sqlite);
                return error;
            }
            return TNative::Create(ls,sqlite,[](void* a)->void {
                if(a != nullptr)
                    sqlite3_close((sqlite3*)a);
            });
        }

        return Undefined();
        
    }
    TObject Sqlite_Exec(GCList& ls, std::vector<TObject> args)
    {
        if(args.size() == 2 && std::holds_alternative<THeapObjectHolder>(args[0]) && std::holds_alternative<std::string>(args[1]))
        {
            TNative* native = dynamic_cast<TNative*>(std::get<THeapObjectHolder>(args[0]).obj);
            std::string sqlStatement = std::get<std::string>(args[1]);
            if(native != nullptr && !native->GetDestroyed())
            {
                sqlite3* sql = (sqlite3*)native->GetPointer();
                TList* myLs = TList::Create(ls);
                std::pair<GCList*,TList*> result(&ls, myLs);
                char* err;
                int res = sqlite3_exec(sql,sqlStatement.c_str(),sqlcollector,&result,&err);
                if(res != SQLITE_OK)
                {
                    std::string errstr = err == nullptr ? "" : err;
                    sqlite3_free(err);
                    return errstr;
                }
                
                return myLs;
            }
        }

        return Undefined();
        
    }

    TObject Sqlite_Escape(GCList& ls, std::vector<TObject> args)
    {
        if(args.size() == 1 && std::holds_alternative<std::string>(args[0]))
        {
            std::string srcStr = std::get<std::string>(args[0]);
            std::string myStr = "\'";
            for(auto c : srcStr)
            {
                if(c == '\'') myStr += "\'\'";
                else
                    myStr += c;
            }
            myStr += '\'';
            return myStr;
        }
        return Undefined();
    }
    
     TObject Sqlite_Close(GCList& ls, std::vector<TObject> args)
    {
        if(args.size() == 1 && std::holds_alternative<THeapObjectHolder>(args[0]))
        {
            TNative* native = dynamic_cast<TNative*>(std::get<THeapObjectHolder>(args[0]).obj);
           
            if(native != nullptr && !native->GetDestroyed())
            {
                native->Destroy();
            }
        }
        return Undefined();
    }
    
    #endif
    void TStd::RegisterSqlite(GC* gc,TRootEnvironment* env)
    {

        env->permissions.canRegisterSqlite=true;
        #if defined(CROSSLANG_ENABLE_SQLITE)
        GCList ls(gc);
        TDictionary* dict = TDictionary::Create(ls);
        dict->DeclareFunction(gc,"Open","Opens the database (returns database handle or an error message as string or undefined)",{"filename"},[env](GCList& ls, std::vector<TObject> args)->TObject {
            return Sqlite_Open(ls,args,env);
        });
        dict->DeclareFunction(gc,"Exec","Execute sql (returns dictionary of columns key=value, an error message as string or undefined)",{"handle","sql"},Sqlite_Exec);
        dict->DeclareFunction(gc,"Close","Close sql database",{"handle"},Sqlite_Close);
        dict->DeclareFunction(gc,"Escape","Escape sql text",{"text"},Sqlite_Escape);
       
        gc->BarrierBegin();
        env->DeclareVariable("Sqlite", dict);
        gc->BarrierEnd();
        #endif
    }
}
