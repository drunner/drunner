#include "buildnum.h"

std::string getVersionStr()
{
#ifdef _WIN32
   return std::string("WINDOWS - ") + __DATE__ + " " + __TIME__;
#else
   return VERSION_STR ;
#endif
}

std::string getVersionStrShort()
{
#ifdef _WIN32
   return "Win Build";
#else
   return VERSION_STR_SHORT ;
#endif
}
