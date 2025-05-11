#include "CrossLang.hpp"
namespace Tesses::CrossLang {
    TDateTime::TDateTime()
    {
        this->dt=new Tesses::Framework::Date::DateTime();
    }
    TDateTime::TDateTime(Tesses::Framework::Date::DateTime t)
    {
        this->dt = new Tesses::Framework::Date::DateTime(t);
    }
    TDateTime::TDateTime(const TDateTime& dt)
    {
        this->dt = new Tesses::Framework::Date::DateTime(*dt.dt);
    }
    Tesses::Framework::Date::DateTime& TDateTime::GetDate()
    {
        return *this->dt;
    }
    TDateTime::~TDateTime()
    {
        delete this->dt;
    }
}