#include "service_lua.h"
#include "dassert.h"

// -----------------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------
namespace servicelua
{

   void luafile::_register_lua_cfuncs()
   {
      lua_pushcfunction(L, l_addconfig);   // see also http://stackoverflow.com/questions/2907221/get-the-lua-command-when-a-c-function-is-called
      lua_setglobal(L, "addconfig");

      lua_pushcfunction(L, l_addvolume);
      lua_setglobal(L, "addvolume");

      lua_pushcfunction(L, l_addcontainer);
      lua_setglobal(L, "addcontainer");

      lua_pushcfunction(L, l_drun);
      lua_setglobal(L, "drun");
   }

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

      get_luafile(L)->addConfiguration(c);

      cResult rval = kRSuccess;
      lua_pushinteger(L, rval);
      return 1; // one argument to return.
   }


   // -----------------------------------------------------------------------------------------------------------------------


   extern "C" int l_addvolume(lua_State *L)
   {
      if (lua_gettop(L) != 1)
         return luaL_error(L, "Expected exactly one argument (the docker container to stop) for addvolume.");
      std::string cname = lua_tostring(L, 1); // first argument. http://stackoverflow.com/questions/29449296/extending-lua-check-number-of-parameters-passed-to-a-function

      cResult rval = kRSuccess;
      lua_pushinteger(L, rval);
      return 1; // one argument to return.
   }


   // -----------------------------------------------------------------------------------------------------------------------


   extern "C" int l_addcontainer(lua_State *L)
   {
      if (lua_gettop(L) != 1)
         return luaL_error(L, "Expected exactly one argument (the name of the container) for addcontainer.");
      std::string cname = lua_tostring(L, 1); // first argument. http://stackoverflow.com/questions/29449296/extending-lua-check-number-of-parameters-passed-to-a-function

      get_luafile(L)->addContainer(cname);

      cResult rval = kRSuccess;
      lua_pushinteger(L, rval);
      return 1; // one argument to return. Stack auto-balances.
   }

   // -----------------------------------------------------------------------------------------------------------------------

   extern "C" int l_drun(lua_State *L)
   {
      if (lua_gettop(L) != 1)
         return luaL_error(L, "Expected exactly one argument (the command to run) for drun.");
      std::string command = lua_tostring(L, 1);
      
      luafile * lf = get_luafile(L);
      std::string subcmd = lf->getVariables().substitute(command);

      CommandLine operation;
      utils::split_in_args(subcmd, operation);

      Poco::Path runin = lf->getPathdService();

      utils::runcommand_stream(operation, kORaw, runin, lf->getVariables().getEnv());

      cResult rval = kRSuccess;
      lua_pushinteger(L, rval);
      return 1; // one argument to return. Stack auto-balances.
   }

}
