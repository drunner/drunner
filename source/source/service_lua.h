#ifndef __SERVICE_YML_H
#define __SERVICE_YML_H

#include <string>
#include <vector>

#include <Poco/Path.h>
#include "cresult.h"
#include "utils.h"
#include "service_paths.h"
#include "variables.h"
#include "lua.hpp"
#include "service_vars.h"

namespace servicelua
{
   struct Volume 
   {
      bool backup;
      bool external;
      std::string name;
   };

   struct Container
   {
      std::string name;
      bool runasroot;
   };

   struct BackupVol
   {
      std::string volumeName;
      std::string backupName;
   };

   // lua file.
   class luafile {
   public:
      luafile(std::string serviceName);
      ~luafile();
      
      // loads the lua file, initialises the variables, loads the variables if able.
      cResult loadlua();

      const std::vector<Container> & getContainers() const;
      const std::vector<envDef> & getLuaConfigurationDefinitions() const;
      void getManageDockerVolumeNames(std::vector<std::string> & vols) const;
      void getBackupDockerVolumeNames(std::vector<BackupVol> & vols) const;

      // for service::serviceRunnerCommand
      cResult runCommand(const CommandLine & serviceCmd, serviceVars * sVars);

      bool isLoaded() { return mLuaLoaded; }

      std::string getServiceName() { return mServicePaths.getName(); }

      // for lua
      void addContainer(Container c);
      void addConfiguration(envDef cf);
      void addVolume(Volume v);
      Poco::Path getdRunDir() const;
      void setdRunDir(std::string p);

      Poco::Path getPathdService();
      serviceVars * getServiceVars();

   private:
      cResult _showHelp(serviceVars * sVars);

      const servicePaths mServicePaths;

      std::vector<Container> mContainers;
      std::vector<envDef> mLuaConfigurationDefinitions; // This is not the full configuration for the service, just the parts defined by service.lua (e.g. missing IMAGENAME, DEVMODE).
      std::vector<Volume> mVolumes;

      lua_State * L;
      serviceVars * mSVptr;

      bool mLuaLoaded;
      bool mVarsLoaded;
      bool mLoadAttempt;

      Poco::Path mdRunDir;
   };

   void _register_lua_cfuncs(lua_State *L);

} // namespace



#endif
