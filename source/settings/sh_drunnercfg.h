#ifndef __sh_drunnercfg_H
#define __sh_drunnercfg_H

#include <map>

#include "settingsbash.h"
#include "params.h"

class sh_drunnercfg : public settingsbash_reader
{
public:
   sh_drunnercfg(const std::string & rootpath); // sets defaults and reads the file if present.

   bool write();

   std::string getPath_Root()         const { return getString("ROOTPATH"); }
   std::string getPath_Services()     const { return getPath_Root() + "/services"; }
   std::string getPath_Support()      const { return getPath_Root() + "/support"; }
   std::string getPath_Temp()         const { return getPath_Root() + "/temp"; }

   std::string getRootUtilImage()     const { return getString("ROOTUTILIMAGE");       }
   std::string getdrunnerInstallURL() const { return getString("DRUNNERINSTALLURL");   }
   std::string getdrunnerInstallTime()const { return getString("DRUNNERINSTALLTIME");  }
   bool getPullImages() const               { return getBool("PULLIMAGES"); }

protected:
   void setDefaults(const std::string & rootpath);
};

#endif
