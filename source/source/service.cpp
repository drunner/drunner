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

// ---------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------

cResult service::_launchOperation(std::string command, const std::vector<std::string> & args) const
{
   std::string clog = command;
   for (const auto & arg : args)
      clog += " " + arg;
   logmsg(kLDEBUG, "Running command " + clog);

   int result = utils::runcommand_stream(
      command,
      args,
      GlobalContext::getParams()->serviceCmdMode(),
      getPathdRunner(),
      mServiceCfg.getVariables().getEnv()
   );
   return (result == 0 ? kRSuccess : kRError);
}

cResult service::_handleStandardCommands(std::string command, const std::vector<std::string> & args, bool & processed) const
{
   for (const auto & y : mServiceYml.getCommands())
      if (utils::stringisame(y.name, command))
      { // link to another command.
         processed = true;
         return _runserviceRunnerCommand(y, args);
      }

   processed = false;
   return kRNoChange;
   //for (const auto & z : _internalcommands())
}


cResult service::_runserviceRunnerCommand(const serviceyml::CommandLine & x, const std::vector<std::string> & args) const
{
   cResult rval = kRNoChange;

   variables v(mServiceCfg.getVariables());
   for (unsigned int i = 0; i < args.size(); ++i)
      v.setVal(std::to_string(i), args[i]);

   // loop through all the operations in the command.
   for (const auto & operation : x.operations)
   {
      // do variable substitution on all the arguments of the operation
      std::vector<std::string> finalargs;
      for (const auto &arg : operation.args)
      {
         if (arg.compare("$@") == 0)
            finalargs.insert(finalargs.end(), args.begin() + 1, args.end());
         else
            finalargs.push_back(v.substitute(arg));
      }

      bool processed = false;
      // see if it's a standard command (e.g. link to another command).
      cResult standres = _handleStandardCommands(operation.command, finalargs, processed);
      if (processed)
         rval += standres;
      else
      {
         rval += _launchOperation(operation.command, finalargs);
         if (rval == kRError)
            fatal("Result from command " + operation.command + " was non-zero. Aborting.");
      }
   }
   return rval;
}


cResult service::serviceRunnerCommand(const std::vector<std::string> & args) const
{
   if (args.size() < 1 || utils::stringisame(args[0], "help"))
   { // show help
      std::cout << std::endl << mServiceYml.getHelp() << std::endl;
      return kRSuccess;
   }

   std::ostringstream oss;
   for (const auto & x : args) oss << " " << x;
   logmsg(kLDEBUG, "serviceRunner - args are" + oss.str());
      
   std::string servicecommand = args[0];

   // find the command in our command list and run it.
   for (const auto & y : mServiceYml.getCommands())
      if (utils::stringisame(y.name,servicecommand) == 0)
         return _runserviceRunnerCommand(y,args);

   return kRNotImplemented;
}
