#include "Poco/String.h"
#include "Poco/StringTokenizer.h"

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
extern "C" int l_drun_outputexit(lua_State *L);
extern "C" int l_dstop(lua_State *L);
extern "C" int l_dsub(lua_State *L);
extern "C" int l_dconfig_get(lua_State *L);
extern "C" int l_dconfig_set(lua_State *L);
extern "C" int l_dsplit(lua_State *L);

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
      REGISTERLUAC(l_drun_outputexit, "drun_outputexit")
      REGISTERLUAC(l_dstop, "dstop")
      REGISTERLUAC(l_dsub, "dsub")
      REGISTERLUAC(l_dconfig_get, "dconfig_get")
      REGISTERLUAC(l_dconfig_set, "dconfig_set")
      REGISTERLUAC(l_dsplit, "dsplit")
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
         return luaL_error(L, "Expected five arguments for addconfig.");

      // name(n), defaultval(dflt), description(desc), type(t), required(rqd), usersettable(user) {}
      drunner_assert(lua_isstring(L, 1), "The name must be a string.");
      drunner_assert(lua_isstring(L, 2), "The default value must be a string.");
      drunner_assert(lua_isstring(L, 3), "The decsription must be a string.");
      drunner_assert(lua_isstring(L, 4), "The type must be a string.");
      drunner_assert(lua_isboolean(L, 5), "The required flag must be a boolean.");

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
      drunner_assert(lua_isstring(L, 1), "The name of the volume must be a string.");
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
      drunner_assert(lua_isstring(L, 1), "The name of the container must be a string.");
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
      drunner_assert(lua_isstring(L, 1), "virtual host must be a string.");
      drunner_assert(lua_isstring(L, 2), "http port must be a string.");
      drunner_assert(lua_isstring(L, 3), "https port must be a string.");
      p.vhost = lua_tostring(L, 1);
      p.dport_http = lua_tostring(L, 2);
      p.dport_https = lua_tostring(L, 3);

      get_luafile(L)->addProxy(p);

      return _luasuccess(L);
   }


   // -----------------------------------------------------------------------------------------------------------------------
   

   int drun(lua_State *L, bool returnOutput, bool returnExit)
   {
      luafile * lf = get_luafile(L);
      CommandLine operation;


      if (lua_isstring(L, 1) == 1)
      {
         operation.command = lua_tostring(L, 1);
         for (int i = 2; i <= lua_gettop(L); ++i)
            operation.args.push_back(lf->getServiceVars()->substitute(lua_tostring(L, i)));
      }
      else if (lua_istable(L, 1) == 1)
      {
         int t = 1;
         lua_pushnil(L); // we want the first key.
         while (lua_next(L, t) != 0)
         {
            drunner_assert(lua_isnumber(L, -2), "Table keys aren't numbers");
            drunner_assert(lua_isstring(L, -1), "Table value isn't a string");
            //int i = (int)lua_tonumber(L, -2); // key
            std::string s = lua_tostring(L, -1); // value
            if (operation.command.length() == 0)
               operation.command = s;
            else
               operation.args.push_back(s);
         }
      }
      else
         fatal("Unrecognised argument type passed to drun family.");

      Poco::Path runin = lf->getPathdService();
      std::string out;
      int r = utils::runcommand_stream(operation, returnOutput ? kOSuppressed : kORaw, runin, lf->getServiceVars()->getAll(), &out);

      int rcount = 0;
      if (returnOutput)
      {
         Poco::trimInPlace(out);
         lua_pushstring(L, out.c_str());
         ++rcount;
      }
      if (returnExit)
      {
         lua_pushinteger(L, r);
         ++rcount;
      }

      return rcount;
   }


   // -----------------------------------------------------------------------------------------------------------------------

   extern "C" int l_drun(lua_State *L)
   {
      if (lua_gettop(L) < 1)
         return luaL_error(L, "Expected at least one argument: drun( command,  arg1, arg2, ... )");

      return drun(L, false, true);
   }


   // -----------------------------------------------------------------------------------------------------------------------

   extern "C" int l_drun_output(lua_State *L)
   {
      if (lua_gettop(L) < 1)
         return luaL_error(L, "Expected at least one argument: drun_output( command,  arg1, arg2, ... )");

      return drun(L, true, false);
   }

   // -----------------------------------------------------------------------------------------------------------------------

   extern "C" int l_drun_outputexit(lua_State *L)
   {
      if (lua_gettop(L) < 1)
         return luaL_error(L, "Expected at least one argument: drun_outputexit( command,  arg1, arg2, ... )");

      return drun(L, true, true);
   }

   // -----------------------------------------------------------------------------------------------------------------------

   extern "C" int l_dstop(lua_State *L)
   {
      if (lua_gettop(L) != 1)
         return luaL_error(L, "Expected exactly one argument (the container name to stop) for dstop.");
      drunner_assert(lua_isstring(L, 1), "container name must be a string.");
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

      drunner_assert(lua_isstring(L, 1), "String expected as argument.");
      luafile * lf = get_luafile(L);
      std::string s = lf->getServiceVars()->substitute(lua_tostring(L, 1));
      lua_pushstring(L, s.c_str());
      return 1; // one argument to return.
   }

   // -----------------------------------------------------------------------------------------------------------------------

   extern "C" int l_dconfig_get(lua_State *L)
   {
      if (lua_gettop(L) != 1)
         return luaL_error(L, "Expected exactly one argument (the variable name) for dconfig_get.");

      drunner_assert(lua_isstring(L, 1), "String expected as argument.");
      std::string s = lua_tostring(L, 1);
      luafile *lf = get_luafile(L);
      std::string v = lf->getServiceVars()->getVal(s);
      lua_pushstring(L, v.c_str());
      return 1; // one arg to return.
   }

   // -----------------------------------------------------------------------------------------------------------------------

   extern "C" int l_dconfig_set(lua_State *L)
   {
      if (lua_gettop(L) != 2)
         return luaL_error(L, "Expected exactly two arguments (the variable name and value) for dconfig_set.");
      luafile *lf = get_luafile(L);

      drunner_assert(lua_isstring(L, 1), "String expected as argument.");
      drunner_assert(lua_isstring(L, 2), "String expected as argument.");
      std::string s = lua_tostring(L, 1);
      std::string v = lua_tostring(L, 2);

      cResult r = lf->getServiceVars()->setVal(s, v);

      if (r == kRError)
         logmsg(kLWARN, "Failed to set " + s + " to " + v+":\n "+r.what());

      lua_pushinteger(L, r);
      return 1;
   }

   // -----------------------------------------------------------------------------------------------------------------------

   extern "C" int l_dsplit(lua_State *L)
   {
      if (lua_gettop(L)!=1)
         return luaL_error(L, "Expected exactly one argument (the string to split) for dsplit.");

      drunner_assert(lua_isstring(L, 1), "String expected as argument.");
      std::string s = lua_tostring(L, 1);
      std::vector<std::string> tok;

      std::string elem;
      for (size_t i = 0; i<s.length(); ++i) {
         switch (s[i])
         {
         case ' ':
            tok.push_back(elem);
            elem.clear();
            break;

         case '\\':
            ++i;
            drunner_assert(i < s.length(), "Incomplete escape sequence.");
            elem += s[i];
            break;

         case '"':
            ++i;
            while (i < s.length() && s[i] != '"') { elem += s[i]; ++i; }
            break;

         default:
            elem += s[i];
         }
      }
      if (elem.length() > 0)
         tok.push_back(elem);

      lua_createtable(L, tok.size(),0);
      int top = lua_gettop(L);

      for (unsigned int i = 0; i < tok.size(); ++i)
      {
         lua_pushnumber(L, i);
         lua_pushstring(L, tok[i].c_str());
         lua_settable(L, top);
      }

      return 1;
   }

   // -----------------------------------------------------------------------------------------------------------------------

}
