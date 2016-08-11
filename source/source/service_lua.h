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

namespace servicelua
{

   struct Volume 
   {
      bool backup;
      bool external;
      std::string name;
   };

   // lua file.
   class luafile {
   public:
      luafile(std::string serviceName);
      ~luafile();
      
      // loads the lua file, initialises the variables, loads the variables if able.
      cResult loadlua();
      cResult showHelp();

      std::string getImageName() const;

      const std::vector<std::string> & getContainers() const;
      const std::vector<Configuration> & getConfigItems() const;
      void getManageDockerVolumeNames(std::vector<std::string> & vols) const;
      void getBackupDockerVolumeNames(std::vector<std::string> & vols) const;

      // pass through commands for the service variables.
      cResult saveVariables() const { return mServiceVars.savevariables(); }
      const variables getVariables() const { return mServiceVars.getVariables(); }
      cResult setVariable(std::string key, std::string val) { return mServiceVars.setVal(key, val); }

      // for service::serviceRunnerCommand
      cResult runCommand(const CommandLine & serviceCmd);

      bool isLoaded() { return mLuaLoaded; }
      bool isVariablesLoaded() { return mVarsLoaded; }

      std::string getServiceName() { return mServicePaths.getName(); }

      // for lua
      void addContainer(std::string cname);
      void addConfiguration(Configuration cf);
      void addVolume(Volume v);
      Poco::Path getPathdService();

   private:
      const servicePaths mServicePaths;
      persistvariables mServiceVars;

      std::vector<std::string> mContainers;
      std::vector<Configuration> mConfigItems;
      std::vector<Volume> mVolumes;

      lua_State * L;

      bool mLuaLoaded;
      bool mVarsLoaded;
      bool mLoadAttempt;
   };

   void _register_lua_cfuncs(lua_State *L);

} // namespace



#endif
