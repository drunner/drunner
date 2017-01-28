#include <algorithm>
#include <Poco/String.h>

#include "service_lua.h"
#include "utils.h"
#include "globallogger.h"
#include "dassert.h"

#include "lua.hpp"

/*

The luafile class is a combination of:

A) the interpreted service.lua, which defines:
    i)   the commands the dService can run
    ii)  the docker containers to be used
    iii) the docker volumes to manage/back up
    iv)  the user-configurable variables for the dService (e.g. port)

and

B) the service configuration variables, which includes:
    i)   the variables set by the user (as per (iv) above)
    ii)  the variable IMAGENAME set to the installed image name
    iii) the in-memory SERVICENAME variable

*/


namespace servicelua
{   
 // -------------------------------------------------------------------------------



   luafile::luafile(const serviceVars & sv) :
      mServicePaths(sv.getServiceName()), 
      mLuaLoaded(false), 
      mLoadAttempt(false),
      mServiceVars(sv)
   {
      drunner_assert(mServicePaths.getPathServiceLua().isFile(), "Coding error: services.lua path provided to luafile is not a file!");
      drunner_assert(mServicePaths.getPathVariablesLua().isFile(), "Coding error: variables.lua path provided to luafile is not a file!");

      L = luaL_newstate();
      luaL_openlibs(L);

      // add pointer to ourselves, so C functions can access us.
      lua_pushlightuserdata(L, (void*)this);  // value
      lua_setglobal(L, "luafile");

      // set the current working directory to where the service.lua file is.
      setdRunDir("");
      drunner_assert(mdRunDir.isDirectory(), "mPwd is not a directory");
   }
      
   // -------------------------------------------------------------------------------

   luafile::~luafile()
   {
      if (L)
         lua_close(L);
   }

   // -------------------------------------------------------------------------------


   cResult luafile::loadlua()
   {
      drunner_assert(!mLoadAttempt, "Load called multiple times."); // mainly because I haven't checked this makes sense.
      mLoadAttempt = true;

      if (!utils::fileexists(mServicePaths.getPathVariablesLua()))
         return cError("loadlua: the variables.lua file does not exist: " + mServicePaths.getPathVariablesLua().toString());

      cResult loadvarresult = loadvariableslua();
      if (!loadvarresult.success())
         return loadvarresult;

      Poco::Path path = mServicePaths.getPathServiceLua();
      drunner_assert(path.isFile(),"Coding error: path provided to loadlua is not a file.");
      if (!utils::fileexists(path))
         return cError("loadlua: the service.lua file does not exist: " + path.toString());

      _register_lua_cfuncs(L);

      int loadok = luaL_loadfile(L, path.toString().c_str());
      if (loadok != 0)
         fatal("Failed to load " + path.toString() + "\n"+ lua_tostring(L, -1));
      if (lua_pcall(L, 0, LUA_MULTRET, 0) != 0)
         fatal("Failed to execute " + path.toString() + " " + lua_tostring(L, -1));

      // pull out the relevant config items.
      lua_getglobal(L, "drunner_setup");
      if (lua_isnil(L, -1))
         fatal("Lua file is not drunner compatible - there is no drunner_setup function defined.");
      if (lua_pcall(L, 0, 0, 0) != LUA_OK)
         fatal("Error running drunner_setup, " + std::string(lua_tostring(L, -1)));
      drunner_assert(lua_gettop(L) == 0, "Lua stack not empty after getglobal + pcall, when there's no error.");

      mLuaLoaded = true;
      return kRSuccess;
   }

   // -------------------------------------------------------------------------------

   cResult luafile::_showHelp()
   {
      if (!mLuaLoaded)
         return cError("Can't show help because we weren't able to load the service.lua file.");

      lua_getglobal(L, "help");
      if (lua_isnil(L, -1))
         fatal("Help not defined in service.lua for " + getServiceName());
      if (lua_pcall(L, 0, 1, 0) != LUA_OK)
         fatal("Couldn't run help command in service.lua for " + getServiceName());
      if (lua_gettop(L) < 1)
         fatal("Help function didn't return anything.");
      if (lua_gettop(L) > 1)
         fatal("Help function returned too many arguments.");
      if (lua_isstring(L, 1) != 1)
         fatal("The argument returned by help was not a string, rather "+std::string(lua_typename(L,1)));
      std::string help = lua_tostring(L, 1);

      logmsg(kLINFO, mServiceVars.substitute(help));
      return kRSuccess;
   }

   // -------------------------------------------------------------------------------

   cResult luafile::runCommand(const CommandLine & serviceCmd)
   {
      drunner_assert(L != NULL, "service.lua has not been successfully loaded, can't run commands.");
      drunner_assert(Poco::icompare(serviceCmd.command, "configure") != 0, "Configure leaked through");

      if (Poco::icompare(serviceCmd.command, "help") == 0)
         return _showHelp();

      cResult rval;
      lua_getglobal(L, serviceCmd.command.c_str());
      if (lua_isnil(L, -1))
      {
         lua_pop(L, 1); // leave stack balanced.
         rval = kRNotImplemented;
      }
      else
      { // run the command
         for (auto x : serviceCmd.args)
            lua_pushstring(L, x.c_str());

         if (lua_pcall(L, serviceCmd.args.size(), 1, 0) != LUA_OK)
            fatal("Command " + serviceCmd.command + " failed:\n "+ std::string(lua_tostring(L,-1)));

         if (!lua_isnumber(L, -1))
         {
            logdbg("No return code from command " + serviceCmd.command);
            rval = kRSuccess;
         }
         else
            rval = (int)lua_tonumber(L, -1);
      }

      lua_settop(L, 0); // clear stack in case function didn't use args etc.
      return rval;
   }

   // -------------------------------------------------------------------------------

   void luafile::getManageDockerVolumeNames(std::vector<std::string> & vols) const
   {
      drunner_assert(vols.size() == 0,"Coding error: passing dirty volume vector to getManageDockerVolumeNames");
      for (const auto & v : mVolumes)
         if (!v.external)
         {
            std::string volname = utils::replacestring(v.name, "${SERVICENAME}", mServicePaths.getName());
            vols.push_back(volname);
         }
   }

   // -------------------------------------------------------------------------------

   void luafile::getBackupDockerVolumeNames(std::vector<BackupVol> & vols) const
   {
      drunner_assert(vols.size() == 0, "Coding error: passing dirty volume vector to getBackupDockerVolumeNames");
      for (const auto & v : mVolumes)
         if (v.backup)
         {
            BackupVol bv;
            bv.volumeName = utils::replacestring(v.name, "${SERVICENAME}", mServicePaths.getName());
            bv.backupName = utils::replacestring(v.name, "${SERVICENAME}", "SERVICENAME");
            vols.push_back(bv);
         }
   }

   // -------------------------------------------------------------------------------

   void luafile::addContainer(Container c)
   {
      drunner_assert(c.name.size() > 0, "Empty container name passed to addContainer.");
      //drunner_assert(std::find(mContainers.begin(), mContainers.end(), c.name) == mContainers.end(), "Container already exists " + c.name);
      mContainers.push_back(c);
   }

   void luafile::addConfiguration(envDef cf)
   {
      mLuaConfigurationDefinitions.push_back(cf);
   }

   void luafile::addVolume(Volume v)
   {
      mVolumes.push_back(v);
   }

   Poco::Path luafile::getdRunDir() const
   {
      return mdRunDir;
   }

   void luafile::setdRunDir(std::string p)
   {
      if (p.length() > 0)
      {
         mdRunDir = p;
         mdRunDir.makeDirectory();
      }
      else
         mdRunDir = getPathdService();

      drunner_assert(mdRunDir.isDirectory(), "mPwd is not a directory");
   }


   Poco::Path luafile::getPathdService()
   {
      return mServicePaths.getPathdService();
   }

   const std::vector<Container> & luafile::getContainers() const
   {
      return mContainers;
   }
   const std::vector<envDef> & luafile::getLuaConfigurationDefinitions() const
   {
      return mLuaConfigurationDefinitions;
   }

} // namespace
