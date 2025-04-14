
#include "CrossLang.hpp"
#include <unistd.h>
#include "../HowardHinnant_date/date.h"


namespace Tesses::CrossLang
{
    static int64_t ToLocalTime(int64_t local)
    {
	#if defined(__SWITCH__)
	local -= _timezone;
	if(_daylight)
	#else
        local -= timezone;
        if(daylight)
	#endif
        {
            auto epoch = date::sys_days{date::January/1/1970};
            epoch += date::days(local/86400);

            bool isDST = false;

            date::year_month_day ymd(epoch);
            
            auto month = (uint32_t)ymd.month();

            if(month > 3 && month < 11)
            {
                isDST=true;
            }
            else if(month == 3)
            {
                auto day = (uint32_t)ymd.day();
                if(day > 14) isDST=true;
                else if(day >= 8 && day <= 14)
                {
                    date::year_month_weekday ymw(epoch);
                    auto dow=ymw.weekday().c_encoding();
                    auto secondSunday = day - dow;
                    if(secondSunday < 8) secondSunday+=7;

                    if(day > secondSunday) isDST=true;
                    else if(day == secondSunday)
                    {
                        std::chrono::duration<int64_t> local_epoch_time(local);
                        date::hh_mm_ss hms(local_epoch_time%86400);
                        if(hms.hours().count() >= 2) isDST=true;
                    }
                }
            }    
            else if(month == 11)
            {
                auto day = (uint32_t)ymd.day();
                if(day >= 1 && day <= 7)
                {
                    date::year_month_weekday ymw(epoch);
                    auto dow=ymw.weekday().c_encoding();
                    int32_t firstSunday = (int32_t)day - (int32_t)dow;
                    if(firstSunday < 1) firstSunday+=7;

                    if(day < firstSunday) isDST=true;
                    else if(day == firstSunday)
                    {
                        std::chrono::duration<int64_t> local_epoch_time(local);
                        date::hh_mm_ss hms(local_epoch_time%86400);
                        if(hms.hours().count() <=1) isDST=true;
                    }
                }
            }
                
            if(isDST) local += 3600;
        }
        return local;
    }
    static TObject Time_getNow(GCList& ls, std::vector<TObject> args)
    {
        time_t t = time(NULL);
        return (time_t)t;
    }
    static TObject Time_Sleep(GCList& ls, std::vector<TObject> args)
    {
        int64_t msec;
        if(GetArgument(args,0,msec))
        {
            usleep(1000*msec);   
        }
        return nullptr;
    }
    static TObject Time_LocalTime(GCList& ls, std::vector<TObject> args)
    {
        //THIS code isn't the best but should work
        int64_t local;
        if(GetArgument(args,0,local))
        {
            local = ToLocalTime(local);
            
            std::chrono::duration<int64_t> local_epoch_time(local);
            date::hh_mm_ss hms(local_epoch_time%86400);
            auto epoch = date::sys_days{date::January/1/1970};
            epoch += date::days(local/86400);
            
    

            //date::days<int64_t> sys_days_since_epoch = date::days<int64_t>(gmt);


            // Convert sys_days to year_month_day
            date::year_month_day ymd = date::year_month_day(epoch);

            TDictionary* dict = TDictionary::Create(ls);
            ls.GetGC()->BarrierBegin();
            dict->SetValue("Hour",hms.hours().count());
            dict->SetValue("Minute",hms.minutes().count());
            dict->SetValue("Second",hms.seconds().count());

            dict->SetValue("Day",(int64_t)(uint32_t)ymd.day());
            dict->SetValue("Month",(int64_t)(uint32_t)ymd.month());
            dict->SetValue("Year",(int64_t)(int32_t)ymd.year());
            ls.GetGC()->BarrierEnd();
            return dict;
        }
        
        return nullptr;
    }
    static TObject Time_GMTTime(GCList& ls, std::vector<TObject> args)
    {
        int64_t gmt;
        if(GetArgument(args,0,gmt))
        {
            std::chrono::duration<int64_t> epoch_time(gmt);
            date::hh_mm_ss hms(epoch_time%86400);
            auto epoch = date::sys_days{date::January/1/1970};
            epoch += date::days(gmt/86400);
    

            //date::days<int64_t> sys_days_since_epoch = date::days<int64_t>(gmt);


            // Convert sys_days to year_month_day
            date::year_month_day ymd = date::year_month_day(epoch);

            TDictionary* dict = TDictionary::Create(ls);
            ls.GetGC()->BarrierBegin();
            dict->SetValue("Hour",hms.hours().count());
            dict->SetValue("Minute",hms.minutes().count());
            dict->SetValue("Second",hms.seconds().count());

            dict->SetValue("Day",(int64_t)(uint32_t)ymd.day());
            dict->SetValue("Month",(int64_t)(uint32_t)ymd.month());
            dict->SetValue("Year",(int64_t)(int32_t)ymd.year());
            ls.GetGC()->BarrierEnd();
            return dict;
        }
        return nullptr;
    }
    void TStd::RegisterTime(GC* gc,TRootEnvironment* env)
    {
        


        env->permissions.canRegisterTime=true;
        GCList ls(gc);
        TDictionary* dict = TDictionary::Create(ls);
        dict->DeclareFunction(gc, "GetLocalTime", "Get local time from epoch value",{"epoch"},Time_LocalTime);
        dict->DeclareFunction(gc, "GetGMTTime","Get the GMT time from epoch value",{"epoch"},Time_GMTTime);
        dict->DeclareFunction(gc, "getNow","Get the time right now, returns C's time(NULL) return value",{},Time_getNow);
        dict->DeclareFunction(gc, "Sleep","Sleep for a specified amount of milliseconds (multiply seconds by 1000 to get milliseconds)", {"ms"},Time_Sleep);

        gc->BarrierBegin();
	#if defined(__SWITCH__)
	dict->SetValue("Zone", (int64_t)-(_timezone)); 
        dict->SetValue("SupportsDaylightSavings",(int64_t)_daylight);
	#else
        dict->SetValue("Zone", (int64_t)-(timezone));
        dict->SetValue("SupportsDaylightSavings",(int64_t)daylight);
	#endif
        env->DeclareVariable("Time", dict);

        gc->BarrierEnd();
    }
}
