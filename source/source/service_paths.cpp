#include "service_paths.h"
#include "drunner_paths.h"



servicePaths::servicePaths(const std::string & servicename) :
   mName(servicename)
{
}

/*

.drunner
         /bin
         /dServices
                  /minecraft
                        /minecraft.lua
                        / (… any other files needed, e.g. docker-compose.yml)
                        /variables.json
         /settings
                  /drunnerSettings.json
         /logs
                  /log.txt
         /temp

*/


Poco::Path servicePaths::getPathdService() const
{
   Poco::Path p = drunnerPaths::getPath_dServices().pushDirectory(mName);
   poco_assert(p.isDirectory());
   return p;
}

Poco::Path servicePaths::getPathServiceLua() const
{
   return getPathdService().setFileName("service.lua");
}

Poco::Path servicePaths::getPathServiceVars() const
{
   return getPathHostVolume().setFileName("serviceconfig.json");
}

Poco::Path servicePaths::getPathLaunchScript() const
{
   return drunnerPaths::getPath_Bin().setFileName(getName());
}

std::string servicePaths::getName() const
{
   return mName;
}
