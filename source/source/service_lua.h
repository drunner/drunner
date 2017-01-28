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
   // lua file.
   class luafile {
   public:
      // runs the given command.
      luafile(serviceVars & sv, const CommandLine & serviceCmd);
      ~luafile();

      cResult getResult() { return mResult; }
      
      // for lua
      void addConfiguration(envDef cf);
      Poco::Path getdRunDir() const;
      void setdRunDir(std::string p);
      std::string getServiceName() { return mServicePaths.getName(); }

      serviceVars & getServiceVars() { return mServiceVars; }
      //Poco::Path getPathdService();

   private:
      // loads the lua file, initialises the variables, loads the variables if able.
      cResult _loadlua();
      cResult _showHelp();
      cResult _runCommand(const CommandLine & serviceCmd);

      const servicePaths mServicePaths;

      lua_State * L;
      serviceVars & mServiceVars;

      Poco::Path mdRunDir;
      cResult mResult;
   };

   void _register_lua_cfuncs(lua_State *L);

   luafile * get_luafile(lua_State *L);

} // namespace




#endif
