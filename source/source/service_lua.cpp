#include <algorithm>

#include "service_lua.h"
#include "utils.h"
#include "globallogger.h"
#include "dassert.h"
#include "luautils.h"

#include "lua.hpp"

namespace servicelua
{
   static luautils::staticLuaStorage<luafile> sFile; // provide access back to the calling luafile C++ object.
   
   extern "C" int l_addconfig(lua_State *L)
   {
      if (lua_gettop(L) != 5)
         return luaL_error(L, "Expected exactly one argument (the docker container to stop) for addconfig.");

      Configuration c;
      c.name = lua_tostring(L, 1);
      c.description = lua_tostring(L, 2);
      c.defaultval = lua_tostring(L, 3);
      c.required = (lua_toboolean(L, 5) == 1);

      {
         using namespace utils;
         switch (str2int(lua_tostring(L, 4)))
         {
         case str2int("port"):
            c.type = kCF_port;
            break;
         case str2int("path"):
            c.type = kCF_path;
            break;
         case str2int("existingpath"):
            c.type = kCF_existingpath;
            break;
         case str2int("string"):
            c.type = kCF_string;
            break;
         default:
            fatal("Unknown configuration type: " + std::string(lua_tostring(L, 4)));
         }
      }

      cResult rval = kRSuccess;
      lua_pushinteger(L, rval);
      return 1; // one argument to return.
   }

   extern "C" int l_addvolume(lua_State *L)
   {
      if (lua_gettop(L) != 1)
         return luaL_error(L, "Expected exactly one argument (the docker container to stop) for addvolume.");
      std::string cname = lua_tostring(L, 1); // first argument. http://stackoverflow.com/questions/29449296/extending-lua-check-number-of-parameters-passed-to-a-function

      cResult rval = kRSuccess;
      lua_pushinteger(L, rval);
      return 1; // one argument to return.
   }

   extern "C" int l_addcontainer(lua_State *L)
   {
      if (lua_gettop(L) != 1)
         return luaL_error(L, "Expected exactly one argument (the name of the container) for addcontainer.");
      std::string cname = lua_tostring(L, 1); // first argument. http://stackoverflow.com/questions/29449296/extending-lua-check-number-of-parameters-passed-to-a-function

      sFile.get(L)->addContainer(cname);

      cResult rval = kRSuccess;
      lua_pushinteger(L, rval);
      return 1; // one argument to return.
   }

   luafile::luafile(const servicePaths & p) : mServicePaths(p), mServiceVars(p), mL(), mMonitor(mL.get(),this,&sFile)
   {
      drunner_assert(mServicePaths.getPathServiceLua().isFile(),"Coding error: path provided to simplefile is not a file!");
   }
      
   cResult luafile::loadlua()
   {
      lua_State * L = mL.get();
      drunner_assert(L==NULL,"Load called on dirty lua state.");

      _safeloadvars(); // set defaults, load real values if we can

      Poco::Path path = mServicePaths.getPathServiceLua();
      drunner_assert(path.isFile(),"Coding error: path provided to loadlua is not a file.");
      if (!utils::fileexists(path))
         return kRError;

      lua_pushcfunction(L, l_addconfig);   // see also http://stackoverflow.com/questions/2907221/get-the-lua-command-when-a-c-function-is-called
      lua_setglobal(L, "addconfig");
      lua_pushcfunction(L, l_addvolume);
      lua_setglobal(L, "addvolume");
      lua_pushcfunction(L, l_addcontainer);
      lua_setglobal(L, "addcontainer");

      int loadok = luaL_loadfile(L, path.toString().c_str());
      if (loadok != 0)
         fatal("Failed to load " + path.toString());
      if (lua_pcall(L, 0, LUA_MULTRET, 0) != 0)
         fatal("Failed to execute " + path.toString() + " " + lua_tostring(L, -1));

      // pull out the relevant config items.
      lua_getglobal(L, "drunner_setup");
      if (lua_pcall(L, 0, 0, 0) != LUA_OK)
         fatal("Error running drunner_setup, " + std::string(lua_tostring(L, -1)));
      
      // update IMAGENAME if not yet set.
      if (mServiceVars.getVariables().hasKey("IMAGENAME"))
         drunner_assert(utils::stringisame(mServiceVars.getVariables().getVal("IMAGENAME"), getImageName()), "Service's IMAGENAME has changed.");
      mServiceVars.setVariable("IMAGENAME", getImageName());

      return kRSuccess;
   }


   cResult luafile::runCommand(const CommandLine & serviceCmd) const
   {
      lua_State * L = mL.get();
      drunner_assert(L != NULL, "service.lua has not been successfully loaded, can't run commands.");

      return kRSuccess;
   }



   void luafile::getManageDockerVolumeNames(std::vector<std::string> & vols) const
   {
      drunner_assert(vols.size() == 0,"Coding error: passing dirty volume vector to getManageDockerVolumeNames");
      for (const auto & v : mVolumes)
         if (v.external)
            vols.push_back(v.name);
   }
   void luafile::getBackupDockerVolumeNames(std::vector<std::string> & vols) const
   {
      drunner_assert(vols.size() == 0, "Coding error: passing dirty volume vector to getBackupDockerVolumeNames");
      for (const auto & v : mVolumes)
         if (v.backup)
            vols.push_back(v.name);
   }


   void luafile::addContainer(std::string cname)
   {
      drunner_assert(cname.size() > 0, "Empty container name passed to addContainer.");
      drunner_assert(std::find(mContainers.begin(), mContainers.end(), cname) == mContainers.end(), "Container already exists " + cname);
      mContainers.push_back(cname);
   }

   cResult luafile::_safeloadvars()
   {
      // set standard vars (always present).
      mServiceVars.setVariable("SERVICENAME", mServicePaths.getName());
      drunner_assert(mContainers.size() == 0, "_safeloadvars: containers already defined.");

      // set defaults for custom vars.
      for (const auto & ci : mConfigItems)
         mServiceVars.setVariable(ci.name, ci.defaultval);

      // attempt to load config file.
      if (mServiceVars.loadconfig() != kRSuccess)
         logmsg(kLDEBUG, "Couldn't load service variables.");

      // we always succeed.
      return kRSuccess;
   }

   const std::vector<std::string> & luafile::getContainers() const
   {
      return mContainers;
   }
   const std::vector<Configuration> & luafile::getConfigItems() const
   {
      return mConfigItems;
   }

   std::string luafile::getImageName() const
   {
      drunner_assert(mContainers.size() > 0, "getImageName: no containers defined.");
      return mContainers[0];
   }


} // namespace