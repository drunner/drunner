#include "service_vars.h"
#include "service_paths.h"

serviceVars::serviceVars(std::string servicename, const std::vector<Configuration> & config) : 
   persistvariables(servicename, servicePaths(servicename).getPathServiceVars(), config)
{
   _setconfig();
}

serviceVars::serviceVars(std::string servicename, std::string imagename, const std::vector<Configuration> & config) :
   persistvariables(servicename, servicePaths(servicename).getPathServiceVars(), config)
{
   _setconfig();
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

void serviceVars::_setconfig()
{
   // we always know the servicename.
   setVal_mem("SERVICENAME", mName);
   
   _addConfig(Configuration("IMAGENAME", "", "Image name", kCF_string, true, false));
}
