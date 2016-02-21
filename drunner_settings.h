#include <map>

#ifndef __DRUNNER_SETTINGS_H
#define __DRUNNER_SETTINGS_H

class drunner_settings
{
public:
   drunner_settings(std::string rootpath); // sets defaults, reads from config.sh if present.
   
   bool writeSettings();
   bool readFromFileOkay();
   
   std::string getRootPath()           {   return mSettings["ROOTPATH"];            }
   //std::string getSupportImage()       {   return mSettings["SUPPORTIMAGE"];        }
   std::string getRootUtilImage()      {   return mSettings["ROOTUTILIMAGE"];       }
   std::string getdrunnerInstallURL()  {   return mSettings["DRUNNERINSTALLURL"];   }
   std::string getdrunnerInstallTime() {   return mSettings["DRUNNERINSTALLTIME"];  }
   
   static std::string getSettingsFileName()   {   return settingsFileName;                 }
   
private:   
   bool readSettings();
   std::map< std::string, std::string > mSettings;
   bool mRead;
   static const char * settingsFileName;
};

#endif
