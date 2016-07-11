#include "service_paths.h"
#include "drunner_paths.h"


Poco::Path servicePaths::getPath() const
{
   Poco::Path p = drunnerPaths::getPath_dServices().pushDirectory(mName);
   poco_assert(p.isDirectory());
   return p;
}

Poco::Path servicePaths::getPathdRunner() const
{
   return getPath().pushDirectory("drunner");
}

Poco::Path servicePaths::getPathHostVolume() const
{
   return drunnerPaths::getPath_HostVolumes().pushDirectory(mName);
}

Poco::Path servicePaths::getPathHostVolume_environment() const
{
   return getPathHostVolume().pushDirectory("environment");
}

Poco::Path servicePaths::getPathServiceRunner() const
{
   return getPathdRunner().setFileName("servicerunner");
}

Poco::Path servicePaths::getPathDockerCompose() const
{
   return getPathdRunner().setFileName("docker-compose.yml");
}

Poco::Path servicePaths::getPathLaunchScript() const
{
   return drunnerPaths::getPath_Bin().setFileName(getName());
}

std::string servicePaths::getName() const
{
   return mName;
}
