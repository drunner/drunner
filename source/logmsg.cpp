#include <iostream>
#include <ctime>
#include <sstream>
#include <fstream>
#include <boost/algorithm/string.hpp>

#include "enums.h"
#include "params.h"
#include "logmsg.h"
#include "exceptions.h"
#include "utils.h"
#include "termcolor.h"

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

void logverbatim(eLogLevel level, std::string s, eLogLevel cutoff)
{
   if (level<cutoff)
      return;

   // we use stdout for normal messages, stderr for warn and error.
   switch (level)
   {
      case kLDEBUG: std::cout << termcolor::cyan << s << termcolor::reset; break;
      case kLINFO:  std::cout << termcolor::blue << s << termcolor::reset; break;
      case kLWARN:  std::cerr << termcolor::yellow <<  s << termcolor::reset; break;
      case kLERROR:
         std::cerr << termcolor::red <<  s << termcolor::reset;
         throw eExit(s.c_str());
         break;
      default:      std::cerr << termcolor::green <<  s << termcolor::reset; break;
   }
}

void logmsg(eLogLevel level, std::string s, eLogLevel cutoff)
{
   std::ostringstream ost;
   ost<<"|"<<levelname(level)<<"|"<<timestamp()<<"| ";
   std::string info=ost.str();
   std::string s2=utils::replacestring(s,"\n","\n"+info);
   boost::erase_all(s2, "\r");

   ost << s2 <<std::endl;
   logverbatim(level,ost.str(),cutoff);
}