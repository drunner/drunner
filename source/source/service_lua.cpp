#include <algorithm>

#include "service_lua.h"
#include "utils.h"
#include "globallogger.h"
#include "dassert.h"

#include "lua.hpp"

namespace servicelua
{
//   static luautils::staticLuaStorage<luafile> sFile; // provide access back to the calling luafile C++ object.
   

   luafile::luafile(std::string serviceName) : mServicePaths(serviceName), mServiceVars(mServicePaths), mLoaded(false)
   {
      drunner_assert(mServicePaths.getPathServiceLua().isFile(),"Coding error: path provided to simplefile is not a file!");

      L = luaL_newstate();
      luaL_openlibs(L);

      // add pointer to ourselves, so C functions can access us.
      lua_pushlightuserdata(L, (void*)this);  // value
      lua_setglobal(L, "luafile");
   }
      
   luafile::~luafile()
   {
      if (L)
         lua_close(L);
   }

   cResult luafile::loadlua()
   {
      drunner_assert(!mLoaded, "Load called multiple times."); // mainly because I haven't checked this makes sense.
      mLoaded = true; 

      Poco::Path path = mServicePaths.getPathServiceLua();
      drunner_assert(path.isFile(),"Coding error: path provided to loadlua is not a file.");
      if (!utils::fileexists(path))
         return kRError;

      _register_lua_cfuncs();

      int loadok = luaL_loadfile(L, path.toString().c_str());
      if (loadok != 0)
         fatal("Failed to load " + path.toString());
      if (lua_pcall(L, 0, LUA_MULTRET, 0) != 0)
         fatal("Failed to execute " + path.toString() + " " + lua_tostring(L, -1));

      // pull out the relevant config items.
      lua_getglobal(L, "drunner_setup");
      if (lua_isnil(L, -1))
         fatal("Lua file is not drunner compatible - there is no drunner_setup function defined.");
      if (lua_pcall(L, 0, 0, 0) != LUA_OK)
         fatal("Error running drunner_setup, " + std::string(lua_tostring(L, -1)));
      drunner_assert(lua_gettop(L) == 0, "Lua stack not empty after getglobal + pcall, when there's no error.");

      // set standard vars (always present).
      mServiceVars.setVariable("IMAGENAME", getImageName());
      mServiceVars.setVariable("SERVICENAME", mServicePaths.getName());

      // set defaults for custom vars.
      for (const auto & ci : mConfigItems)
         mServiceVars.setVariable(ci.name, ci.defaultval);

      // attempt to load config file.
      if (mServiceVars.loadconfig() != kRSuccess)
         logmsg(kLDEBUG, "Couldn't load service variables.");

      return kRSuccess;
   }


   cResult luafile::runCommand(const CommandLine & serviceCmd) const
   {
      drunner_assert(L != NULL, "service.lua has not been successfully loaded, can't run commands.");

      cResult rval;
      lua_getglobal(L, serviceCmd.command.c_str());
      if (lua_isnil(L, -1))
      {
         lua_pop(L, 1); // leave stack balanced.
         rval = kRNotImplemented;
      }
      else
      { // run the command
         if (lua_pcall(L, 0, 0, 0) != LUA_OK)
            fatal("Command " + serviceCmd.command + " failed:\n "+ std::string(lua_tostring(L,-1)));
         rval = kRSuccess;
      }
      drunner_assert(lua_gettop(L) == 0, "Lua stack not empty after runCommand.");
      return rval;
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

   void luafile::addConfiguration(Configuration cf)
   {
      mConfigItems.push_back(cf);
   }


   Poco::Path luafile::getPathdService()
   {
      return mServicePaths.getPathdService();
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
