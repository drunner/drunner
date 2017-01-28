#include <sstream>
#include <algorithm>

#include <Poco/String.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#include "service.h"
#include "globallogger.h"
#include "utils.h"
#include "globalcontext.h"
#include "drunner_paths.h"
#include "service_lua.h"
#include "dassert.h"

// --------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------


service::service(std::string servicename) : 
   servicePaths(servicename),
   mServiceVars(servicename)
{
}

cResult service::servicecmd()
{
   const params & p(*GlobalContext::getParams());
   drunner_assert(p.numArgs() > 0, "servicecmd requires an argument..."); // should never have 0 args to servicecmd!
   drunner_assert(0==Poco::icompare(p.getArg(0), mName), "First argument should be service name.");

   // args are e.g. minecraft import james.backup
   // CommandLine will be  import james.backup
   CommandLine cl;
   cl.command = (p.numArgs()<2 ? "help" : p.getArg(1)); // import
   std::transform(cl.command.begin(), cl.command.end(), cl.command.begin(), ::tolower); // convert to lower case.

   if (p.numArgs() > 2)
      cl.args = std::vector<std::string>(p.getArgs().begin() + 2, p.getArgs().end());


   // handle configure
   if (0 == Poco::icompare(cl.command, "configure"))
      return mServiceVars.handleConfigureCommand(cl);

   // check reserved commands
   if (p.isdrunnerCommand(cl.command) && cl.command != "help")
      fatal(cl.command + " is a reserved word.\nTry:\n drunner " + cl.command + " " + mName);
   if (p.isHook(cl.command))
      fatal(cl.command + " is a reserved word and not available from the comamnd line for " + mName);

   return runLuaFunction(cl);
}

cResult service::runLuaFunction(CommandLine cl)
{
   // handle the command.
   std::ostringstream oss;
   oss << "[" << cl.command << "]";
   for (const auto & x : cl.args) oss << " " << x;
   logmsg(kLDEBUG, "serviceCmd is: " + oss.str());

   servicelua::luafile sl(mServiceVars);
   sl.loadlua();

   cResult rval = sl.runCommand(cl);

   if (rval == kRNotImplemented)
      logmsg(kLERROR, "Command is not implemented by " + mName + ": " + cl.command);

   return rval;
}

const std::string service::getImageName() const
{
   return mServiceVars.getImageName();
}
