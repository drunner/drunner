#include <map>

#ifndef __DRUNNER_SETTINGS_H
#define __DRUNNER_SETTINGS_H

class drunner_settings
{
public:
   drunner_settings(std::string rootpath); // sets defaults, reads from config.sh if present.
   
   bool writeSettings();
   bool readFromFileOkay();
   
   std::string getPath_Root()         const {   return mSettings.at("ROOTPATH");    }
   std::string getPath_Services()     const {   return getPath_Root()+"/services"; }

   //std::string getSupportImage()       {   return mSettings["SUPPORTIMAGE"];        }
   std::string getRootUtilImage()     const {   return mSettings.at("ROOTUTILIMAGE");       }
   std::string getdrunnerInstallURL() const {   return mSettings.at("DRUNNERINSTALLURL");   }
   std::string getdrunnerInstallTime()const {   return mSettings.at("DRUNNERINSTALLTIME");  }
   
   static std::string getSettingsFileName()   {   return settingsFileName;                 }
   
private:   
   bool readSettings();
   std::map< std::string, std::string > mSettings;
   bool mRead;
   static const char * settingsFileName;
};

#endif
