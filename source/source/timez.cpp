#include <sstream>
#include <ctime>
#include <time.h>
#include <Poco/DateTime.h>
#include <Poco/LocalDateTime.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/DateTimeParser.h>
#include <Poco/Exception.h>

#include "timez.h"
#include "globallogger.h"

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
   static const std::string jformat = "%Y_%m_%d__%H_%M_%S";

   std::string getDateTimeStr()
   {
      Poco::LocalDateTime now;
      return Poco::DateTimeFormatter::format(now, jformat);
   }

   std::string getLogTimeStamp()
   {
      Poco::LocalDateTime now;
      return Poco::DateTimeFormatter::format(now, "%Y-%m-%d %H:%M");
   }

   std::string getArchiveName(std::string servicename)
   {
      return getDateTimeStr() + "___" + servicename + ".dbk";
   }

   Poco::DateTime  archiveName2Time(std::string aname)
   {
      size_t pos = aname.find("___");
      if (pos == std::string::npos)
         logmsg(kLERROR, "Impure archive: " + aname);

      aname.erase(pos);

      return dateTimeStr2Time(aname);
   }

   Poco::DateTime dateTimeStr2Time(std::string dts)
   {
      Poco::DateTime time;
      int timeZoneDifferential;
      try
      {
         Poco::DateTimeParser::parse(jformat, dts, time, timeZoneDifferential);
      }
      catch (const Poco::SyntaxException & )
      {
         logmsg(kLERROR, "Invalid time conversion - can't process " + dts);
      }
      return time;
   }

  
}