#include "dcron.h"
#include "drunner_paths.h"

dcron::dcron()
{
   addConfig("LastRun", "The last time dcron was run [automatically set].", "", kCF_string, false,false);
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
      return cError("Only command supported is 'run'");
   }

   logdbg("Running dcron.");
   return _runcron(cl, v);
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
   Poco::Path target = drunnerPaths::getPath_Settings().setFileName("dcron.cfg");
   return target;
}

cResult dcron::_runcron(const CommandLine & cl, const variables & v) const
{
   return cResult();
}
