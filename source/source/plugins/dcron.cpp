#include <time.h>
#include <sstream>
#include <fstream>

#include "dcron.h"
#include "drunner_paths.h"
#include "service_lua.h"
#include "service.h"
#include "timez.h"

// ----------------------------------------------------------------------------


class lockfile
{
public:
   lockfile(std::string uniquename) : mUniqueName(uniquename)
   {
   }

   cResult getlock()
   {
      // check lockfile.
      if (utils::fileexists(lockfilepath()))
      {
         std::ifstream ifs(lockfilepath().toString());
         time_t z;
         ifs >> z;
         ifs.close();
         if (time(NULL) - z < 60 * 60 * 2) // 2 hours
         {
            logmsg(kLINFO, "dcron already running for "+mUniqueName+" (lock file " + lockfilepath().toString() + " exists)");
            return kRNoChange;
         }
         logmsg(kLWARN, "Lock file is stale (>= 2 hours old). Removing it.");
         if (!utils::delfile(lockfilepath()).success())
            return cError("Couldn't remove lockfile.");
      }

   // write lockfile.
   std::ofstream ofs(lockfilepath().toString());
   ofs << time(NULL);
   ofs.close();

   logdbg("Got dcron lock for " + mUniqueName + " ("+lockfilepath().toString()+")");

   return kRSuccess;
   }

   cResult freelock()
   {
      if (!utils::delfile(lockfilepath()).success())
         return cError("Couldn't remove lockfile.");
      return kRSuccess;
   }

private:
   Poco::Path lockfilepath()
   {
      return drunnerPaths::getPath_Temp().setFileName(mUniqueName + ".lockfile");
   }
   std::string mUniqueName;
};

// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------


dcron::dcron()
{
}

dcron::~dcron()
{
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

   // now run cron.
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
   Poco::Path target = drunnerPaths::getPath_Settings().setFileName("dcron.json");
   return target;
}

bool dcron::_runjob(std::string uniquename, const servicelua::CronEntry & c) const
{
   Poco::Path lastrunpath = drunnerPaths::getPath_Settings().setFileName("dcron-"+uniquename + ".lastrun");
   time_t lastrun = 55555;
   if (utils::fileexists(lastrunpath))
   {
      std::ifstream ifs(lastrunpath.toString());
      ifs >> lastrun;
      ifs.close();
   }

   std::istringstream offsetmin_s(c.offsetmin);
   std::istringstream repeatmin_s(c.repeatmin);

   time_t offsetmin, repeatmin;
   offsetmin_s >> offsetmin;
   repeatmin_s >> repeatmin;

   time_t x = (time(NULL) / 60 - offsetmin) / repeatmin;
   time_t z = (lastrun / 60 - offsetmin) / repeatmin;

   if (x > z) // different time segment.
   {
      std::ofstream ofs(lastrunpath.toString());
      ofs << time(NULL);
      ofs.close();

      return true;
   }
   return false;
}

cResult dcron::_runcron(const CommandLine & cl, const variables & v) const
{
   // cycle through all dServices testing if cron jobs need to run.

   std::vector<std::string> services;
   utils::getAllServices(services);

   for (auto const & s : services)
   {
      service svc(s);

      for (auto ce : svc.getServiceLua().getCronEntries())
      {
         std::string uniquename = s + "-" + ce.function;
         lockfile lockf(uniquename);
         cResult lfr = lockf.getlock();
         if (lfr.success())
         {
            ce.offsetmin = svc.getServiceVars().substitute(ce.offsetmin);
            ce.repeatmin = svc.getServiceVars().substitute(ce.repeatmin);

            if (_runjob(uniquename, ce))
            { // time interval has changed, run the command.
               logdbg("Invoking cron function: " + ce.function);
               CommandLine cl;
               cl.command = ce.function;
               svc.runLuaFunction(cl);
            }

            lockf.freelock();
         } // lock
      } // dcron entries
   } // services

   return cResult();
}
