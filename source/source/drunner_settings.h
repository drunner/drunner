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
   std::string getProxy() const { return getVal("PROXY"); }

   bool mReadOkay;

private:
   static const std::vector<envDef> _getConfig();
};

#endif
