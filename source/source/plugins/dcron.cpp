#include <time.h>
#include <sstream>
#include <fstream>

#include "dcron.h"
#include "drunner_paths.h"
#include "service_lua.h"
#include "service.h"
#include "timez.h"
#include "globalcontext.h"
#include "dassert.h"

// ----------------------------------------------------------------------------

// load a time_t value from a file.
time_t dcron::_gettimefile(Poco::Path fname)
{
   std::ifstream ifs(fname.toString());
   time_t z;
   ifs >> z;
   ifs.close();
   return z;
}

// save a time_t value to a file.
cResult dcron::_settimefile(time_t t, Poco::Path fname)
{
   // write lockfile.
   std::ofstream ofs(fname.toString());
   ofs << t;
   ofs.close();
   return kRSuccess;
}

// A simple lock file. Expires in 2 hours.
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
         time_t z = dcron::_gettimefile(lockfilepath());
         if (time(NULL) - z < expiretime) // 2 hours
         {
            logmsg(kLINFO, "dcron already running for " + mUniqueName); // (lock file " + lockfilepath().toString() + " exists)");
            return kRNoChange;
         }
         logmsg(kLWARN, "Lock file is stale (>= 2 hours old). Removing it.");
         if (!utils::delfile(lockfilepath()).success())
            return cError("Couldn't remove lockfile.");
      }

   // write lockfile.
   dcron::_settimefile(time(NULL),lockfilepath());

   logdbg("Got dcron lock for " + mUniqueName);
   
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
   static const time_t expiretime = 60 * 60 * 2;
};

// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------

std::string dcron::getName() const
{
   return "dcron";
}

cResult dcron::runCommand(const CommandLine & cl, persistvariables & v) const
{
   if (cl.command != "run")
   {
      showHelp();
      return cError("Only commands supported are 'run' or 'configure'.");
   }

   logdbg("Running dcron.");

   // now run cron.
   return _runcron(cl, v.getVariables());
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
   if (!c.isvalid())
      return false;

   // determine intervals.
   std::istringstream offsetmin_s(c.offsetmin);
   std::istringstream repeatmin_s(c.repeatmin);

   time_t offsetmin, repeatmin;
   offsetmin_s >> offsetmin;
   repeatmin_s >> repeatmin;

   drunner_assert(repeatmin > 0, "Invalid cron entry returned valid.");
    
   // read in last run time from file
   Poco::Path lastrunpath = drunnerPaths::getPath_Settings().setFileName("dcron-"+uniquename + ".lastrun");
   time_t lastrun = 55555;
   if (utils::fileexists(lastrunpath))
      lastrun = _gettimefile(lastrunpath);

   // calculate the time segments.
   time_t x = (time(NULL) / 60 - offsetmin) / repeatmin;
   time_t z = (lastrun / 60 - offsetmin) / repeatmin;

   if (x <= z) // same time segment (or cron job was last run in future - clock wound back).
   {
      time_t delta = time(NULL) / 60 - lastrun / 60;
      time_t target = (x + 1)*repeatmin + offsetmin - lastrun / 60;
      logdbg("Not running: " + std::to_string(delta) + " minutes elapsed, next run at " + std::to_string(target));
      return false;
   }

   // we're into a new time segment, update the file on disk to the current time and return true (run the job!).
   _settimefile(time(NULL), lastrunpath);
   return true;
}

cResult dcron::_runcron(const CommandLine & cl, const variables & v) const
{
   cResult r;
   // cycle through all dServices testing if cron jobs need to run.

   std::vector<std::string> services;
   utils::getAllServices(services);

   for (auto const & s : services)
   {
      service svc(s);
      for (auto ce : svc.getServiceLua().getCronEntries())
         r += _runcron(svc, ce, v);
   } // services

   // plugins (e.g. dbackup)
   std::vector<std::string> plugins;
   GlobalContext::getPlugins()->getPluginNames(plugins);
   for (auto p : plugins)
   {
      servicelua::CronEntry ce = GlobalContext::getPlugins()->getCronJob(p);
      if (_runjob("plugin-" + p, ce))
         r += GlobalContext::getPlugins()->runCron(p);
   }

   return r;
}

cResult dcron::_runcron(service & svc, servicelua::CronEntry & ce, const variables & v) const
{
   std::string uniquename = svc.getName() + "-" + ce.function;
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

      return kRSuccess;
   } // lock

   return kRNoChange; // couldn't get lock - cron job already running.
}
