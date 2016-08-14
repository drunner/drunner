#include <sstream>
#include <algorithm>

#include <Poco/String.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#include "service.h"
#include "globallogger.h"
#include "utils.h"
#include "servicehook.h"
#include "globalcontext.h"
#include "drunner_paths.h"
#include "service_lua.h"
#include "dassert.h"

// --------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------


service::service(std::string servicename) : 
   servicePaths(servicename),
   mServiceLua(servicename)
{
   if (mServiceLua.loadlua() != kRSuccess)
      fatal("Could not load service.lua: " + getPathServiceLua().toString());

   // make_unique is C++14.
   mServiceVarsPtr = std::unique_ptr<serviceVars>(new serviceVars(servicename, mServiceLua.getConfigItems()));

   if (mServiceVarsPtr->loadvariables() != kRSuccess)
      fatal("Could not load service varialbes.");
   mImageName = mServiceVarsPtr->getImageName();
   drunner_assert(mImageName.length() > 0,"IMAGENAME not set in service variables.");
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
   {
      servicehook hook(getName(), "configure", cl.args);
      hook.starthook();
      cResult rval= mServiceLua.runCommand(cl, mServiceVarsPtr.get()); // no hooks.
      hook.endhook();
      return rval;
   }

   // check reserved commands
   if (p.isdrunnerCommand(cl.command) && cl.command != "help")
      fatal(cl.command + " is a reserved word.\nTry:\n drunner " + cl.command + " " + mName);
   if (p.isHook(cl.command))
      fatal(cl.command + " is a reserved word and not available from the comamnd line for " + mName);

   // check all required variables are configured.
   for (const auto & var : mServiceLua.getConfigItems())
      if (var.required)
         if (!mServiceVarsPtr->hasKey(var.name))
            fatal("A required configuration variable " + var.name + " has not yet been set.");

   // handle the command.
   std::ostringstream oss;
   oss << "[" << cl.command << "]";
   for (const auto & x : cl.args) oss << " " << x;
   logmsg(kLDEBUG, "serviceCmd is: " + oss.str());

   cResult rval = mServiceLua.runCommand(cl,mServiceVarsPtr.get());

   if (rval == kRNotImplemented)
      logmsg(kLERROR, "Command is not implemented by " + mName + ": " + cl.command);

   return rval;
}

const std::string service::getImageName() const
{
   return mImageName;
}

int service::status()
{
   servicehook hook(mName, "status");
   hook.starthook();

   if (!utils::fileexists(getPathdService()))
   {
      logmsg(kLINFO, getName() + " is not installed.");
      hook.endhook();
      return 1;
   }
   logmsg(kLINFO, getName() + " is installed and valid.");
   hook.endhook();
   return 0;
}

