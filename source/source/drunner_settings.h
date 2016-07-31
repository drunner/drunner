#ifndef __drunnerSettings_H
#define __drunnerSettings_H

#include <map>
#include <Poco/Path.h>

#include "params.h"
#include "variables.h"
#include "cresult.h"

class drunnerSettings : public persistvariables
{
public:
   drunnerSettings(); // sets defaults, loads if able.

   std::string getdrunnerInstallURL() const { return mVariables.getVal("INSTALLURL"); }
   std::string getdrunnerInstallTime()const { return mVariables.getVal("INSTALLTIME"); }
   bool getPullImages() const               { return mVariables.getBool("PULLIMAGES"); }

   bool mReadOkay;

private:
   variables mVariables;
};
CEREAL_CLASS_VERSION(drunnerSettings, 1);

#endif
