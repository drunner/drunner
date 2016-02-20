#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <ctime>

#include "drunner_settings.h"
#include "utils.h"


drunner_settings::drunner_settings(std::string rootpath)
{
   std::time_t rtime = std::time(nullptr);
   std::string timestamp = std::asctime(std::localtime(&rtime));

   // set defaults.
   mSettings["ROOTPATH"]          =rootpath;
   mSettings["SUPPORTIMAGE"]      ="drunner/install-support";
   mSettings["ROOTUTILIMAGE"]     ="drunner/install-rootutils";
   mSettings["DRUNNERINSTALLURL"] =R"EOF(https://raw.githubusercontent.com/drunner/install/master/drunner-install)EOF";
   mSettings["DRUNNERINSTALLTIME"]=timestamp;
      
   readSettings();
}


bool parse(std::string line, std::string & left, std::string & right)
{
   std::size_t end=0;
   if ((end=line.find("=",0)) != std::string::npos)
      {
         left=utils::trim_copy(line.substr(0,end));
         right=utils::trim_copy(line.substr(end+1)); //If this is equal to the string length, substr returns an empty string.
         return true;
      }
   return false;
}

bool drunner_settings::readSettings()
{
   std::string settingsfile=getRootPath()+"/config.sh";
   
   if (! utils::fileexists(settingsfile)) 
      return false;
   
   std::string line,left,right;
   std::ifstream configfile( settingsfile.c_str() );
   while (std::getline(configfile, line))
   {
      if (parse(line,left,right))
      {
         if (mSettings.find(left)!=mSettings.end()) // we only update entries, not add random stuff.
            mSettings[left]=right;
      }
   }
   
   return true;
}

bool drunner_settings::writeSettings()
{
  // iterate through array etc.
  return false;
}
