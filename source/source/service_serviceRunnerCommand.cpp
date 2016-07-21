#include "service.h"
#include "globalcontext.h"
#include "globallogger.h"
#include "utils.h"
#include "utils_docker.h"

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

cResult service::_dstop(const std::vector<std::string> & args) const
{
   if (args.size() != 2)
      fatal("dstop requires exactly one argument: the docker container to stop and remove.");
   std::string cname = args[1];
   if (utils_docker::dockerContainerExists(cname))
   {
      std::string out;
      std::vector<std::string> args = { "stop", cname };
      if (utils::runcommand("docker", args, out, utils::RC_LogCmd) != 0)
         return kRError;
      args[0] = "rm";
      if (utils::runcommand("docker", args, out, utils::RC_LogCmd) != 0)
         return kRError;
      return kRSuccess;
   }
   return kRNoChange;
}


cResult service::_handleStandardCommands(std::string command, const std::vector<std::string> & args, bool & processed) const
{
   processed = true;

   for (const auto & y : mServiceYml.getCommands())
      if (utils::stringisame(y.name, command))
         return _runserviceRunnerCommand(y, args);  // link to another command.
      
   // check other commands.
   {
      using namespace utils;
      switch (str2int(command.c_str()))
      {
      case str2int("dstop"):
         return _dstop(args);
         break;
      default:
         break;
      }
   }
   processed = false;
   return kRNoChange;
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
      if (utils::stringisame(y.name, servicecommand) == 0)
         return _runserviceRunnerCommand(y, args);

   return kRNotImplemented;
}
