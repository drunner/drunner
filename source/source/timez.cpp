#include <sstream>
#include <ctime>
#include <time.h>

#include "timez.h"
#include "globallogger.h"

#ifdef _WIN32
#include "strptime.h"
#endif


timez::timez()
{
   restart();
}

void timez::restart()
{
   mStart = std::chrono::steady_clock::now();
}

int timez::getmilliseconds()
{
   auto end = std::chrono::steady_clock::now();
   int t=(int)std::chrono::duration_cast<std::chrono::microseconds>(end - mStart).count();
   return t;
}

std::string timez::getelpased()
{
   int ms = getmilliseconds();
   std::ostringstream oss;
   oss << 0.01*((ms+5000)/10000) <<"s";
   return oss.str();
}


namespace timeutils
{

   std::string getDateTimeStr()
   {
      char s[100];

      auto tnow = std::chrono::system_clock::now();
      std::time_t now_time = std::chrono::system_clock::to_time_t(tnow);

      strftime(s, 99, "%Y_%m_%d__%H_%M_%S", std::localtime(&now_time));
      return std::string(s);
   }

   std::string getArchiveName(std::string servicename)
   {
      return getDateTimeStr() + "___" + servicename + ".dbk";
   }

   std::chrono::system_clock::time_point archiveName2Time(std::string aname)
   {
      size_t pos = aname.find("___");
      if (pos == std::string::npos)
         logmsg(kLERROR, "Impure archive: " + aname);

      aname.erase(pos);

      return dateTimeStr2Time(aname);
   }

   std::chrono::system_clock::time_point dateTimeStr2Time(std::string dts)
   {
      std::tm tm = {};
      char * rval = strptime(dts.c_str(), "%Y_%m_%d__%H_%M_%S", &tm);
      if (rval == NULL)
         logmsg(kLERROR, "Invalid time conversion - can't process " + dts);

      auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
      return tp;
   }

  
}