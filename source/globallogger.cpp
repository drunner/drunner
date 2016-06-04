#include <iostream>
#include <ctime>
#include <sstream>
#include <fstream>
#include <boost/algorithm/string.hpp>

#include "enums.h"
#include "params.h"
#include "globallogger.h"
#include "exceptions.h"
#include "utils.h"
#include "termcolor.h"

using namespace std;

std::shared_ptr<GlobalLogger> GlobalLogger::instance()
{
   if (!s_instance)
      fatal("Attempt to get instance of logger when not initialised!");
   return s_instance;
}
void GlobalLogger::init(const params & p)
{
   s_instance = std::make_shared<GlobalLogger>(p.getLogLevel,true);
}
void GlobalLogger::init()
{
   s_instance = std::make_shared<GlobalLogger>(kLFATAL, false);
}
GlobalLogger::GlobalLogger(eLogLevel logcutoff, bool loggingenabled) : mLogCutoff(logcutoff), mLoggingEnabled(loggingenabled)
{
}

std::string GlobalLogger::timestamp()
{
        //Notice the use of a stringstream, yet another useful stream medium!
        ostringstream stream;
        time_t rawtime;
        tm * timeinfo;

        time(&rawtime);
        timeinfo = localtime( &rawtime );

        stream << (timeinfo->tm_year)+1900<<"/"<<timeinfo->tm_mon
        <<"/"<<timeinfo->tm_mday<<" "<<timeinfo->tm_hour
        <<":"<<timeinfo->tm_min;
        // The str() function of output stringstreams return a std::string.
        return stream.str();
}

std::string GlobalLogger::levelname(eLogLevel level)
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

void GlobalLogger::logverbatim(eLogLevel level, std::string s)
{
   if (level < mLogCutoff)
      return;

   // we use stdout for normal messages, stderr for warn and error.
   switch (level)
   {
      case kLDEBUG: std::cout << termcolor::cyan << s << termcolor::reset; break;
      case kLINFO:  std::cout << termcolor::blue << s << termcolor::reset; break;
      case kLWARN:  std::cerr << termcolor::yellow <<  s << termcolor::reset; break;
      case kLERROR: std::cerr << termcolor::red << s << termcolor::reset; break;
      case kLFATAL: 
         std::cerr << termcolor::red << s << termcolor::reset; 
         throw eExit(s.c_str());
         break;
      default:      std::cerr << termcolor::green <<  s << termcolor::reset; break;
   }
}

std::string GlobalLogger::getheader(eLogLevel level)
{
   std::ostringstream ost;
   ost << "|" << levelname(level) << "|" << timestamp() << "| ";
   return ost.str();
}


void GlobalLogger::logmsg(eLogLevel level, std::string s)
{   
   if (level < mLogCutoff)
      return;

   std::string info = getheader(level);
   std::string s2=utils::replacestring(s,"\n","\n"+info);
   boost::erase_all(s2, "\r");

   logverbatim(level,info+s2+"\n");
}

// ----------------------------------------------------------------------------------------------------

