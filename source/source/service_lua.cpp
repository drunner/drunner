#include <algorithm>
#include <Poco/String.h>
#include <fstream>

#include "service_lua.h"
#include "utils.h"
#include "globallogger.h"
#include "dassert.h"

#include "lua.hpp"

/*

The luafile class handles the interpreted service.lua, which defines the commands the dService can run

*/

namespace servicelua
{   
 // -------------------------------------------------------------------------------

   luafile::luafile(serviceVars & sv, const CommandLine & serviceCmd) :
      mServicePaths(sv.getServiceName()), 
      mServiceVars(sv)
   {
      drunner_assert(mServicePaths.getPathServiceLua().isFile(), "Coding error: services.lua path provided to luafile is not a file!");

      L = luaL_newstate();
      luaL_openlibs(L);

      // add pointer to ourselves, so C functions can access us.
      lua_pushlightuserdata(L, (void*)this);  // value
      lua_setglobal(L, "luafile");

      // set the current working directory to where the service.lua file is.
      setdRunDir("");
      drunner_assert(mdRunDir.isDirectory(), "mPwd is not a directory");

      // load the lua file and run the command.
      mResult = _loadlua();
      if (!mResult.success())
         fatal("Could not load the service.lua file from " + mServicePaths.getPathServiceLua().toString()+"\n"+ mResult.what());

      mResult = _runCommand(serviceCmd);
   }
      
   // -------------------------------------------------------------------------------

   luafile::~luafile()
   {
      if (L)
         lua_close(L);
   }

   // -------------------------------------------------------------------------------

   bool iscomment(std::string line)
   {
      Poco::trimInPlace(line);
      if (line.length() == 0) return true;
      return (line.find("--") == 0);
   }

   cResult luafile::_loadlua()
   {
      logmsg(kLDEBUG, "Loading lua.");

      Poco::Path path = mServicePaths.getPathServiceLua();
      drunner_assert(path.isFile(),"Coding error: path provided to loadlua is not a file.");
      if (!utils::fileexists(path))
         return cError("loadlua: the service.lua file does not exist: " + path.toString());

      logmsg(kLDEBUG, "Registering C functions.");
      _register_lua_cfuncs(L);

      //int loadok = luaL_loadfile(L, path.toString().c_str());
      //if (loadok != 0)
      //   fatal("Failed to load " + path.toString() + "\n"+ lua_tostring(L, -1));
      ////if (lua_pcall(L, 0, LUA_MULTRET, 0) != 0)
      //if (lua_pcall(L, 0, 0, 0) != 0)
      //   fatal("Failed to execute " + path.toString() + " " + lua_tostring(L, -1));


      std::ifstream infile(path.toString());
      std::string line, wholefile;
      if (!infile.is_open())
         fatal("Couldn't open service.lua at " + path.toString());

      logmsg(kLDEBUG, "Executing "+path.toString());

      while (std::getline(infile, line))
      {  // substitute variables in the line.
         bool comment = iscomment(line);
         if (!comment)
         {
            while (true) // breakable loop
            {
               size_t pos = line.find("${");
               if (pos == std::string::npos)
                  break;
               size_t pos2 = line.find("}", pos);
               if (pos2 == std::string::npos)
                  fatal("Unmatched ${ in lua file: \n" + line);
               std::string key = line.substr(pos + 2, pos2 - pos - 2);
               std::string repstr = utils::getenv(key);
               if (repstr.length() == 0)
                  repstr = mServiceVars.getVal(key);
               line.replace(pos, pos2 - pos + 1, repstr);
            }
         }
         logmsg(kLDEBUG, "<< " + line);
         wholefile += line + "\n";
      }
      // line has had variables substituted. Execute.
      int error = luaL_loadbuffer(L, wholefile.c_str(), wholefile.length(), path.toString().c_str());
      if (error)
         fatal("Failed loading:\n" + wholefile + "\n\n" + lua_tostring(L, -1));

      if (lua_pcall(L, 0, 0, 0) != 0)
         fatal("Failed to execute " + path.toString() + ":\n" + lua_tostring(L, -1));



      //// pull out the relevant config items.
      //lua_getglobal(L, "drunner_setup");
      //if (lua_isnil(L, -1))
      //   fatal("Lua file is not drunner compatible - there is no drunner_setup function defined.");
      //if (lua_pcall(L, 0, 0, 0) != LUA_OK)
      //   fatal("Error running drunner_setup, " + std::string(lua_tostring(L, -1)));
      drunner_assert(lua_gettop(L) == 0, "Lua stack not empty after getglobal + pcall, when there's no error.");

      return kRSuccess;
   }

   // -------------------------------------------------------------------------------

   cResult luafile::_showHelp()
   {
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

   cResult luafile::_runCommand(const CommandLine & serviceCmd)
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
         mdRunDir = mServicePaths.getPathdService(); 
      
      drunner_assert(mdRunDir.isDirectory(), "mPwd is not a directory");
   }

} // namespace
