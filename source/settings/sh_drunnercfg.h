#include <map>

#ifndef __sh_drunnercfg_H
#define __sh_drunnercfg_H

#include "settingsbash.h"
#include "params.h"

class sh_drunnercfg : public settingsbash
{
public:
   sh_drunnercfg(const params & p, std::string rootpath);

   bool readFromFileOkay()            const { return mRead; }

   std::string getPath_Root()         const { return getSetting("ROOTPATH");    }
   std::string getPath_Services()     const { return getPath_Root()+"/services"; }
   std::string getPath_Support()      const { return getPath_Root()+"/support"; }
   std::string getPath_TempServices() const { return getPath_Root()+"/temp/services"; }

   //std::string getSupportImage()       {   return mSettings["SUPPORTIMAGE"];        }
   std::string getRootUtilImage()     const { return getSetting("ROOTUTILIMAGE");       }
   std::string getdrunnerInstallURL() const { return getSetting("DRUNNERINSTALLURL");   }
   std::string getdrunnerInstallTime()const { return getSetting("DRUNNERINSTALLTIME");  }
   bool getPullImages() const               { return getSettingb("PULLIMAGES"); }

private:
   bool mRead;
};

#endif
