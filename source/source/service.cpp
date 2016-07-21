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

cResult service::_handleconfigure(const std::vector<std::string> & cargs)
{ // configure variable. cargs[1]: key=value
   if (cargs.size() == 1)
   { // show current variables.
      logmsg(kLINFO, "Current configuration for " + mName + " is:");
      for (const auto & x : mServiceCfg.getVariables().getEnv())
         for (const auto & y : mServiceYml.getConfigItems())
            if (utils::stringisame(y.name, x.first))
               logmsg(kLINFO, " " + x.first + " = " + x.second);
      return kRSuccess;
   }

   std::string key, val;
   if (cargs.size() == 3)
   {
      key = cargs[1];
      val = cargs[2];
      logmsg(kLINFO, "Setting " + key + " to " + val);
   }
   else
   {
      if (cargs.size() != 2)
         logmsg(kLERROR, "Configuration variables must be given in form key=val, or key val, or set as an environment variable.");
      std::string keyval = cargs[1];
      size_t epos = keyval.find('=');
      if (epos == std::string::npos)
      { // env variable
         key = cargs[1];
         try {
            val = Poco::Environment::get(key);
         }
         catch (const Poco::Exception & e)
         {
            logmsg(kLDEBUG, e.what());
            logmsg(kLERROR, "Configuration variables must be given in form key=val, or key val, or set as an environment variable.");
         }
         logmsg(kLINFO, "Setting " + key + " to value from environment [not logged].");
      }
      else
      { // form key=val.
         if (epos == 0)
            logmsg(kLERROR, "Missing key.");
         if (epos == keyval.length() - 1)
            logmsg(kLERROR, "Missing value.");

         key = keyval.substr(0, epos);
         val = keyval.substr(epos + 1);
         logmsg(kLINFO, "Setting " + key + " to " + val);
      }
   }

   bool set = false;
   for (const auto & y : mServiceYml.getConfigItems())
      if (utils::stringisame(key, y.name))
      {
         mServiceCfg.setSaveVariable(key, val);
         set = true;
      }
   if (!set)
   {
      logmsg(kLERROR, "Unrecognised setting " + key);
      return kRError;
   }
   else
      return kRSuccess;
}

cResult service::servicecmd()
{
   const params & p(*GlobalContext::getParams());
   drunner_assert(p.numArgs() > 0, "servicecmd requires an argument..."); // should never have 0 args to servicecmd!
   drunner_assert(utils::stringisame(p.getArg(0), mName), "First argument should be service name.");

   std::vector<std::string> cargs( p.getArgs().begin() + 1, p.getArgs().end() );

   if (cargs.size() == 0)
   {
      cargs.push_back("help");
      return serviceRunnerCommand(cargs);
   }

   if (utils::stringisame(cargs[0], "configure"))
   {
      _handleconfigure(cargs);
      return kRSuccess;
   }

   cResult rval = kRError;
   std::string command = cargs[0];
   if (p.isdrunnerCommand(command))
      logmsg(kLERROR, command + " is a reserved word.\nTry:\n drunner " + command + " " + mName);
   if (p.isHook(command))
      logmsg(kLERROR, command + " is a reserved word and not available from the comamnd line for " + mName);
      
   // check all required variables are configured.

   // run the command
   servicehook hook(this, "servicecmd", cargs);
   hook.starthook();

   rval = serviceRunnerCommand(cargs);
   if (rval == kRNotImplemented)
      logmsg(kLERROR, "Command is not implemented by " + mName + ": " + command);

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
