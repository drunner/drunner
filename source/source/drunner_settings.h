#ifndef __drunnerSettings_H
#define __drunnerSettings_H

#include <map>
#include <Poco/Path.h>

#include "params.h"
#include "variables.h"
#include "cresult.h"

class drunnerSettings
{
public:
   drunnerSettings(); // sets defaults, loads if able.

   std::string getdrunnerInstallURL() const { return mVariables.getVal("DRUNNERINSTALLURL"); }
   std::string getdrunnerInstallTime()const { return mVariables.getVal("DRUNNERINSTALLTIME"); }
   bool getPullImages() const               { return mVariables.getBool("PULLIMAGES"); }

   cResult readSettings();
   cResult writeSettings() const;

   bool mReadOkay;

private:
   variables mVariables;
};
CEREAL_CLASS_VERSION(drunnerSettings, 1);

#endif
