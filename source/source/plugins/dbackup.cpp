#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <Poco/File.h>
#include <Poco/String.h>
#include <cereal/archives/json.hpp>

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

cResult dbackup::_run(persistvariables &v) const
{
   Poco::Path p = _getPath(v);
   std::string path = p.toString();

   if (!utils::fileexists(p))
      fatal("Configure dbackup before running.");
   utils::makedirectory(path + "/daily", S_700);
   utils::makedirectory(path + "/weekly", S_700);
   utils::makedirectory(path + "/monthly", S_700);

   std::vector<std::string> services;
   utils::getAllServices(services);

   std::string datefolder = path + "/daily/" + timeutils::getDateTimeStr();
   utils::makedirectory(datefolder, S_700);

   logmsg(kLINFO, "Backing up services.");
   std::vector<std::string> excludedservices;
   _getExcluded(excludedservices, v);

   for (auto const & s : services)
   {
      if (std::find(excludedservices.begin(), excludedservices.end(), s) == excludedservices.end()) // not excluded.
      {
         // backup service s.
         std::string path = datefolder + "/" + timeutils::getArchiveName(s);

         logmsg(kLINFO, "----------------- " + s + " --------------------");
         service svc(s);
         svc.backup(path);
      }
   }

   return _purgeOldBackups(v);
}

Poco::Path dbackup::_getPath(persistvariables & v) const
{
   Poco::Path p(v.getVal("BACKUPPATH"));
   if (!p.isAbsolute())
      p.makeAbsolute();
   return p;
}

void dbackup::_getExcluded(std::vector<std::string>& vs, persistvariables &v) const
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

void shifty(std::string src, std::string dst, int interval, unsigned int numtokeep)
{
   Poco::File f(src);
   if (!f.exists())
      fatal("shift: source doesn't exist - "+src);

   std::vector<std::string> folders;
   f.list(folders);
   std::sort(folders.begin(), folders.end());
   if (folders.size() <= numtokeep)
      return; // nothing to do.

   // move/delete anything old
   for (auto f : folders)
   {
      auto tf = timeutils::dateTimeStr2Time(f);
      Poco::DateTime now;

      if ((now-tf).totalHours() > interval)
      { // old
         if (dst.length() != 0)
         { // shift
            logmsg(kLINFO, "Moving backup " + f + " to "+dst);
            utils::movetree(src + "/" + f, dst + "/" + f);
         }
         else
         {
            logmsg(kLINFO, "Deleting old backup " + src + "/" + f);
            utils::deltree(src + "/" + f);
         }
      }
   }

   // prune anything unneeded that puts us over storage limit.
   for (unsigned int i = 0; i < folders.size(); ++i)
   { // check trio folder[i]..[i+2]
      Poco::DateTime f0, f2;

      f0 = timeutils::dateTimeStr2Time(folders[i]);
      f2 = timeutils::dateTimeStr2Time(folders[i+2]);

      if (f2 < f0)
         logmsg(kLERROR, "Backup folders after sorting are in incorrect order. :/");

      if ((f2 - f0).totalHours() < interval)
      { // f1 is redundant. Delete it!
         logmsg(kLINFO, "Deleting unneeded backup " + src+"/"+folders[i + 1]);
         utils::deltree(src + "/" + folders[i + 1]);
         folders.erase(folders.begin() + i + 1);
         if (folders.size() <= numtokeep)
            return; // we've done enough
         --i; // try again with folders[i]..[i+2]
      }
   }

}

cResult dbackup::_purgeOldBackups(persistvariables &v) const
{
   std::string path = _getPath(v).toString();

   logmsg(kLINFO, "--------------------------------------------------");
   logmsg(kLINFO, "Managing older backups");
   //bool getFolders(const std::string & parent, std::vector<std::string> & folders)

   std::string dailyfolder = path + "/daily";
   std::string weeklyfolder = path + "/weekly";
   std::string monthlyfolder = path + "/monthly";

   shifty(dailyfolder, weeklyfolder, 24, 7);
   shifty(weeklyfolder, monthlyfolder, 24*7, 4);
   shifty(monthlyfolder, "", 24*7*30, 6);

   logmsg(kLINFO, "Done");

   return kRSuccess;
}