#include <sstream>
#include <Poco/Environment.h>

//#include <Poco/Process.h>
//#include <Poco/Pipe.h>
//#include <Poco/StreamCopier.h>
//#include <Poco/PipeStream.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#include "service.h"
#include "globallogger.h"
#include "utils.h"
#include "servicehook.h"
#include "sh_servicevars.h"
#include "globalcontext.h"
#include "drunner_paths.h"
#include "service_yml.h"
#include "dassert.h"

// --------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------


service::service(std::string servicename) : 
   servicePaths(servicename),
   mServiceCfg(servicePaths(servicename).getPathServiceConfig()),
   mServiceYml(servicePaths(servicename).getPathServiceYml())
{
   if (mServiceCfg.loadconfig() != kRSuccess)
      fatal("Could not load service configuration: " + getPathServiceConfig().toString());

   if (mServiceYml.loadyml(mServiceCfg.getVariables()) != kRSuccess)
      fatal("Could not load service yml: " + getPathServiceYml().toString());
   mImageName = mServiceCfg.getImageName();
   poco_assert(mImageName.length() > 0);
}

servicePaths::servicePaths(const std::string & servicename) :
   mName(servicename)
{
}

cResult service::_handleconfigure(const CommandLine & cl)
{ // configure variable. cargs[0]: key=value
   if (cl.args.size()==0)
   { // show current variables.
      logmsg(kLINFO, "Current configuration for " + mName + " is:");
      for (const auto & x : mServiceCfg.getVariables().getEnv())
         for (const auto & y : mServiceYml.getConfigItems())
            if (utils::stringisame(y.name, x.first))
               logmsg(kLINFO, " " + x.first + " = " + x.second);
      return kRSuccess;
   }

   for (const auto & kv : cl.args)
   {
      std::string key, val;
      size_t epos = kv.find('=');
      if (epos == std::string::npos)
      { // env variable
         try {
            key = kv;
            val = Poco::Environment::get(key);
         }
         catch (const Poco::Exception & e)
         {
            logmsg(kLDEBUG, e.what());
            logmsg(kLERROR, "Configuration variables must be given in form key=val or set as an environment variable.");
         }
         logmsg(kLINFO, "Setting " + key + " to value from environment [not logged].");
      }
      else
      { // form key=val.
         if (epos == 0)
            logmsg(kLERROR, "Missing key.");
         if (epos == kv.length() - 1)
            logmsg(kLERROR, "Missing value.");

         key = kv.substr(0, epos);
         val = kv.substr(epos + 1);
         logmsg(kLINFO, "Setting " + key + " to " + val);
      }

      // find the corresponding configuration definition and set the variable.
      for (const auto & y : mServiceYml.getConfigItems())
         if (utils::stringisame(key, y.name))
         {
            // TODO: validate the value to be set against hte configuration definition! (e.g. if a port, is it valid?)
            mServiceCfg.setSaveVariable(key, val);
         }
      if (!mServiceCfg.getVariables().hasKey(key))
         fatal("Unrecognised setting " + key);
   }
   return kRSuccess;
}

cResult service::servicecmd()
{
   const params & p(*GlobalContext::getParams());
   drunner_assert(p.numArgs() > 0, "servicecmd requires an argument..."); // should never have 0 args to servicecmd!
   drunner_assert(utils::stringisame(p.getArg(0), mName), "First argument should be service name.");

   if (p.getArgs().size() < 2)
      return serviceRunnerCommand( CommandLine("help") );

   // e.g. minecraft import james.backup
   CommandLine cl;
   cl.command = p.getArg(1); // import
   if (p.getArgs().size() > 2)
      cl.args = std::vector<std::string>(p.getArgs().begin() + 2, p.getArgs().end());

   if (utils::stringisame(cl.command, "configure"))
   {
      _handleconfigure(cl);
      return kRSuccess;
   }

   cResult rval = kRError;
   if (p.isdrunnerCommand(cl.command))
      logmsg(kLERROR, cl.command + " is a reserved word.\nTry:\n drunner " + cl.command + " " + mName);
   if (p.isHook(cl.command))
      logmsg(kLERROR, cl.command + " is a reserved word and not available from the comamnd line for " + mName);
      
   // check all required variables are configured.

   // run the command
   servicehook hook(this, "servicecmd", p.getArgs());
   hook.starthook();

   rval = serviceRunnerCommand(cl);
   if (rval == kRNotImplemented)
      logmsg(kLERROR, "Command is not implemented by " + mName + ": " + cl.command);

   hook.endhook();
   return rval;
}

const std::string service::getImageName() const
{
   return mImageName;
}

int service::status()
{
   servicehook hook(this, "status");
   hook.starthook();

   if (!utils::fileexists(getPath()))
   {
      logmsg(kLINFO, getName() + " is not installed.");
      hook.endhook();
      return 1;
   }
   logmsg(kLINFO, getName() + " is installed and valid.");
   hook.endhook();
   return 0;
}
