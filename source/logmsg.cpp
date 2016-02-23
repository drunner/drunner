#include <iostream>
#include <ctime>
#include <sstream>
#include <fstream>

#include "enums.h"
#include "params.h"
#include "logmsg.h"
#include "exceptions.h"
#include "utils.h"

using namespace std;

std::string timestamp()
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

std::string getcolor(eLogLevel level)
{
   switch (level)
   {
      case kLDEBUG: return "\e[90m";
      case kLINFO:  return "\e[34m";
      case kLWARN:  return "\e[33m";
      case kLERROR: return "\e[31m";
      default: return "\e[32m";
   }
}

void logverbatim(eLogLevel level, std::string s, eLogLevel cutoff)
{
   if (level<cutoff)
      return;

   // we use stdout for normal messages, stderr for warn and error.
   if (level<=kLINFO)
      std::cout << "\e[0m" << getcolor(level) << s << "\e[0m";
   else
      std::cerr << "\e[0m" << getcolor(level) << s << "\e[0m";

   if (level==kLERROR)
      throw eExit(s.c_str());
}

void logmsg(eLogLevel level, std::string s, eLogLevel cutoff)
{
   std::ostringstream ost;
   ost<<"|"<<levelname(level)<<"|"<<timestamp()<<"| ";
   std::string info=ost.str();
   std::string s2=utils::replacestring(s,"\n","\n"+info);

   ost << s2 <<std::endl;
   logverbatim(level,ost.str(),cutoff);
}
