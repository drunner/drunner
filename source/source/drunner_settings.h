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


   std::string getdrunnerInstallURL() const { return getString("DRUNNERINSTALLURL"); }
   std::string getdrunnerInstallTime()const { return getString("DRUNNERINSTALLTIME"); }
   bool getPullImages() const               { return getBool("PULLIMAGES"); }


   bool readSettings();
   bool writeSettings() const;

   mutable bool mReadOkay;
};

#endif
