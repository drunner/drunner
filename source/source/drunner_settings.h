#ifndef __drunnerSettings_H
#define __drunnerSettings_H

#include "variables.h"

class drunnerSettings : public persistvariables
{
public:
   drunnerSettings(); // sets defaults, loads if able.

   static std::string getdrunnerInstallURL();
   std::string getdrunnerInstallTime()const { return getVal("INSTALLTIME"); }
   bool getPullImages() const               { return getBool("PULLIMAGES"); }

   bool mReadOkay;

private:
   static const std::vector<envDef> _getConfig();
};

#endif
