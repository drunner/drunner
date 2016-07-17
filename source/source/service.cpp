#include <sstream>

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

cResult service::servicecmd()
{
   const params & p(*GlobalContext::getParams());
   poco_assert(p.numArgs() > 0); // should never have 0 args to servicecmd!

   std::vector<std::string> cargs( p.getArgs().begin() + 1, p.getArgs().end() );

   if (p.numArgs() < 2 || utils::stringisame(mName, "help"))
   {
      cargs.push_back("help");
      serviceRunnerCommand(cargs);
      return kRSuccess;
   }

   std::string command = p.getArgs()[1];
   std::string reservedwords = " install backupstart backupend backup restore update enter uninstall obliterate recover ";
   if (utils::findStringIC(reservedwords, " " + command + " ")) // spaces are to ensure whole word match.
   {
      if (command != "enter")
         logmsg(kLERROR, command + " is a reserved word. You might want  drunner " + command + " " + mName + "  instead.");
      enter(); // uses execl so never returns.
      logmsg(kLERROR, "Should never get here. Enter command shouldn't return.");
   }

   servicehook hook(this, "servicecmd", cargs);
   hook.starthook();

   cResult rval( serviceRunnerCommand(cargs) );

   hook.endhook();

   return rval;
}


const std::string service::getImageName() const
{
   return mImageName;
}

void service::enter()
{
   logmsg(kLERROR, "Enter is not implemented.");
//#else
//   servicehook hook(this, "enter");
//   hook.starthook();
//
//   execl(getPathServiceRunner().toString().c_str(), "servicerunner", "enter", NULL);
//#endif
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


cResult service::serviceRunnerCommand(const std::vector<std::string> & args) const
{
   std::ostringstream oss;
   for (const auto & x : args) oss << " " << x;
   logmsg(kLDEBUG, "serviceRunner - args are" + oss.str());

   if (args.size() < 1)
      return serviceRunnerCommand({ "help" });
   else
   {
      variables v(mServiceCfg.getVariables());
      for (unsigned int i = 0; i < args.size(); ++i)
         v.setVal(std::to_string(i), args[i]);

      std::string servicecommand = args[0];

      for (const auto & x : mServiceYml.getCommands())
         if (x.name.compare(servicecommand)==0)
         for (const auto & op : x.operations)
         {
            std::string clog = op.command;
            std::vector<std::string> finalargs;
            for (const auto &arg : op.args)
            {
               std::string argument = v.substitute(arg);
               if (argument.compare("$@") == 0)
                  finalargs.insert(finalargs.end(), args.begin()+1, args.end());
               else
                  finalargs.push_back(argument);
               clog += " " + argument;
            }
            logmsg(kLDEBUG, "Running command " + clog);

            if (utils::runcommand_stream(op.command, finalargs, true, getPathdRunner(), v.getEnv()) != 0)
               return kRError;
            return kRSuccess;
         }
   }
   //logmsg(kLDEBUG, "Command is not implemented by "+mName+": " + args[0]);
   return kRNotImplemented;
}
