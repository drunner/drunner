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

class CommandLine
{
public:
   CommandLine() {}
   CommandLine(std::string c) : command(c) {}
   CommandLine(std::string c, const std::vector<std::string> & a) : command(c), args(a) { }
   void logcommand(std::string prefix, eLogLevel ll=kLDEBUG) const;
   std::string command;
   std::vector<std::string> args;
};

// A very simple hasing of strings, useful for switch statements. Case sensitive.
constexpr unsigned int s2i(const char* str, int h = 0)
{
   return !str[h] ? 5381 : (s2i(str, h + 1) * 33) ^ str[h];
}

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

   cResult _makedirectories(Poco::Path path);
   cResult makedirectories(Poco::Path path,mode_t mode);
   cResult makedirectory(Poco::Path d, mode_t mode);
//   void makesymlink(Poco::Path file, Poco::Path link);

   cResult deltree(Poco::Path s);
   cResult delfile(Poco::Path fullpath);

   void movetree(const std::string & src, const std::string & dst);
   bool getFolders(const std::string & parent, std::vector<std::string> & folders);

   bool fileexists(const Poco::Path& name);
   bool commandexists(std::string command);
   
   int runcommand(const CommandLine & operation, std::string &out);
   int runcommand_stream(const CommandLine & operation, edServiceOutput outputMode, Poco::Path initialDirectory = "", const Poco::Process::Env & env = {});

   bool findStringIC(const std::string & strHaystack, const std::string & strNeedle);
   std::string replacestring(std::string subject, const std::string& search, const std::string& replace);
   std::string alphanumericfilter(std::string s, bool whitespace);
   bool wordmatch(std::string s, std::string word);

   bool imageislocal(const std::string & imagename);
   cResult pullimage(const std::string & imagename);

   std::string getTime();
   std::string getPWD();

   std::string getenv(std::string envParam);

   void getAllServices(std::vector<std::string> & services);

   cResult split_in_args(std::string command, CommandLine & cl);

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

} // namespace


#endif
