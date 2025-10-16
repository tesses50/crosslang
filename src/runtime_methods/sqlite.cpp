#include "CrossLang.hpp"
#include <TessesFramework/TessesFrameworkFeatures.h>

#include <iostream>
namespace Tesses::CrossLang {
    #if defined(TESSESFRAMEWORK_ENABLE_SQLITE)
    using namespace Tesses::Framework::Serialization;
     TObject Sqlite_Escape(GCList& ls, std::vector<TObject> args)
    {
        int64_t n;
        double d;
        bool b;


        std::string str;
        if(GetArgument(args,0,str))
        {
            return SQLiteDatabase::Escape(str);
        }
        if(GetArgument(args,0,n))
        {
            return std::to_string(n);
        }
        if(GetArgument(args,0,b))
        {
            return b ? "1" : "0";
        }
        if(GetArgument(args,0,d))
        {
            return std::to_string(d);
        }

        return "NULL";
    }
    
     
    class SQLiteObject : public TNativeObject
    {   
        public:
            SQLiteDatabase* db;
            SQLiteObject(Tesses::Framework::Filesystem::VFSPath path)
            {
                db=new SQLiteDatabase(path);
            }

            std::string TypeName()
            {
                return "SQLiteDatabase";
            }
            void Close()
            {
                if(this->db == nullptr) return;
                delete this->db;
                this->db = nullptr;
                
            }
            bool ToBool()
            {
                return this->db != nullptr;
            }

            TObject CallMethod(GCList& ls,std::string name, std::vector<TObject> args)
            {
                if(name == "Close") this->Close();
                if(name == "Escape") {
                    return Sqlite_Escape(ls,args);
                }
                if(name == "Exec")
                {
                    std::string arg;
                    if(GetArgument(args,0,arg))
                    {
                        if(this->db == nullptr) return nullptr;
                        std::vector<std::vector<std::pair<std::string, std::optional<std::string>>>> res;

                        this->db->Exec(arg,res);
                        
                        TList* list = TList::Create(ls);

                        for(auto& item : res)
                        {
                            TDictionary* dict = TDictionary::Create(ls);
                            for(auto& item2 : item)
                            {
                                if(item2.second)
                                dict->SetValue(item2.first,item2.second.value());
                                else
                                dict->SetValue(item2.first,nullptr);
                            }

                            list->Add(dict);
                        }

                        return list;
                    }
                }
                return Undefined();
            }
            

            ~SQLiteObject()
            {
                if(this->db != nullptr)
                    delete this->db;
            }
    };

    TObject Sqlite_Open(GCList& ls, std::vector<TObject> args,TRootEnvironment* env)
    {
        Tesses::Framework::Filesystem::VFSPath p;
        if(GetArgumentAsPath(args,0,p))
        {
            if(env->permissions.sqlite3Scoped)
            {
                p = env->permissions.sqliteOffsetPath / p.CollapseRelativeParents();
            }
            
            return TNativeObject::Create<SQLiteObject>(ls,p);
        }

        return Undefined();
        
    }
    TObject Sqlite_Exec(GCList& ls, std::vector<TObject> args)
    {
        SQLiteObject* sql;
        std::string cmd;
        if(GetArgumentHeap(args,0,sql) && GetArgument(args,1,cmd))
        {
            try { 
            return sql->CallMethod(ls,"Exec",{cmd});
            } catch(std::runtime_error& ex)
            {
                return (std::string)ex.what();
            }
        }

        return Undefined();
        
    }

   TObject Sqlite_Close(GCList& ls, std::vector<TObject> args)
    {
        SQLiteObject* sql;
        if(GetArgumentHeap(args,0,sql))
        {
            sql->Close();
        }
        return Undefined();
    }
    
    #endif
    void TStd::RegisterSqlite(GC* gc,TRootEnvironment* env)
    {

        env->permissions.canRegisterSqlite=true;
        #if defined(TESSESFRAMEWORK_ENABLE_SQLITE)
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
