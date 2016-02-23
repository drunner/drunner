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
   std::string timestamp = utils::trim_copy(std::asctime(std::localtime(&rtime))); // asctime puts a newline in the string :/

   // set defaults.
   mSettings["ROOTPATH"]          =rootpath;
   //mSettings["SUPPORTIMAGE"]      ="drunner/install-support";
   mSettings["ROOTUTILIMAGE"]     ="drunner/install-rootutils";
   mSettings["DRUNNERINSTALLURL"] =R"EOF(https://drunner.s3.amazonaws.com/drunner-install)EOF";
   mSettings["DRUNNERINSTALLTIME"]=timestamp;
   mSettings["PULLIMAGES"]="1";

   mRead=readSettings();
}

bool drunner_settings::readFromFileOkay()
{
   return mRead;
}

bool parse(std::string line, std::string & left, std::string & right)
{
   std::size_t end=0;
   if ((end=line.find("=",0)) != std::string::npos)
      {
         left=utils::trim_copy(line.substr(0,end));
         right=utils::trim_copy(line.substr(end+1)); //If this is equal to the string length, substr returns an empty string.
         if (left.length()==0) return false;    // empty key
         if (left[0]=='#') return false;        // comment line
         return true;
      }
   return false;
}

bool drunner_settings::readSettings()
{
   std::string settingsfile=getPath_Root()+"/"+settingsFileName;

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
   std::string settingsfile=getPath_Root()+"/"+settingsFileName;
   std::ofstream ofile;
   ofile.open(settingsfile);
   if (!ofile.is_open()) return false; // can't open the file.

   ofile << "# Docker Runner configuration file." << std::endl;
   ofile << "# You can edit this file - user changes are preserved on update." << std::endl << std::endl;

  // iterate through map. C++11 style.
  for (auto const &entry : mSettings) {
     ofile << entry.first << "=" << entry.second << std::endl;
  }
  ofile.close();
  return true;
}
