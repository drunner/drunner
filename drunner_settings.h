#include <map>


#ifndef __DRUNNER_SETTINGS_H
#define __DRUNNER_SETTINGS_H

class drunner_settings
{
public:
   drunner_settings(std::string rootpath);

   bool readSettings();
   bool writeSettings();
   
   std::string getRootPath()           {   return mSettings["ROOTPATH"];            }
   std::string getSupportImage()       {   return mSettings["SUPPORTIMAGE"];        }
   std::string getRootUtilImage()      {   return mSettings["ROOTUTILIMAGE"];       }
   std::string getdrunnerInstallURL()  {   return mSettings["DRUNNERINSTALLURL"];   }
   std::string getdrunnerInstallTime() {   return mSettings["DRUNNERINSTALLTIME"];  }
   
private:   
   std::map< std::string, std::string > mSettings;
};

#endif
