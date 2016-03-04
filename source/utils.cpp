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
#include <boost/locale.hpp>
#include <boost/algorithm/string.hpp>

#include <sys/stat.h>
#include <unistd.h>

#include "pstream.h"
#include "utils.h"
#include "exceptions.h"
#include "logmsg.h"

namespace utils
{

   bool fileexists (const std::string& name)
   {
      struct stat buffer;
      return (stat (name.c_str(), &buffer) == 0);
   }

   bool stringisame(const std::string & s1, const std::string &s2 )
   {
      return boost::iequals(s1,s2);
   }

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

   std::string getcanonicalpath(std::string path)
   {
      boost::filesystem::path rval;
      try
      {
         rval = boost::filesystem::canonical(path);
      }
      catch(...)
      {
         return "";
      }
      return rval.string();
   }


   eResult mkdirp(std::string path)
   {
      if (fileexists(path))
         return kRNoChange;
      try
      {
         boost::filesystem::create_directories(path);
      }
      catch (...)
      {
         return kRError;
      }
      return kRSuccess;
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
         logmsg(kLERROR,"Couldn't get current user.");
      return op;
   }

   bool commandexists(std::string command)
   {
      std::string op;
      int rval = bashcommand("command -v "+command,op);
      return (rval==0);
   }

   std::string get_exefullpath()
   {
      char buff[PATH_MAX];
      ssize_t len = ::readlink("/proc/self/exe", buff, sizeof(buff)-1);
      if (len != -1)
      {
         buff[len] = '\0';
         return std::string(buff);
      }
      logmsg(kLERROR,"Couldn't get path to drunner executable!");
      return "";
   }

   std::string get_exename()
   {
      boost::filesystem::path p( get_exefullpath() );
      return p.filename().string();
   }

   std::string get_exepath()
   {
      boost::filesystem::path p( get_exefullpath() );
      return p.parent_path().string();
   }

   std::string get_usersbindir()
   {
      std::string op;
      int rval = bashcommand("echo $HOME",op);
      if (rval!=0)
         logmsg(kLERROR,"Couldn't get current user's home directory.");
      return op+"/bin";
   }

   bool imageisbranch(const std::string & imagename)
   {
      std::size_t end=0;
      if ((end=imagename.find(":",0)) == std::string::npos)
         return false;
      std::string branchname=imagename.substr(end+1);
      if (stringisame(branchname,"master"))
         return false;

      return true;
   }

   eResult pullimage(const std::string & imagename)
   {
      if (imageisbranch(imagename))
         return kRNoChange;
      std::string op;

      int rval = bashcommand("docker pull "+imagename, op);

      if (rval==0 && op.find("Image is up to date",0) != std::string::npos)
         return kRNoChange;

      return (rval==0) ? kRSuccess : kRError;
   }



   bool getFolders(const std::string & parent, std::vector<std::string> & services)
   {
      boost::filesystem::path dir_path(parent);
      if ( ! boost::filesystem::exists( dir_path ) ) return false;

      boost::filesystem::directory_iterator itr(dir_path),end_itr; // default construction yields past-the-end
      for ( ; itr != end_itr; ++itr )
      {
         if ( boost::filesystem::is_directory(itr->status()) )
            services.push_back(itr->path().filename().string());
      }
      return true;
   }

   // quick crude check to see if we're installed.
   bool isInstalled()
   {
      std::string rootpath = get_exepath();
      return (boost::filesystem::exists(rootpath + "/" + "drunnercfg.sh"));
   }


   /// Try to find in the Haystack the Needle - ignore case
   bool findStringIC(const std::string & strHaystack, const std::string & strNeedle)
   {
     auto it = std::search(
       strHaystack.begin(), strHaystack.end(),
       strNeedle.begin(),   strNeedle.end(),
       [](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); }
     );
     return (it != strHaystack.end() );
   }


   void makedirectory(const std::string & d, const params & p)
   {
      eResult rslt = utils::mkdirp(d);
      if (rslt==kRError)
         logmsg(kLERROR,"Couldn't create "+d,p);
      if (rslt==kRSuccess)
         logmsg(kLDEBUG,"Created "+d,p);
      if (rslt==kRNoChange)
         logmsg(kLDEBUG,d+" exists. Unchanged.",p);
   }

   void deltree(const std::string & s,const params & p)
   {
      std::string op;
      if (!fileexists(s))
      {
         logmsg(kLDEBUG,"Directory "+s+" does not exist (no need to delete).");
      }
      else
      {
         if (bashcommand("rm -rf "+s, op) != 0)
            logmsg(kLERROR, "Unable to remove existing support directory at "+s,p);
         logmsg(kLDEBUG,"Recursively deleted "+s,p);
      }
   }

   std::string getHostIP(const params & p )
   {
   std::string hostIP;
   if (utils::bashcommand("ip route get 1 | awk '{print $NF;exit}'",hostIP) !=0)
      logmsg(kLERROR,"Couldn't get host IP.",p);
   logmsg(kLDEBUG,"Using "+hostIP+" for hostIP.");
   return hostIP;
   }

   std::string getTime()
   {
      std::time_t rtime = std::time(nullptr);
      return utils::trim_copy(std::asctime(std::localtime(&rtime)));
   }

   std::string getPWD()
   {
      char p[300];
      getcwd(p,300);
      return std::string(p);
   }


} // namespace utils
