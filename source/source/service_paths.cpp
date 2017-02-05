#include "service_paths.h"
#include "drunner_paths.h"
#include "utils.h"
#include "timez.h"


servicePaths::servicePaths(const std::string & servicename) :
   mName(servicename)
{
}

Poco::Path servicePaths::getPathdService() const
{
   Poco::Path p = drunnerPaths::getPath_dServices().pushDirectory(mName);
   poco_assert(p.isDirectory());
   return p;
}

Poco::Path servicePaths::getPathHostVolume() const
{
   return drunnerPaths::getPath_HostVolumes().pushDirectory(mName);
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

backupPathManager::backupPathManager(std::string servicename) :
   servicePaths(servicename),
   mTempFolder(drunnerPaths::getPath_Temp().pushDirectory("service-backuprestore-" + timeutils::getDateTimeStr()))
{
   poco_assert(utils::makedirectory(getPathArchive(), S_777).success());
   poco_assert(utils::makedirectory(getPathSubArchives(), S_777).success());
}

Poco::Path backupPathManager::getPathTempFolder() const
{
   return mTempFolder.getpath();
}

Poco::Path backupPathManager::getPathArchive() const
{
   return getPathTempFolder().pushDirectory("archive");
}

Poco::Path backupPathManager::getPathSubArchives() const
{
   return getPathTempFolder().pushDirectory("subarchives");
}

Poco::Path backupPathManager::getPathHostVolArchiveFile() const
{
   return getPathSubArchives().setFileName("hostvol.tar");
}

Poco::Path backupPathManager::getPathdServiceDefArchiveFile() const
{
   return getPathSubArchives().setFileName("dservicedef.tar");
}

Poco::Path backupPathManager::getPathArchiveFile() const
{
   return getPathArchive().setFileName("fullarchive.tar.enc");
}
