#include <fstream>

#include "yaml-cpp/yaml.h"

#include "backupConfig.h"
#include "enums.h"
#include "globalcontext.h"
#include "globallogger.h"
#include "utils.h"

const std::string backupConfig::s_DisabledServices("DisabledServices");
const std::string backupConfig::s_BackupPath("BackupPath");

backupConfig::backupConfig()
{
   mIsConfigured = false;
}

bool backupConfig::load()
{
   if (!utils::fileexists(configfilepath()))
      return false;

   YAML::Node config = YAML::LoadFile(configfilepath().toString());
   if (!config)
      logmsg(kLERROR, "Failed to load " + configfilepath().toString());
   if (!config[s_BackupPath])
      logmsg(kLERROR, configfilepath().toString() + " exists but appears to be corrupt.");

   mIsConfigured = true;
   mBackupPath = config[s_BackupPath].as<std::string>();
   mDisabledServices.clear();

   if (config[s_DisabledServices])
   {
      YAML::Node DisabledServices = config[s_DisabledServices];
      for (auto it = DisabledServices.begin(); it != DisabledServices.end(); ++it)
         mDisabledServices.push_back(it->as<std::string>());
   }

   return true;
}

bool backupConfig::save()
{
   YAML::Node config;
   config[s_BackupPath] = mBackupPath;
   for (auto s : mDisabledServices)
      config[s_DisabledServices].push_back(s);

   std::ofstream fout(configfilepath().toString());
   if (!fout.is_open())
   {
      logmsg(kLWARN, "Couldn't write " + configfilepath().toString());
      return false;
   }
   fout << config;

   return true;
}

Poco::Path backupConfig::configfilepath()
{
   return GlobalContext::getSettings()->getPath_drunnerbackups_cfg();
}

bool backupConfig::isEnabled(std::string servicename)
{
   auto result = std::find(mDisabledServices.begin(), mDisabledServices.end(), servicename);
   return (result == mDisabledServices.end());
}

eResult backupConfig::enable(std::string servicename)
{
   if (isEnabled(servicename))
      return kRNoChange;
   auto result = std::find(mDisabledServices.begin(), mDisabledServices.end(), servicename);
   if (result == mDisabledServices.end())
      logmsg(kLERROR, "dbackups: Result of isEnabled is broken?");
   mDisabledServices.erase(result);
   return kRSuccess;
}
eResult backupConfig::disable(std::string servicename)
{
   if (!isEnabled(servicename))
      return kRNoChange;
   mDisabledServices.push_back(servicename);
   return kRSuccess;
}

void backupConfig::update(std::string backuppath)
{
   mBackupPath = backuppath;
   mIsConfigured = true;
}

std::string backupConfig::getBackupPath()
{
   return mBackupPath;
}

