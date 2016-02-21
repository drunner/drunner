#include <iostream>
#include <ctime>
#include <sstream>
#include <fstream>

#include "enums.h"
#include "params.h"
#include "logmsg.h"
#include "exceptions.h"
 
 using namespace std;

// ostream& operator<<(ostream& ost, const LogStatement& ls)
// {
//         ost<<"~|"<<ls.mTime<<'|'<<ls.mData<<"|~";
//         return ost;
// }
 
std::string timestamp() 
{
        //Notice the use of a stringstream, yet another useful stream medium!
        ostringstream stream;    
        time_t rawtime;
        tm * timeinfo;
 
        time(&rawtime);
        timeinfo = localtime( &rawtime );
 
        stream << (timeinfo->tm_year)+1900<<" "<<timeinfo->tm_mon
        <<" "<<timeinfo->tm_mday<<" "<<timeinfo->tm_hour
        <<" "<<timeinfo->tm_min<<" "<<timeinfo->tm_sec;
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
   if (level<=kLINFO)
      std::cout << s;
   else
      std::cerr << s;

   if (level==kLERROR)
      throw eExit(s.c_str());
}

void logmsg(eLogLevel level, std::string s, eLogLevel cutoff)
{
   std::ostringstream ost;
   ost<<"|"<<levelname(level)<<"|"<<timestamp()<<"| "<<s<<std::endl;

   logverbatim(level,ost.str(),cutoff);
}

void logmsg(eLogLevel level, std::string s, const params::params & p)
{
   logmsg(level,s,p.getLogLevel());
}

void logverbatim(eLogLevel level, std::string s, const params::params & p)
{
   logverbatim(level,s,p.getLogLevel());
}
