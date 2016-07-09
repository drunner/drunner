#ifndef __drunnerSettings_H
#define __drunnerSettings_H

#include <map>
#include <Poco/Path.h>

#include "settingsbash.h"
#include "params.h"

class drunnerSettings : protected settingsbash
{
public:
   drunnerSettings(); // sets defaults, loads if able.

   static Poco::Path getPath_Root(); // the path of the drunner root (for all config). %APPDATA%/drunner [win] or $HOME/.drunner [lin]
   static Poco::Path getPath_Exe();  // the path of the drunner execuatble. can be anywhere

   Poco::Path getPath_dServices()    const { return getPath_Root().pushDirectory("dServices"); }
   Poco::Path getPath_Support()      const { return getPath_Root().pushDirectory("support"); }
   Poco::Path getPath_Temp()         const { return getPath_Root().pushDirectory("temp"); }
   Poco::Path getPath_HostVolumes()  const { return getPath_Root().pushDirectory("hostVolumes"); }

   std::string getRootUtilImage()     const { return getString("ROOTUTILIMAGE"); }
   std::string getdrunnerInstallURL() const { return getString("DRUNNERINSTALLURL"); }
   std::string getdrunnerInstallTime()const { return getString("DRUNNERINSTALLTIME"); }
   bool getPullImages() const { return getBool("PULLIMAGES"); }

   Poco::Path getPath_drunnercfg_sh() const { return getPath_Root().setFileName("drunnercfg.sh"); }
   Poco::Path getPath_drunnerbackups_cfg() const { return getPath_Root().setFileName("drunnerbackups.cfg"); }

   bool readSettings();
   bool writeSettings() const;

   mutable bool mReadOkay;
};

#endif
