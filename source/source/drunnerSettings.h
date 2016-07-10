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

   static Poco::Path getPath_Bin()          { return getPath_Root().pushDirectory("bin"); } // bin subfolder 
   static Poco::Path getPath_dServices()    { return getPath_Root().pushDirectory("dServices"); }
   static Poco::Path getPath_Support()      { return getPath_Root().pushDirectory("support"); }
   static Poco::Path getPath_Temp()         { return getPath_Root().pushDirectory("temp"); }
   static Poco::Path getPath_HostVolumes()  { return getPath_Root().pushDirectory("hostVolumes"); }

   std::string getdrunnerUtilsImage()     const { return getString("DRUNNERUTILSIMAGE"); }
   std::string getdrunnerInstallURL() const { return getString("DRUNNERINSTALLURL"); }
   std::string getdrunnerInstallTime()const { return getString("DRUNNERINSTALLTIME"); }
   bool getPullImages() const               { return getBool("PULLIMAGES"); }

   static Poco::Path getPath_drunnerSettings_sh() { return getPath_Root().setFileName("drunnerSettings.sh"); }
   static Poco::Path getPath_drunnerbackups_cfg() { return getPath_Root().setFileName("drunnerBackups.cfg"); }

   bool readSettings();
   bool writeSettings() const;

   mutable bool mReadOkay;
};

#endif
