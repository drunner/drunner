#ifndef __UTILS_H
#define __UTILS_H

#include <vector>
#include <string>
#include "params.h"
#include "drunner_settings.h"

// Shell utilities
namespace utils
{
   const std::string kCODE_S="\e[32m";
   const std::string kCODE_E="\e[0m";

   std::string getabsolutepath(std::string path);
   std::string getcanonicalpath(std::string path);

   eResult mkdirp(std::string path);
   void makedirectory(const std::string & d, const params & p);
   void deltree(const std::string & s,const params & p);

   bool fileexists (const std::string& name);
   bool commandexists(std::string command);

   std::string getUSER();

   bool canrundocker(std::string username);
   bool isindockergroup(std::string username);

   std::string replacestring(std::string subject, const std::string& search, const std::string& replace);

   int bashcommand(std::string command, std::string & output);

   std::string trim_copy(std::string s, const char* t = " \t\n\r\f\v");
   std::string& trim(std::string& s, const char* t = " \t\n\r\f\v");

   // case insensitive comparison.
   bool stringisame(const std::string & s1, const std::string &s2 );

   std::string get_exepath();
   std::string get_exefullpath();
   std::string get_exename();
   std::string get_usersbindir();   // dies if fails.

   bool imageisbranch(const std::string & imagename);
   eResult pullimage(const std::string & imagename);

   bool getFolders(const std::string & parent, std::vector<std::string> & services);

   bool isInstalled();

   bool findStringIC(const std::string & strHaystack, const std::string & strNeedle);

   std::string getTime();
   std::string getPWD();
}

#endif
