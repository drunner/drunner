#include "service.h"
#include "globalcontext.h"
#include "globallogger.h"
#include "utils.h"
#include "utils_docker.h"

// ---------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------

cResult service::_launchCommandLine(const CommandLine & operation) const
{
   int result = utils::runcommand_stream(
      operation,
      GlobalContext::getParams()->serviceCmdMode(),
      getPathdRunner(),
      mServiceCfg.getVariables().getEnv()
   );
   return (result == 0 ? kRSuccess : kRError);
}

cResult service::_dstop(const CommandLine & operation) const
{
   if (operation.args.size() != 1)
      fatal("dstop requires exactly one argument: the docker container to stop and remove.");
   std::string cname = operation.args[0];
   if (utils_docker::dockerContainerExists(cname))
   {
      logmsg(kLDEBUG, "Stopping and removing container " + cname);
      std::string out;
      CommandLine cl("docker", { "stop",cname });
      if (utils::runcommand(cl, out, utils::kRC_LogCmd) != 0)
         return kRError;
      cl.args[0] = "rm";
      if (utils::runcommand(cl, out, utils::kRC_LogCmd) != 0)
         return kRError;
      return kRSuccess;
   }

   logmsg(kLDEBUG, "Container " + cname + " is not running.");
   return kRNoChange;
}


cResult service::_handleStandardCommands(const CommandLine & operation, bool & processed) const
{
   processed = true;

   for (const auto & y : mServiceYml.getCommands())
      if (utils::stringisame(y.name, operation.command))
         return _runserviceRunnerCommand(y, operation);  // link to another command.
      
   // check other commands.
   {
      using namespace utils;
      switch (str2int(operation.command.c_str()))
      {
      case str2int("dstop"):
         return _dstop(operation);
         break;
      default:
         break;
      }
   }
   processed = false;
   return kRNoChange;
}


cResult service::_runserviceRunnerCommand(const serviceyml::CommandDefinition & x, const CommandLine & serviceCmd) const
{
   cResult rval = kRNoChange;

   variables v(mServiceCfg.getVariables());
   for (unsigned int i = 0; i < serviceCmd.args.size(); ++i)
      v.setVal(std::to_string(i), serviceCmd.args[i]);

   // loop through all the operations in the command.
   for (const auto & rawoperation : x.operations)
   {
      // do variable substitution on all the arguments of the operation
      CommandLine operation;
      operation.command = rawoperation.command;
      for (const auto &arg : operation.args)
      {
         if (arg.compare("$@") == 0)
            operation.args.insert(operation.args.end(), serviceCmd.args.begin(), serviceCmd.args.end());
         else
            operation.args.push_back(v.substitute(arg));
      }

      std::string clog = operation.command;
      for (const auto & arg : operation.args)
         clog += " " + arg;
      logmsg(kLDEBUG, "Running command " + clog);


      bool processed = false;
      // see if it's a standard command (e.g. link to another command).
      cResult standres = _handleStandardCommands(operation, processed);
      if (processed)
         rval += standres;
      else
      {
         rval += _launchCommandLine(operation);
         if (rval == kRError)
            fatal("Result from command " + operation.command + " was non-zero. Aborting.");
      }
   }
   return rval;
}


cResult service::serviceRunnerCommand(const CommandLine & serviceCmd) const
{
   if (serviceCmd.command.length()==0 || utils::stringisame(serviceCmd.command, "help"))
   { // show help
      std::cout << std::endl << mServiceYml.getHelp() << std::endl;
      return kRSuccess;
   }

   std::ostringstream oss(serviceCmd.command);
   for (const auto & x : serviceCmd.args) oss << " " << x;
   logmsg(kLDEBUG, "serviceRunner - cli args are" + oss.str());

   // find the command in our command list and run it.
   for (const auto & y : mServiceYml.getCommands())
      if (utils::stringisame(y.name, serviceCmd.command) == 0)
         return _runserviceRunnerCommand(y, serviceCmd);

   return kRNotImplemented;
}
