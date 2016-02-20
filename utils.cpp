#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
//#include <fcntl.h>
#include <errno.h>

#include "pstream.h"
#include "utils.h"

namespace utils
{
   // std::string getVersion()    {
   //    return "0.1 dev";
   // }

   std::string replacestring(std::string subject, const std::string& search,
        const std::string& replace)
   {
      size_t pos = 0;
      if (search.empty() || subject.empty())
         return "";
      while((pos = subject.find(search, pos)) != std::string::npos)
      {
         subject.replace(pos, search.length(), replace);
         pos += replace.length();
      }
      return subject;
   }

   void dielow( std::string msg, int exit_code )
   {
      if (msg.length()>0)
         std::cerr << std::endl << "\e[31m" << msg << "\e[0m" << std::endl << std::endl;
      exit(exit_code);
   }

   void die( const params & p, std::string msg, int exit_code )
   {
      dielow( p.mOMode==om_silent ? "" : msg , exit_code );
   }


} // namespace utils
