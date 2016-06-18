#include <string>
#include <vector>
#include <fstream>

#include "dbackup.h"
#include "utils.h"
#include "globallogger.h"
#include "globalcontext.h"
#include "yaml-cpp/yaml.h"

namespace dbackup
{

   // -----------------------------------------------------------------------------------------------------------

   class backupConfig
   {
   public:
      backupConfig()
      {
         mIsConfigured = false;
      }

      bool load()
      {
         if (!utils::fileexists(path()))
            return false;

         YAML::Node config = YAML::LoadFile(path());
         if (!config)
            logmsg(kLERROR, "Failed to load " + path());
         if (!config[s_BackupPath])
            logmsg(kLERROR, path() + " exists but appears to be corrupt.");

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

      bool save()
      {
         YAML::Node config;
         config[s_BackupPath] = mBackupPath;
         for (auto s : mDisabledServices)
            config[s_DisabledServices].push_back(s);

         std::ofstream fout(path());
         if (!fout.is_open())
         {
            logmsg(kLWARN, "Couldn't write " + path());
            return false;
         }
         fout << config;

         return true;
      }

      std::string path()
      {
         return GlobalContext::getSettings()->getPath_drunnerbackups_cfg();
      }

      bool isEnabled(std::string servicename)
      {
         auto result = std::find(mDisabledServices.begin(), mDisabledServices.end(), servicename);
         return (result == mDisabledServices.end());
      }

      eResult enable(std::string servicename)
      {
         if (isEnabled(servicename))
            return kRNoChange;
         auto result = std::find(mDisabledServices.begin(), mDisabledServices.end(), servicename);
         if (result == mDisabledServices.end())
            logmsg(kLERROR, "dbackups: Result of isEnabled is broken?");
         mDisabledServices.erase(result);
         return kRSuccess;
      }
      eResult disable(std::string servicename)
      {
         if (!isEnabled(servicename))
            return kRNoChange;
         mDisabledServices.push_back(servicename);
         return kRSuccess;
      }


      std::vector<std::string> mDisabledServices;
      std::string mBackupPath;
      bool mIsConfigured;

      static const std::string s_DisabledServices;
      static const std::string s_BackupPath;
   };

   const std::string backupConfig::s_DisabledServices("DisabledServices");
   const std::string backupConfig::s_BackupPath("BackupPath");


   // -----------------------------------------------------------------------------------------------------------

   eResult include(std::string servicename)
   {
      backupConfig config;
      if (!config.load())
         logmsg(kLERROR, "Backups are not yet configured. Run dbackup configure first.");

      if (config.enable(servicename) == kRNoChange)
         return kRNoChange;

      if (!config.save())
         logmsg(kLERROR, "Failed to write to configuration file.");

      return kRSuccess;
   }

   eResult exclude(std::string servicename)
   {
      backupConfig config;
      if (!config.load())
         logmsg(kLERROR, "Backups are not yet configured. Run dbackup configure first.");

      if (config.disable(servicename) == kRNoChange)
         return kRNoChange;

      if (!config.save())
         logmsg(kLERROR, "Failed to write to configuration file.");

      return kRSuccess;
   }

   eResult run()
   {
      backupConfig config;
      if (!config.load())
      {
         logmsg(kLINFO, "Backups are not yet configured.  Run dbackup configure first.");
         return kRError;
      }

      return kRNotImplemented;
   }

   eResult configure(std::string path)
   {
      backupConfig config;
      config.load();

      // TODO: check path exists and make canonical.
      if (!utils::fileexists(path))
         logmsg(kLERROR, "The path " + path + " does not exist.");
      path = utils::getcanonicalpath(path);

      if (config.mBackupPath == path)
      {
         logmsg(kLINFO, "Path unchanged.");
         return kRNoChange;
      }

      config.mBackupPath = path;
      config.mIsConfigured = true;
      if (!config.save())
         logmsg(kLERROR, "Unable to save backup configuration file.");

      return kRSuccess;
   }

   eResult info()
   {
      backupConfig config;
      if (!config.load())
      {
         logmsg(kLINFO, "Backups are not yet configured.");
         return kRNoChange;
      }

      std::vector<std::string> services;
      utils::getAllServices(services);

      logmsg(kLINFO, "SERVICES TO BE BACKED UP:");
      for (auto const & s : services)
      {
         if (config.isEnabled(s))
            logmsg(kLINFO, "\e[32m\u2714\e[0m " + s);
         else
            logmsg(kLINFO, "\e[31m\e[1m\u2718\e[0m "+s);
      }
      logmsg(kLINFO, " ");
      logmsg(kLINFO, "Backups will be saved to:");
      logmsg(kLINFO, " " + config.mBackupPath);

      return kRNoChange;
   }

}

