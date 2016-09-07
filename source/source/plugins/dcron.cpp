#include <time.h>
#include <sstream>

#include "dcron.h"
#include "drunner_paths.h"
#include "service_lua.h"
#include "service.h"

dcron::dcron()
{
   addConfig("LastRun", "The last time dcron was run [automatically set].", "55555", kCF_string, false,false);
}

std::string dcron::getName() const
{
   return "dcron";
}

cResult dcron::runCommand(const CommandLine & cl, const variables & v) const
{
   if (cl.command != "run")
   {
      showHelp();
      return cError("Only commands supported are 'run' or 'configure'.");
   }

   logdbg("Running dcron.");

   std::istringstream s(v.getVal("LastRun"));
   time_t t;
   s >> t;
   cResult r = _runcron(cl, v, t);

   // update stored time to now.
   logdbg("Updating LastRun time.");
   r += setVariable("LastRun", std::to_string(time(NULL)));
   return r;
}

cResult dcron::runHook(std::string hook, std::vector<std::string> hookparams, const servicelua::luafile * lf, const serviceVars * sv) const
{
   return kRNoChange; // nothing to do.
}

cResult dcron::showHelp() const
{
   std::string help = R"EOF(
NAME
   dcron

DESCRIPTION
   A dRunner plugin to provide auto cron tasks for dServices.

SYNOPSIS
   dcron run

SYSTEM CONFIGRUATION
   Add the following line to the user's crontab:

* * * * * .drunner/bin/dcron run > /dev/null 2>&1

TODO: use logrotate like here: http://askubuntu.com/questions/405663/configuring-logrotate-without-root-access-per-user-log-rotation

)EOF";

   logmsg(kLINFO, help);

   return kRSuccess;
}

Poco::Path dcron::configurationFilePath() const
{
   Poco::Path target = drunnerPaths::getPath_Settings().setFileName("dcron.json");
   return target;
}

cResult dcron::_runcron(const CommandLine & cl, const variables & v, time_t lasttime) const
{
   // cycle through all dServices testing if cron jobs need to run.

   std::vector<std::string> services;
   utils::getAllServices(services);

   for (auto const & s : services)
   {
      service svc(s);

      for (auto const & ce : svc.getServiceLua().getCronEntries())
      {
         std::istringstream offsetmin_s(svc.getServiceVars().substitute(ce.offsetmin));
         std::istringstream repeatmin_s(svc.getServiceVars().substitute(ce.repeatmin));

         time_t offsetmin, repeatmin;
         offsetmin_s >> offsetmin;
         repeatmin_s >> repeatmin;

         time_t x = (time(NULL)/60 - offsetmin) / repeatmin;
         time_t z = (lasttime/60 - offsetmin) / repeatmin;

         if (x > z)
         { // time interval has changed, run the command.
            logdbg("Invoking cron function: " + ce.function);
            CommandLine cl;
            cl.command = ce.function;
            svc.runLuaFunction(cl);
         }
      }
   }

   return cResult();
}
