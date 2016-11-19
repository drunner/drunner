#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <Poco/File.h>
#include <Poco/String.h>
#include <cereal/archives/json.hpp>
#include <math.h>

#include "utils.h"
#include "globallogger.h"
#include "globalcontext.h"
#include "timez.h"
#include "service.h"

#include "dbackup.h"
#include "drunner_paths.h"
#include "dassert.h"

// -----------------------------------------------------------------------------------------------------------

dbackup::dbackup() 
{
   addConfig("BACKUPPATH", "The path to save backups into.", "", kCF_string, true, true);
   addConfig("MAXDAYS", "The maximum number of days to keep backups for.", "90", kCF_string, true, true);
   addConfig("MAXBACKUPS", "The maximum number of backup sets to keep.", "20", kCF_string, true, true);
   addConfig("ALWAYSKEEP", "Always keep this many of the most recent backups.", "3", kCF_string, true, true);

   addConfig("AUTOBACKUP", "Run the backups automatically using dcron once per day", "false", kCF_bool, true, true);

   // system set (not user settable).
   addConfig("DISABLEDSERVICES", "Services that have been disabled (base64 encoded).", "", kCF_string, false, false);
}

std::string dbackup::getName() const
{
   return std::string("dbackup");
}

Poco::Path dbackup::configurationFilePath() const
{
   return drunnerPaths::getPath_Settings().setFileName("dbackup.json");
}

servicelua::CronEntry dbackup::getCron() const
{
   servicelua::CronEntry ce;

   persistvariables p(getPersistVariables());

   if (p.getBool("AUTOBACKUP"))
   { // could make these user settable if there was a need.
      ce.offsetmin = "180"; // 3 am
      ce.repeatmin = "1440"; // 24 hours
   }

   return ce;
}

cResult dbackup::runCron() const
{
   return _run(getPersistVariables());
}

cResult dbackup::runCommand(const CommandLine & cl, persistvariables & v) const
{
   switch (s2i(cl.command.c_str()))
   {
   case s2i("exclude") :
      if (cl.args.size() == 0)
         fatal("Usage:  dbackup exclude SERVICENAME");
      return _exclude(cl.args[0],v);

   case s2i("include"):
      if (cl.args.size() == 0)
         fatal("Usage:  dbackup include SERVICENAME");
      return _include(cl.args[0],v);

   case s2i("run"):
      return _run(v);

   case s2i("info"):
   case s2i("list"):
      return _info(v);

   default:
      return cError("Unrecognised command " + cl.command);
   }
}

cResult dbackup::runHook(std::string hook, std::vector<std::string> hookparams, const servicelua::luafile * lf, const serviceVars * sv) const
{
   return kRNoChange;
}


// -----------------------------------------------------------------------------------------------------------

cResult dbackup::_include(std::string servicename, persistvariables &v) const
{
   std::vector<std::string> excluded;
   _getExcluded(excluded, v);
   auto f = std::find(excluded.begin(), excluded.end(), servicename);
   if (f == excluded.end())
      return kRNoChange; // not in excluded list, so automatically included.

   excluded.erase(f);
   _setExcluded(excluded, v);
   return v.savevariables();
}

cResult dbackup::_exclude(std::string servicename, persistvariables &v) const
{
   std::vector<std::string> excluded;
   _getExcluded(excluded, v);
   auto f = std::find(excluded.begin(), excluded.end(), servicename);
   if (f != excluded.end())
      return kRNoChange; // already excluded

   excluded.push_back(servicename);
   _setExcluded(excluded, v);
   return v.savevariables();
}

cResult dbackup::_run(const persistvariables &v) const
{
   if (!v.checkRequired().success())
      return cError("Please configure dbackup before running.");

   Poco::Path p = _getPath(v);
   p.makeDirectory();

   if (!utils::fileexists(p))
      fatal("Configure dbackup before running.");

   std::vector<std::string> services;
   utils::getAllServices(services);

   p.pushDirectory(timeutils::getDateTimeStr());
   utils::makedirectory(p, S_700);

   logmsg(kLINFO, "Backing up services.");
   std::vector<std::string> excludedservices;
   _getExcluded(excludedservices, v);

   for (auto const & s : services)
   {
      if (std::find(excludedservices.begin(), excludedservices.end(), s) == excludedservices.end()) // not excluded.
      {
         // backup service s.
         p.setFileName(timeutils::getArchiveName(s));

         logmsg(kLINFO, "----------------- " + s + " --------------------");
         service svc(s);
         svc.backup(p.toString());
      }
   }

   return _purgeOldBackups(v);
}

Poco::Path dbackup::_getPath(const persistvariables & v) const
{
   Poco::Path p(v.getVal("BACKUPPATH"));
   p.makeDirectory();
   if (!p.isAbsolute())
      p.makeAbsolute();
   return p;
}

void dbackup::_getExcluded(std::vector<std::string>& vs, const persistvariables &v) const
{
   std::string ds = v.getVal("DISABLEDSERVICES");
   utils::str2vecstr(ds, vs);
}

void dbackup::_setExcluded(const std::vector<std::string>& vs, persistvariables &v) const
{
   std::string s = utils::vecstr2str(vs);
   v.setVal("DISABLEDSERVICES", s);
}

cResult dbackup::_info(persistvariables &v) const
{
   std::string path = _getPath(v).toString();

   if (!utils::fileexists(_getPath(v)))
   {
      if (path.length() == 0)
         fatal("dbackup is not configured (no path set).");
      else
         fatal("dbackup incorrectly configured, path does not exist:\n "+path);
   }

   std::vector<std::string> services;
   utils::getAllServices(services);

   std::vector<std::string> excluded;
   _getExcluded(excluded, v);

   logmsg(kLINFO, "SERVICES TO BE BACKED UP:");
   for (auto const & s : services)
   {
      bool isEnabled = (std::find(excluded.begin(), excluded.end(), s)==excluded.end()); // enabled if not in excluded list.

#ifdef _WIN32
         logmsg(kLINFO, (isEnabled ? "[Y] " : "[N] ") + s);
#else
      if (isEnabled)
         logmsg(kLINFO, "\e[32m\u2714\e[0m " + s);
      else
         logmsg(kLINFO, "\e[31m\e[1m\u2718\e[0m " + s);
#endif
   }
   logmsg(kLINFO, " ");
   logmsg(kLINFO, "Backups will be saved to:");
   logmsg(kLINFO, " " + path);

   return kRNoChange;
}

cResult dbackup::showHelp() const
{
   std::string help = R"EOF(
NAME
   dbackup

DESCRIPTION
   A dRunner plugin which provides backups across all installed dServices.

SYNOPSIS
   dbackup [COMMAND] [ARGS] ...

COMMANDS
   dbackup help
   dbackup configure [OPTION=[VALUE]]

   dbackup include SERVICENAME
   dbackup exclude SERVICENAME
   dbackup list
   [PASS=?] dbackup run
)EOF";

   logmsg(kLINFO, help);

   return kRSuccess;
}

cResult dbackup::_purgeOldBackups(const persistvariables &v) const
{
   Poco::Path path(_getPath(v));
   Poco::File file(path);

   logmsg(kLINFO, "--------------------------------------------------");
   logmsg(kLINFO, "Managing older backups");
   //bool getFolders(const std::string & parent, std::vector<std::string> & folders)
   int maxdays = atoi(v.getVal("MAXDAYS").c_str());
   int maxbackups = atoi(v.getVal("MAXBACKUPS").c_str());
   int alwayskeep = atoi(v.getVal("ALWAYSKEEP").c_str());

   if (maxdays < 5) fatal("MAXDAYS configuration value must be at least 5.");
   if (maxbackups <= alwayskeep) fatal("MAXBACKUPS configuration value must be greater than ALWAYSKEEP.");

   std::vector<std::string> folders;
   std::vector<int> days;
      
   file.list(folders);

   for (auto f : folders)
   {
      auto tf = timeutils::dateTimeStr2Time(f);
      Poco::DateTime now;
      days.push_back((now - tf).totalHours() / 24);
   }

   logmsg(kLDEBUG, "Currently have " + std::to_string( days.size() ) + " backups of " + std::to_string( maxbackups ) + " max.");

   int drop;
   while ((drop = refine(days, maxdays, maxbackups, alwayskeep)) < (int)days.size())
   {
      Poco::Path ftodel = path;
      ftodel.pushDirectory(folders[drop]);
      logmsg(kLINFO, "Deleting unneeded backup " + ftodel.toString());
      utils::deltree(ftodel);
      folders.erase(folders.begin() + drop);
      days.erase(days.begin() + drop);
   }

   logmsg(kLINFO, "Done");

   return kRSuccess;
}

inline double square(double x) { return x*x; }
#define MEXP1         2.71828182845904523536028747135266250   /* e */

double dbackup::objfn(double di, double i, double maxdays, double n)
{
   double numerator = exp(i / (n - 1)) - 1;
   double denominator = MEXP1 - 1;

   double idealvalue = maxdays * square(numerator / denominator);

   return square(di - idealvalue);
}

double dbackup::droptest(const std::vector<int> & backupdays, unsigned int droppos, double maxdays, unsigned int n)
{
   // calculate goodness.
   double val = 0.0;
   for (unsigned int i = 0, j = 0; i < n; ++i, ++j)
      if (i == droppos)
         --j; // j is index into fictional vector with backupdays[droppos] erased.
      else
         val += objfn(backupdays[i], j, maxdays, n - 1);
   drunner_assert(val > 0, "Logic error.");
   return val;
}

// Given a vector of backups, specified in days since today that the backup was taken (0=today), 
// the oldest backup we want to keep (maxdays), the max number of backups (maxbackups), and
// the number of most recent backups to always keep (alwayskeep), this returns the position
// of the backup to drop (or backupdays.length() if nothing should be dropped).
//
// Run until it returns backupdays.size(), deleting elements as you go.
//
// The basic objective function is a normalized Exp[x]^2, where x is the index normalized to [0,1].
// i.e. ( (Exp[i/(n-1)] - 1)/(e - 1) )^2
// We minimise using least squares the difference between this and the backup vector with one item dropped,
// in order to pick which item to drop (we choose the one with the lowest value of the ojective function).
//
// In addition to the objective function, we also ensure that the most recent 'alwayskeep' backups are retained,
// and we drop anything older than maxdays as our first priority.
//
// see backup algorithm.nb in docs for more info.
unsigned int dbackup::refine(const std::vector<int> & backupdays, int maxdays, int maxbackups, int alwayskeep)
{
   unsigned int n = backupdays.size();

   // keep all backups if we're under our limit.
   if ((int)n <= maxbackups)
      return backupdays.size();

   drunner_assert(alwayskeep < maxbackups, "Parameter error - trying to always keep more than the maximum number of backups!");

   // if our oldest backup is too old just drop that.
   if (backupdays[n-1] > maxdays)
      return n-1;

   // find the least wanted backup to drop, comparing to an exponential curve (keeping more of the recent ones).
   double bestval = -1;
   unsigned int bestpos = 0;
   for (unsigned int droppos = alwayskeep; droppos < n; ++droppos) // droppos must be in [0, ..., n-1]
   {
      double val = droptest(backupdays, droppos, maxdays, n);

      if (bestval < 0 || val < bestval)
      {
         bestval = val;
         bestpos = droppos;
      }
   }

   drunner_assert(bestval > 0, "Logic error.");
   return bestpos;
}
