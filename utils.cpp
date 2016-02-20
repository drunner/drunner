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
#include <stdlib.h>
#include <boost/filesystem.hpp>
#include <grp.h>
#include <pwd.h>

#include "pstream.h"
#include "utils.h"

namespace utils
{
   // trim from left
   inline std::string& ltrim(std::string& s, const char* t = " \t\n\r\f\v")
   {
       s.erase(0, s.find_first_not_of(t));
       return s;
   }

   // trim from right
   inline std::string& rtrim(std::string& s, const char* t = " \t\n\r\f\v")
   {
       s.erase(s.find_last_not_of(t) + 1);
       return s;
   }

   // trim from left & right
   std::string& trim(std::string& s, const char* t)
   {
       return ltrim(rtrim(s, t), t);
   }

   // copying versions

   inline std::string ltrim_copy(std::string s, const char* t = " \t\n\r\f\v")
   {
       return ltrim(s, t);
   }

   inline std::string rtrim_copy(std::string s, const char* t = " \t\n\r\f\v")
   {
       return rtrim(s, t);
   }

   std::string trim_copy(std::string s, const char* t)
   {
       return utils::trim(s, t);
   }

   // std::string getVersion()    {
   //    return "0.1 dev";
   // }
   int bashcommand(std::string command, std::string & output)
   {
      redi::ipstream in(command);
      std::string str;
      while (in >> str)
         output+=trim_copy(str)+" ";
      in.close();
      utils::trim(output);
      return in.rdbuf()->status();
   }

   std::string bashcommand(const params & p, std::string c)
   {
      std::string op;
      int rval = bashcommand(c,op);
      if (rval!=0)
         die(p,"Running command "+c+" failed.",rval);
      return op;
   }

   std::string getabsolutepath(std::string path)
   {
      boost::filesystem::path rval;
      try
      {
         rval = boost::filesystem::absolute(path);
      }
      catch(...)
      {
         return "";
      }
      return rval.string();
   }

   bool mkdirp(std::string path)
   {
      try
      {
         boost::filesystem::create_directories(path);
      }
      catch (...)
      {
         return false;
      }
      return true;
   }


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

   void die( std::string msg, int exit_code )
   {
      if (msg.length()>0)
         std::cerr << std::endl << "\e[31m" << msg << "\e[0m" << std::endl << std::endl;
      exit(exit_code);
   }

   void die( const params & p, std::string msg, int exit_code )
   {
      die( p.mOMode==om_silent ? "" : msg , exit_code );
   }

   bool isindockergroup(std::string username)
   {
      std::string op;
      int rval = bashcommand("groups $USER | grep docker",op);
      return (rval==0);
   }

   bool canrundocker(std::string username)
   {
      std::string op;
      int rval = bashcommand("groups | grep docker",op);
      return (rval==0);
   }

   std::string getUSER()
   {
      std::string op;
      int rval = bashcommand("echo $USER",op);
      if (rval!=0)
         die("Couldn't get current user.");
      return op;
   }

   bool commandexists(std::string command)
   {
      std::string op;
      int rval = bashcommand("command -v "+command,op);
      return (rval==0);
   }



} // namespace utils
