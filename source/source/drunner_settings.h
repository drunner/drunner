#ifndef __drunnerSettings_H
#define __drunnerSettings_H

#include "variables.h"

class drunnerSettings : public persistvariables
{
public:
   drunnerSettings(); // sets defaults, loads if able.

   std::string getdrunnerInstallURL() const { return getVal("INSTALLURL"); }
   std::string getdrunnerInstallTime()const { return getVal("INSTALLTIME"); }
   bool getPullImages() const               { return getBool("PULLIMAGES"); }

   bool mReadOkay;

private:
   static const std::vector<envDef> _getConfig();
};

#endif
