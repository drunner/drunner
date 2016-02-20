#include <string>

#include "drunner_settings.h"
#include "utils.h"


drunner_settings::drunner_settings(std::string rootpath)
{
   mRootPath=rootpath;
   readSettings();
}

bool drunner_settings::readSettings()
{
   std::string settingsfile=mRootPath+"/config.txt";
   
   if (! utils::fileexists(settingsfile)) 
      return false;
   
   return true;
}

