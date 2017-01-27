#include "service_vars.h"
#include "service_paths.h"

serviceVars::serviceVars(std::string servicename, const std::vector<envDef> & config) :
   persistvariables(servicename, servicePaths(servicename).getPathServiceVars(), config)
{
   _extendconfig();
}

serviceVars::serviceVars(std::string servicename, std::string imagename, const std::vector<envDef> & config) :
   persistvariables(servicename, servicePaths(servicename).getPathServiceVars(), config)
{
   _extendconfig();
}

std::string serviceVars::getImageName() const
{
   std::string iname = getVal("IMAGENAME");
   if (iname.length() == 0) logmsg(kLWARN, "IMAGENAME not defined.");
   return iname;
}

std::string serviceVars::getServiceName() const
{
   std::string sname = getVal("SERVICENAME");
   if (sname.length() == 0) logmsg(kLWARN, "SERVICENAME not defined.");
   return sname;
}

bool serviceVars::getIsDevMode() const
{
   return getBool("DEVMODE");
}

void serviceVars::setImageName(std::string imagename)
{
   setVal("IMAGENAME", imagename);
}

void serviceVars::setDevMode(bool isDevMode)
{
   setVal("DEVMODE", isDevMode ? "True" : "False");
}


void serviceVars::_extendconfig()
{
   _addConfig(envDef("SERVICENAME", mName, "Name of Service", ENV_DEFAULTS_MEM)); // in memory, no need to persist this.
   _addConfig(envDef("IMAGENAME", "", "Image name", ENV_PERSISTS));
   _addConfig(envDef("DEVMODE", "False", "Development Mode", ENV_PERSISTS | ENV_USERSETTABLE));
}
