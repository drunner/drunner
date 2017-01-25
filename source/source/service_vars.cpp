#include "service_vars.h"
#include "service_paths.h"

serviceVars::serviceVars(std::string servicename, const std::vector<Configuration> & config) : 
   persistvariables(servicename, servicePaths(servicename).getPathServiceVars(), config)
{
   _setconfig();
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

void serviceVars::setDevMode(bool isDevMode)
{
   setVal("DEVMODE", isDevMode ? "True" : "False");
}


void serviceVars::_setconfig()
{
   // we always know the servicename.
   setVal_mem("SERVICENAME", mName);
   _addConfig(Configuration("DEVMODE", "False", "Development Mode", false));
}
