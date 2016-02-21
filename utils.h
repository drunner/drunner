#include <vector>
#include <string>
#include "params.h"

#ifndef __UTILS_H
#define __UTILS_H

enum eResult 
{
   kSuccess=0,
   kError=1,
   kNoChange=3
};


// Shell utilities
namespace utils
{
   const std::string kCODE_S="\e[32m";
   const std::string kCODE_E="\e[0m";

   std::string getabsolutepath(std::string path);
   std::string getcanonicalpath(std::string path);

   eResult mkdirp(std::string path);

   bool fileexists (const std::string& name);
   bool commandexists(std::string command);

   std::string getUSER();

   bool canrundocker(std::string username);
   bool isindockergroup(std::string username);

   std::string replacestring(std::string subject, const std::string& search, const std::string& replace);

   int bashcommand(std::string command, std::string & output);

   std::string trim_copy(std::string s, const char* t = " \t\n\r\f\v");
   std::string& trim(std::string& s, const char* t = " \t\n\r\f\v");

   void die( const params::params & p, std::string msg, int exit_code=1 );
   void die( std::string msg, int exit_code=1 );
 
   // case insensitive comparison.
   bool stringisame(const std::string & s1, const std::string &s2 );  

   std::string get_rootpath();
   std::string get_exefullpath();
   std::string get_usersbindir();   // dies if fails.
   
   bool imageisbranch(std::string imagename);
   eResult pullimage(std::string imagename);
}

#endif
