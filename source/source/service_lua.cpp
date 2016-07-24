#include "service_lua.h"
#include "utils.h"
#include "globallogger.h"
#include "dassert.h"
#include "luautils.h"

#include "lua.hpp"

namespace servicelua
{
   static staticLuaStorage<luafile> sFile; // provide access to the calling file C++ object.
   
   extern "C" int l_addconfig(lua_State *L)
   {
      if (lua_gettop(L) != 1)
         return luaL_error(L, "Expected exactly one argument (the docker container to stop) for dstop.");
      std::string cname = lua_tostring(L, 1); // first argument. http://stackoverflow.com/questions/29449296/extending-lua-check-number-of-parameters-passed-to-a-function

      cResult rval = kRSuccess;
      lua_pushinteger(L, rval);
      return 1; // one argument to return.
   }

   extern "C" int l_addvolume(lua_State *L)
   {
      if (lua_gettop(L) != 1)
         return luaL_error(L, "Expected exactly one argument (the docker container to stop) for dstop.");
      std::string cname = lua_tostring(L, 1); // first argument. http://stackoverflow.com/questions/29449296/extending-lua-check-number-of-parameters-passed-to-a-function

      cResult rval = kRSuccess;
      lua_pushinteger(L, rval);
      return 1; // one argument to return.
   }

   extern "C" int l_addcontainer(lua_State *L)
   {
      if (lua_gettop(L) != 1)
         return luaL_error(L, "Expected exactly one argument (the docker container to stop) for dstop.");
      std::string cname = lua_tostring(L, 1); // first argument. http://stackoverflow.com/questions/29449296/extending-lua-check-number-of-parameters-passed-to-a-function

      cResult rval = kRSuccess;
      lua_pushinteger(L, rval);
      return 1; // one argument to return.
   }

   luafile::luafile(const servicePaths & p) : mServicePaths(p), mServiceVars(p)
   {
      drunner_assert(mServicePaths.getPathServiceLua().isFile(),"Coding error: path provided to simplefile is not a file!");
   }
 

   //void _setoperation(std::string opline, const variables &v, CommandLine &op)
   //{
   //   std::string sopline = v.substitute(opline);
   //   if (kRError == utils::split_in_args(sopline, op))
   //      fatal("Empty command line in yaml file");
   //}

   extern "C" static const struct luaL_Reg d_lualib[] = {
      { "addconfig", l_addconfig },
      { "addcontainer", l_addcontainer },
      { "addvolume", l_addvolume },
      { NULL, NULL }  /* sentinel */
   };
      
   extern "C" int luaopen_dlib(lua_State *L) {
      luaL_newlib(L, d_lualib);
      return 1;
   }
      
   cResult luafile::loadlua()
   {
      safeloadvars();

      Poco::Path path = mServicePaths.getPathServiceLua();
      drunner_assert(path.isFile(),"Coding error: path provided to loadyml is not a file.");
      drunner_assert(utils::fileexists(path),"The expected file does not exist: "+ path.toString()); // ctor of simplefile should have set mReadOkay to false if this wasn't true.

      lua_State * L = luaL_newstate();
      luaL_openlibs(L);

      staticmonitor<luafile> monitor(L, this, &sFile);

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


      ///* push functions and arguments */
      //lua_getglobal(L, "f"); /* function to be called */
      //lua_pushnumber(L, x); /* push 1st argument */
      //lua_pushnumber(L, y); /* push 2nd argument */
      //                      /* do the call (2 arguments, 1 result) */
      //if (lua_pcall(L, 2, 1, 0) != LUA_OK)
      //   error(L, "error running function 'f': %s",
      //      lua_tostring(L, -1));
      ///* retrieve result */
      //z = lua_tonumberx(L, -1, &isnum);
      //if (!isnum)
      //   error(L, "function 'f' must return a number");
      //lua_pop(L, 1); /* pop returned value */
      //return z;


      //YAML::Node yamlfile = YAML::LoadFile(mPath.toString());
      //drunner_assert(yamlfile,"Unable to read the yaml file: "+mPath.toString());

      //if (yamlfile["volumes"])
      //{ // load volumes
      //   YAML::Node volumes = yamlfile["volumes"];
      //   for (auto it = volumes.begin(); it != volumes.end(); ++it)
      //   {
      //      Volume vol;
      //      vol.name = v.substitute(it->first.as<std::string>());
      //      vol.backup = it->second["backup"].as<bool>();
      //      if (it->second["external"])
      //         vol.external = it->second["external"].as<bool>();
      //      mVolumes.push_back(vol);
      //   }
      //}

      //drunner_assert(yamlfile["help"], "All service.yml files are required to have a help command.");
      //mHelp = v.substitute(yamlfile["help"].as<std::string>());

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

   cResult luafile::safeloadvars()
   {
      // set defaults.
      mServiceVars.setServiceName(mServicePaths.getName());
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
      return mServiceVars.getImageName();
   }


} // namespace