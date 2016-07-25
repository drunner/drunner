#include <stdio.h>

#include <Poco/String.h>

#include "lua.hpp"

#include "service.h"
#include "globalcontext.h"
#include "globallogger.h"
#include "utils.h"
#include "utils_docker.h"
#include "drunner_paths.h"
#include "service_paths.h"
#include "dassert.h"


// ---------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------
//
//const service * sCurrentService = NULL;
//
//extern "C" int l_dstop(lua_State *L)
//{
//   drunner_assert(sCurrentService != NULL, "Current Service is NULL - programming error.");
//
//   if (lua_gettop(L) != 1)
//      return luaL_error(L, "Expected exactly one argument (the docker container to stop) for dstop.");
//   std::string cname = lua_tostring(L, 1); // first argument. http://stackoverflow.com/questions/29449296/extending-lua-check-number-of-parameters-passed-to-a-function
//
//   cResult rval; // defaults to kRNoChange.
//   if (utils_docker::dockerContainerExists(cname))
//   {
//      logmsg(kLDEBUG, "Stopping and removing container " + cname);
//      std::string out;
//      CommandLine cl("docker", { "stop",cname });
//      rval += (utils::runcommand(cl, out, utils::kRC_LogCmd) != 0);
//      cl.args[0] = "rm";
//      rval += (utils::runcommand(cl, out, utils::kRC_LogCmd) != 0);
//
//   }
//   else
//      logmsg(kLDEBUG, "Container " + cname + " is not running.");
//
//   lua_pushinteger(L, rval);
//   return 1; // one argument to return.
//}
//
//extern "C" int l_run(lua_State *L)
//{
//   drunner_assert(sCurrentService != NULL, "Current Service is NULL - programming error.");
//
//   if (lua_gettop(L) != 1)
//      return luaL_error(L, "Expected exactly one argument (the command to run as a string) for run.");
//   std::string command = lua_tostring(L, 1); // first argument. 
//
//   cResult rval; // defaults to kRNoChange.
//
//   CommandLine cl;
//   utils::split_in_args(command, cl);
//   cl.logcommand("Run: ");
//
//   servicePaths sp(sCurrentService->getName());
//   rval += utils::runcommand_stream(
//      cl,
//      GlobalContext::getParams()->serviceCmdMode(),
//      sp.getPathdRunner()
//   );
//
//   lua_pushinteger(L, rval);
//   return 1; // one argument to return.
//}


//cResult service::_handleStandardCommands(const CommandLine & operation, bool & processed) const
//{
//   processed = true;
//
//   for (const auto & y : mServiceLua.getCommands())
//      if (utils::stringisame(y.name, operation.command))
//         return _runserviceRunnerCommand(y, operation);  // link to another command.
//      
//   // check other commands.
//   {
//      using namespace utils;
//      switch (str2int(operation.command.c_str()))
//      {
//      case str2int("dstop"):
//         return _dstop(operation);
//         break;
//      default:
//         break;
//      }
//   }
//   processed = false;
//   return kRNoChange;
//}

//
//cResult service::_runserviceRunnerCommand(const CommandLine & serviceCmd) const
//{
//   cResult rval = kRNoChange;
//
//   sCurrentService = this;
//   
//   lua_State * L = luaL_newstate();
//   luaL_openlibs(L);
//
//   lua_pushcfunction(L, l_dstop);   // see also http://stackoverflow.com/questions/2907221/get-the-lua-command-when-a-c-function-is-called
//   lua_setglobal(L, "dstop");
//   lua_pushcfunction(L, l_run);
//   lua_setglobal(L, "run");
//
//   variables v(mServiceVars.getVariables());
//
//   // $0 is script name (serviceCmd)
//   v.setVal(std::to_string(0), serviceCmd.command);
//
//   // $1, $2, .... $n for individual arguments.
//   for (unsigned int i = 0; i < serviceCmd.args.size(); ++i)
//      v.setVal(std::to_string(i+1), serviceCmd.args[i]);
//
//   // $# is number of args.
//   v.setVal("#", std::to_string(serviceCmd.args.size()));
//   
//   // $@ is all args (but not script command).
//   std::ostringstream allargs;
//   for (const auto & sca : serviceCmd.args)
//      allargs << sca << " ";
//   v.setVal("@", Poco::trim(allargs.str()));
//
//   // loop through all the operations in the command.
//   //for (const auto & rawoperation : x.operations)
//   //{
//   //   // do variable substitution on all the arguments of the operation
//   //   std::string lualine(v.substitute(rawoperation));
//   //   logmsg(kLDEBUG, "Running command " + lualine);
//
//   //   int ls = luaL_loadstring(L, lualine.c_str());
//   //   if (!ls)
//   //      ls = lua_pcall(L, 0, LUA_MULTRET, 0);
//
//   //   if (ls)
//   //      logmsg(kLERROR, "Error " + std::string(lua_tostring(L, -1)));
//   //}
//
//   if (L) 
//      lua_close(L);
//
//   sCurrentService = NULL;
//   return rval;
//}
