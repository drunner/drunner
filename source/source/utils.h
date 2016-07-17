#ifndef __UTILS_H
#define __UTILS_H

#include <vector>
#include <string>
#include <sys/stat.h>

#include <Poco/Path.h>
#include <Poco/Process.h>

#include "enums.h"
#include "cresult.h"

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

   constexpr unsigned int str2int(const char* str, int h = 0)
   {
      return !str[h] ? 5381 : (str2int(str, h + 1) * 33) ^ str[h];
   }

   std::string getabsolutepath(std::string path);

   eResult _makedirectories(Poco::Path path);
   void makedirectories(Poco::Path path,mode_t mode);
   void makedirectory(Poco::Path d, mode_t mode);
//   void makesymlink(Poco::Path file, Poco::Path link);

   cResult deltree(Poco::Path s);
   cResult delfile(Poco::Path fullpath);

   void movetree(const std::string & src, const std::string & dst);
   bool getFolders(const std::string & parent, std::vector<std::string> & folders);

   bool fileexists(const Poco::Path& name);
   bool commandexists(std::string command);

   int runcommand(std::string command, std::vector<std::string> args);
   int runcommand(std::string command, std::vector<std::string> args, std::string &out, bool trim);
   int runcommand_stream(std::string command, const std::vector<std::string> & args, bool isServiceCmd, Poco::Path initialDirectory, const Poco::Process::Env & env);

   std::string trim_copy(std::string s, const char* t = " \t\n\r\f\v");
   std::string& trim(std::string& s, const char* t = " \t\n\r\f\v");
   std::string doquote(std::string s);
   bool findStringIC(const std::string & strHaystack, const std::string & strNeedle);
   // case insensitive comparison.
   bool stringisame(const std::string & s1, const std::string &s2);
   std::string replacestring(std::string subject, const std::string& search, const std::string& replace);
   std::string alphanumericfilter(std::string s, bool whitespace);
   bool wordmatch(std::string s, std::string word);

   bool imageisbranch(const std::string & imagename);
   eResult pullimage(const std::string & imagename);

   std::string getTime();
   std::string getPWD();

   std::string getenv(std::string envParam);

   // bool copyfile(std::string src, std::string dest);

   void getAllServices(std::vector<std::string> & services);

   bool split_in_args(std::string command, std::vector<std::string>& qargs);

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

   //class dockerrun
   //{
   //public:
   //   dockerrun(const std::string & cmd, const std::vector<std::string> & args, std::string dockername);
   //   ~dockerrun();

   //private:
   //   void tidy();

   //   std::string mDockerName;
   //};

} // namespace


#endif
