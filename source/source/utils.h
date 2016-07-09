#ifndef __UTILS_H
#define __UTILS_H

#include <vector>
#include <string>
#include <sys/stat.h>

#include <Poco/Path.h>

#include "enums.h"

typedef std::vector<std::string> tVecStr;

// Shell utilities
namespace utils
{
#ifdef _WIN32
   const std::string kCODE_S = "";
   const std::string kCODE_E = "";
#else
   const std::string kCODE_S="\e[32m";
   const std::string kCODE_E="\e[0m";
#endif

   std::string getabsolutepath(std::string path);

   eResult _makedirectories(Poco::Path path);
   void makedirectories(Poco::Path path,mode_t mode);
   void makedirectory(Poco::Path d, mode_t mode);
   void makesymlink(Poco::Path file, Poco::Path link);

   void deltree(Poco::Path s);
   void delfile(Poco::Path fullpath);

   void movetree(const std::string & src, const std::string & dst);
   bool getFolders(const std::string & parent, std::vector<std::string> & folders);

   bool fileexists(const Poco::Path& name);
   bool commandexists(std::string command);

   int runcommand(std::string command, std::vector<std::string> args);
   int runcommand(std::string command, std::vector<std::string> args, std::string &out, bool trim);
   int bashcommand(std::string bashline, std::string &op, bool trim);
   int bashcommand(std::string bashline);
   int dServiceCmd(std::string command, const std::vector<std::string> & args, bool isServiceCmd);

   std::string trim_copy(std::string s, const char* t = " \t\n\r\f\v");
   std::string& trim(std::string& s, const char* t = " \t\n\r\f\v");
   std::string doquote(std::string s);
   bool findStringIC(const std::string & strHaystack, const std::string & strNeedle);
   // case insensitive comparison.
   bool stringisame(const std::string & s1, const std::string &s2);
   std::string replacestring(std::string subject, const std::string& search, const std::string& replace);
   std::string alphanumericfilter(std::string s, bool whitespace);

   //std::string get_exepath();
   //std::string get_exefullpath();
   //std::string get_exename();

   Poco::Path get_usersbindir();   // dies if fails.

   bool imageisbranch(const std::string & imagename);
   eResult pullimage(const std::string & imagename);

   //bool isInstalled();

   std::string getHostIP();

   std::string getTime();
   std::string getPWD();
   bool dockerVolExists(const std::string & vol);

   std::string getenv(std::string envParam);

   bool copyfile(std::string src, std::string dest);

   void downloadexe(std::string url, Poco::Path filepath);

   void getAllServices(std::vector<std::string> & services);

   class tempfolder
   {
   public:
      tempfolder(Poco::Path d);
      ~tempfolder();
      Poco::Path getpath();
      
   private:
      void die(std::string msg);
      void tidy();
      Poco::Path mPath;
   };

   class dockerrun
   {
   public:
      dockerrun(const std::string & cmd, const std::vector<std::string> & args, std::string dockername);
      ~dockerrun();

   private:
      void tidy();

      std::string mDockerName;
   };

} // namespace


#endif
