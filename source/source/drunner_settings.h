#ifndef __drunnerSettings_H
#define __drunnerSettings_H

#include "variables.h"

class drunnerSettings : public persistvariables
{
public:
   drunnerSettings(); // sets defaults, loads if able.

   std::string getdrunnerInstallURL() const { return mVariables.getVal("INSTALLURL"); }
   std::string getdrunnerInstallTime()const { return mVariables.getVal("INSTALLTIME"); }
   bool getPullImages() const               { return mVariables.getBool("PULLIMAGES"); }

   bool mReadOkay;
};

#endif
