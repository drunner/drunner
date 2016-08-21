#include "Poco/String.h"

#include "service_lua.h"
#include "dassert.h"
#include "utils_docker.h"

// -----------------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------

// lua C functions - defined in service_lua_cfuncs
extern "C" int l_addconfig(lua_State *L);
extern "C" int l_addvolume(lua_State *L);
extern "C" int l_addcontainer(lua_State *L);
extern "C" int l_addproxy(lua_State *L);

extern "C" int l_drun(lua_State *L);
extern "C" int l_drun_output(lua_State *L);
extern "C" int l_dstop(lua_State *L);
extern "C" int l_dsub(lua_State *L);

#define REGISTERLUAC(cfunc,luaname) lua_pushcfunction(L, cfunc); lua_setglobal(L, luaname);

namespace servicelua
{

   // -----------------------------------------------------------------------------------------------------------------------

   void _register_lua_cfuncs(lua_State *L)
   { // define all our C functions that we want available from service.lua
      REGISTERLUAC(l_addconfig, "addconfig")
      REGISTERLUAC(l_addvolume, "addvolume")
      REGISTERLUAC(l_addcontainer, "addcontainer")
      REGISTERLUAC(l_addproxy,"addproxy")

      REGISTERLUAC(l_drun, "drun")
      REGISTERLUAC(l_drun_output, "drun_output")
      REGISTERLUAC(l_dstop, "dstop")  
      REGISTERLUAC(l_dsub, "dsub")
   }

   // -----------------------------------------------------------------------------------------------------------------------


   luafile * get_luafile(lua_State *L)
   {
      int stacksize = lua_gettop(L);
      lua_getglobal(L, "luafile");
      drunner_assert(lua_islightuserdata(L, -1), "Is not lightuserdata.");
      void * luafilevoid = lua_touserdata(L, -1); 
      drunner_assert(luafilevoid != NULL, "`luafile' expected");
      lua_pop(L, 1); // pop top element
      drunner_assert(stacksize == lua_gettop(L), "Stack unbalanced : "+std::to_string(stacksize)+" != "+std::to_string(lua_gettop(L)));
      return (luafile *)luafilevoid;
   }

   // -----------------------------------------------------------------------------------------------------------------------

   extern "C" int l_addconfig(lua_State *L)
   {
      if (lua_gettop(L) != 5)
         return luaL_error(L, "Expected exactly one argument (the docker container to stop) for addconfig.");

      configtype t;
      {
         using namespace utils;
         switch (s2i(lua_tostring(L, 4)))
         {
         case s2i("port"):
            t = kCF_port;
            break;
         case s2i("path"):
            t = kCF_path;
            break;
         case s2i("existingpath"):
            t = kCF_existingpath;
            break;
         case s2i("string"):
            t = kCF_string;
            break;
         default:
            fatal("Unknown configuration type: " + std::string(lua_tostring(L, 4)));
         }
      }

      Configuration c(lua_tostring(L, 1), lua_tostring(L, 3), lua_tostring(L, 2), t, (lua_toboolean(L, 5) == 1), true);
      get_luafile(L)->addConfiguration(c);

      cResult rval = kRSuccess;
      lua_pushinteger(L, rval);
      return 1; // one argument to return.
   }

   // -----------------------------------------------------------------------------------------------------------------------

   int _luasuccess(lua_State *L)
   {
      cResult rval = kRSuccess;
      lua_pushinteger(L, rval);
      return 1; // one argument to return.
   }

   // -----------------------------------------------------------------------------------------------------------------------


   extern "C" int l_addvolume(lua_State *L)
   {
      if (lua_gettop(L) < 1 || lua_gettop(L)>3)
         return luaL_error(L, "Expected one to three arguments: (name, backup, external) for addvolume.");

      luafile * lf = get_luafile(L);

      Volume v;
      v.name = lua_tostring(L, 1); // lf->getServiceVars()->substitute(lua_tostring(L, 1)); // first argument. http://stackoverflow.com/questions/29449296/extending-lua-check-number-of-parameters-passed-to-a-function
      v.backup = true;
      v.external = false;

      if (lua_gettop(L)>1)
         v.backup = (1 == lua_toboolean(L, 2));

      if (lua_gettop(L)>2)
         v.external = (1 == lua_toboolean(L, 3));

      lf->addVolume(v);
      
      return _luasuccess(L);
   }

   // -----------------------------------------------------------------------------------------------------------------------

   extern "C" int l_addcontainer(lua_State *L)
   {
      if (lua_gettop(L) != 1)
         return luaL_error(L, "Expected exactly one argument (the name of the container) for addcontainer.");
      std::string cname = lua_tostring(L, 1); // first argument. http://stackoverflow.com/questions/29449296/extending-lua-check-number-of-parameters-passed-to-a-function

      get_luafile(L)->addContainer(cname);

      return _luasuccess(L);
   }

   // -----------------------------------------------------------------------------------------------------------------------

   extern "C" int l_addproxy(lua_State *L)
   {
      if (lua_gettop(L) != 3)
         return luaL_error(L, "Expected exactly three arguments (VIRTUAL_HOST,HTTP_PORT,HTTPS_PORT) for addproxy.");
      Proxy p;
      p.vhost = lua_tostring(L, 1);
      p.dport_http = lua_tostring(L, 2);
      p.dport_https = lua_tostring(L, 3);

      get_luafile(L)->addProxy(p);

      return _luasuccess(L);
   }


   // -----------------------------------------------------------------------------------------------------------------------
   

   int drun(lua_State *L, bool returnOutput)
   {
      luafile * lf = get_luafile(L);

      CommandLine operation;
      operation.command = lua_tostring(L, 1);
      for (int i = 2; i <= lua_gettop(L); ++i)
         operation.args.push_back(lf->getServiceVars()->substitute(lua_tostring(L, i)));

      Poco::Path runin = lf->getPathdService();
      std::string out;
      utils::runcommand_stream(operation, kORaw, runin, lf->getServiceVars()->getAll(), &out);

      if (returnOutput)
      {
         Poco::trimInPlace(out);
         lua_pushstring(L, out.c_str());
         return 1; // one argument to return.
      }
      else
         return _luasuccess(L);
   }


   // -----------------------------------------------------------------------------------------------------------------------

   extern "C" int l_drun(lua_State *L)
   {
      if (lua_gettop(L) < 1)
         return luaL_error(L, "Expected at least one argument: drun( command,  arg1, arg2, ... )");

      return drun(L, false);
   }


   // -----------------------------------------------------------------------------------------------------------------------

   extern "C" int l_drun_output(lua_State *L)
   {
      if (lua_gettop(L) < 1)
         return luaL_error(L, "Expected at least one argument: drun_output( command,  arg1, arg2, ... )");

      return drun(L, true);
   }

   // -----------------------------------------------------------------------------------------------------------------------

   extern "C" int l_dstop(lua_State *L)
   {
      if (lua_gettop(L) != 1)
         return luaL_error(L, "Expected exactly one argument (the container name to stop) for dstop.");
      std::string containerraw = lua_tostring(L, 1);

      luafile *lf = get_luafile(L);
      std::string subcontainer = lf->getServiceVars()->substitute(containerraw);

      if (utils_docker::dockerContainerExists(subcontainer))
      {
         logmsg(kLINFO, "Stopping " + lf->getServiceName());
         utils_docker::stopContainer(subcontainer);
         utils_docker::removeContainer(subcontainer);
      }
      else
         logmsg(kLDEBUG, subcontainer + " is not running.");
      return _luasuccess(L);
   }

   // -----------------------------------------------------------------------------------------------------------------------

   extern "C" int l_dsub(lua_State *L)
   {
      if (lua_gettop(L) != 1)
         return luaL_error(L, "Expected exactly one argument (the string to substitute) for dsub.");

      luafile * lf = get_luafile(L);
      std::string s = lf->getServiceVars()->substitute(lua_tostring(L, 1));
      lua_pushstring(L, s.c_str());
      return 1; // one argument to return.
   }
}
