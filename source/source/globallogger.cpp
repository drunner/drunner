#include <iostream>
#include <ctime>
#include <sstream>
#include <fstream>
#include <algorithm>

#include "enums.h"
#include "params.h"
#include "globallogger.h"
#include "globalcontext.h"
#include "exceptions.h"
#include "utils.h"
#include "termcolor.h"
#include "timez.h"

eLogLevel getMinLevel()
{
   static bool hasWarned = false;

   if (!GlobalContext::hasParams())
   {
      if (!hasWarned)
      {
         hasWarned = true;
         logmsg(kLWARN, "Logging not yet initialised, but being asked to log.");
      }
      return kLINFO;
   }
   return GlobalContext::getParams()->getLogLevel();
}

std::string levelname(eLogLevel level)
{
   switch (level)
   {
      case kLDEBUG: return "DEBUG";
      case kLINFO:  return "INFO ";
      case kLWARN:  return "WARN ";
      case kLERROR: return "ERROR";
      default: return " ??? ";
   }
}

void logverbatim(eLogLevel level, std::string s)
{
   if (level < getMinLevel())
      return;

   // we use stdout for normal messages, stderr for warn and error.
   switch (level)
   {
      case kLDEBUG: std::cout << termcolor::cyan << s << termcolor::reset; break;
      case kLINFO:  std::cout << termcolor::blue << s << termcolor::reset; break;
      case kLWARN:  std::cerr << termcolor::yellow <<  s << termcolor::reset; break;
      case kLERROR: 
         std::cerr << termcolor::red << s << termcolor::reset; 
         throw eExit();
         break;
      default:      std::cerr << termcolor::green <<  s << termcolor::reset; break;
   }
}

std::string getheader(eLogLevel level)
{
   std::ostringstream ost;
   ost << "|" << levelname(level) << "|" << timeutils::getLogTimeStamp() << "| ";
   return ost.str();
}


void logmsg(eLogLevel level, std::string s)
{   
   if (level < getMinLevel())
      return;

   std::string info = getheader(level);
   std::string s2=utils::replacestring(s,"\n","\n"+info);
   //boost::erase_all(s2, "\r");
   s2.erase(std::remove(s2.begin(), s2.end(), '\r'), s2.end());

   logverbatim(level,info+s2+"\n");
}


void fatal(std::string s)
{
   logmsg(kLERROR, s);
}

// ----------------------------------------------------------------------------------------------------

