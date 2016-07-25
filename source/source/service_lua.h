#ifndef __SERVICE_YML_H
#define __SERVICE_YML_H

#include <string>
#include <vector>

#include <Poco/Path.h>
#include "cresult.h"
#include "utils.h"
#include "service_vars.h"
#include "service_paths.h"
#include "luautils.h"

namespace servicelua
{

   struct Volume 
   {
      bool backup;
      bool external;
      std::string name;
   };

   enum configtype
   {
      kCF_port,
      kCF_path,
      kCF_existingpath,
      kCF_string,
   };

   struct Configuration 
   {
      std::string name;
      std::string defaultval;
      std::string description;
      configtype type;
      bool required;
   };


   // lua file.
   class luafile {
   public:
      luafile(const servicePaths & p);
      
      // loads the lua file, initialises the variables, loads the variables if able.
      cResult loadlua();

      std::string getHelp() const { return mHelp; }
      std::string getImageName() const;

      const std::vector<std::string> & getContainers() const;
      const std::vector<Configuration> & getConfigItems() const;
      void getManageDockerVolumeNames(std::vector<std::string> & vols) const;
      void getBackupDockerVolumeNames(std::vector<std::string> & vols) const;

      // pass through commands for the service variables.
      cResult saveVariables() const { return mServiceVars.saveconfig(); }
      const variables & getVariables() const { return mServiceVars.getVariables(); }
      void setVariable(std::string key, std::string val) { mServiceVars.setVariable(key, val); }

      // for service::serviceRunnerCommand
      cResult runCommand(const CommandLine & serviceCmd) const;

      // for lua
      void addContainer(std::string cname);

   private:
      cResult _safeloadvars();

      const servicePaths & mServicePaths;
      
      serviceVars mServiceVars;

      std::vector<std::string> mContainers;
      std::vector<Configuration> mConfigItems;
      std::vector<Volume> mVolumes;

      std::string mHelp;

      mutable luautils::dLuaState mL;
      luautils::staticmonitor<luafile> mMonitor;
   };

} // namespace

#endif
