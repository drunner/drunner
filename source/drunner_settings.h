#include <map>

#include "settingsbash.h"

#ifndef __DRUNNER_SETTINGS_H
#define __DRUNNER_SETTINGS_H


class drunner_settings : public settingsbash
{
public:
   drunner_settings(std::string rootpath); // sets defaults, reads from config.sh if present.

   bool readFromFileOkay()            const { return mRead; }

   std::string getPath_Root()         const { return getSetting("ROOTPATH");    }
   std::string getPath_Services()     const { return getPath_Root()+"/services"; }
   std::string getPath_Support()      const { return getPath_Root()+"/support"; }

   //std::string getSupportImage()       {   return mSettings["SUPPORTIMAGE"];        }
   std::string getRootUtilImage()     const { return getSetting("ROOTUTILIMAGE");       }
   std::string getdrunnerInstallURL() const { return getSetting("DRUNNERINSTALLURL");   }
   std::string getdrunnerInstallTime()const { return getSetting("DRUNNERINSTALLTIME");  }
   bool getPullImages() const               { return getSettingb("PULLIMAGES"); }

private:
   bool mRead;
};

#endif
