#include "service_vars.h"
#include "service_paths.h"

//serviceVars::serviceVars(std::string servicename, const std::vector<envDef> & config) :
//   persistvariables(servicename, servicePaths(servicename).getPathServiceVars(), config)
//{
//   _extendconfig();
//}
#include "dassert.h"
#include "lua.hpp"
#include "service_lua.h"


extern "C" int l_addconfig(lua_State *L);
void _adddef(lua_State *L,  envDef def);
std::vector<envDef> loadServiceVariablesLua();

serviceVars::serviceVars(std::string servicename) :
   persistvariables(servicename, servicePaths(servicename).getPathServiceVars(), loadServiceVariablesLua())
{
   _extendconfig();

   cResult r = loadvariables();
   if (!r.success())
      logmsg(kLDEBUG, "Failed to load service variables.");
}

std::string serviceVars::getImageName() const
{
   std::string iname = getVal("IMAGENAME");
   if (iname.length() == 0) logmsg(kLWARN, "IMAGENAME not defined.");
   return iname;
}

std::string serviceVars::getServiceName() const
{
   std::string sname = getVal("SERVICENAME");
   if (sname.length() == 0) logmsg(kLWARN, "SERVICENAME not defined.");
   return sname;
}

bool serviceVars::getIsDevMode() const
{
   return getBool("DEVMODE");
}

void serviceVars::setImageName(std::string imagename)
{
   setVal("IMAGENAME", imagename);
}

void serviceVars::setDevMode(bool isDevMode)
{
   setVal("DEVMODE", isDevMode ? "True" : "False");
}


void serviceVars::_extendconfig()
{
   _addConfig(envDef("SERVICENAME", mName, "Name of Service", ENV_DEFAULTS_MEM)); // in memory, no need to persist this.
   _addConfig(envDef("IMAGENAME", "", "Image name", ENV_PERSISTS));
   _addConfig(envDef("DEVMODE", "False", "Development Mode", ENV_PERSISTS | ENV_USERSETTABLE));
}


// ------------------------------------------------------------------------------------------------------



void _adddef(lua_State *L, envDef def);
{
   int stacksize = lua_gettop(L);
   lua_getglobal(L, "luafile");
   drunner_assert(lua_islightuserdata(L, -1), "Is not lightuserdata.");
   void * luafilevoid = lua_touserdata(L, -1);
   drunner_assert(luafilevoid != NULL, "`luafile' expected");
   lua_pop(L, 1); // pop top element
   drunner_assert(stacksize == lua_gettop(L), "Stack unbalanced : " + std::to_string(stacksize) + " != " + std::to_string(lua_gettop(L)));
   return (luafile *)luafilevoid;
}

extern "C" int l_addconfig(lua_State *L)
{
   if (lua_gettop(L) != 3)
      return luaL_error(L, "Expected three arguments for addconfig.");

   // name(n), defaultval(dflt), description(desc), type(t), required(rqd), usersettable(user) {}
   drunner_assert(lua_isstring(L, 1), "The name must be a string.");
   drunner_assert(lua_isstring(L, 2), "The default value must be a string.");
   drunner_assert(lua_isstring(L, 3), "The description must be a string.");

   envDef def(lua_tostring(L, 1), lua_tostring(L, 2), lua_tostring(L, 3), ENV_PERSISTS | ENV_USERSETTABLE);
   servicelua::get_luafile(L)->addConfiguration(def);

   cResult rval = kRSuccess;
   lua_pushinteger(L, rval);
   return 1; // one argument to return.
}

cResult luafile::loadvariableslua()
{
   lua_State * LVars = luaL_newstate();
   luaL_openlibs(LVars);
   // add pointer to ourselves, so C functions can access us.
   lua_pushlightuserdata(LVars, (void*)this);  // value
   lua_setglobal(LVars, "luafile");

   // register the C function with lua.
   lua_pushcfunction(LVars, l_addconfig);
   lua_setglobal(LVars, "addconfig");

   std::string path = mServicePaths.getPathVariablesLua().toString();
   if (luaL_loadfile(L, path.c_str()))    /* Load but don't run the Lua script */
      fatal("Failed to load " + path + "\n" + lua_tostring(L, -1));

   // call global stuff
   if (lua_pcall(L, 0, 0, 0))                  /* Run the loaded Lua script */
      fatal("Failed to execute " + path + " " + lua_tostring(L, -1));

   lua_close(L);                               /* Clean up, free the Lua state var */

   return kRSuccess;
}

std::vector<envDef> loadServiceVariablesLua();
{
   std::vector<envDef> defs;


   return defs;
}