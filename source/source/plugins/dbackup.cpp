#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <Poco/File.h>
#include <Poco/String.h>

#include "utils.h"
#include "globallogger.h"
#include "globalcontext.h"
#include "timez.h"
#include "service.h"

#include "dbackup.h"
#include "backupConfig.h"
#include "drunner_paths.h"

// -----------------------------------------------------------------------------------------------------------

dbackup::dbackup() : pluginhelper("dbackup")
{
   addConfig("BACKUPPATH", "The path to back up to.", "", kCF_path, true);
}

std::string dbackup::getName() const
{
   return std::string("dbackup");
}

cResult dbackup::runCommand(const CommandLine & cl, const variables & v) const
{
   switch (s2i(cl.command.c_str()))
   {
   case s2i("config") :
   case s2i("configure") :
      if (cl.args.size() == 0)
         fatal("Usage:  dbackup configure BACKUPPATH");
      return configure(cl.args[0]);

   case s2i("exclude") :
      if (cl.args.size() == 0)
         fatal("Usage:  dbackup exclude SERVICENAME");
      return exclude(cl.args[0]);

   case s2i("include"):
      if (cl.args.size() == 0)
         fatal("Usage:  dbackup include SERVICENAME");
      return include(cl.args[0]);

   case s2i("run"):
      return run();

   case s2i("info"):
   case s2i("list"):
      return info();

   default:
      return cError("Unrecognised command " + cl.command);
   }
}

// -----------------------------------------------------------------------------------------------------------

cResult dbackup::include(std::string servicename) const
{
   backupConfig config;
   if (!config.load())
      logmsg(kLERROR, "Backups are not yet configured. Run dbackup configure first.");

   if (config.enable(servicename) == kRNoChange)
      return kRNoChange;

   if (!config.save())
      logmsg(kLERROR, "Failed to write to configuration file.");

   return kRSuccess;
}

cResult dbackup::exclude(std::string servicename) const
{
   backupConfig config;
   if (!config.load())
      logmsg(kLERROR, "Backups are not yet configured. Run dbackup configure first.");

   if (config.disable(servicename) == kRNoChange)
      return kRNoChange;

   if (!config.save())
      logmsg(kLERROR, "Failed to write to configuration file.");

   return kRSuccess;
}

cResult dbackup::run() const
{
   backupConfig config;
   if (!config.load())
      return cError("Backups are not yet configured.  Run dbackup configure first.");

   std::vector<std::string> services;
   utils::getAllServices(services);

   std::string datefolder = config.getBackupPath() + "/daily/" + timeutils::getDateTimeStr();
   utils::makedirectory(datefolder, S_700);

   logmsg(kLINFO, "Backing up services.");
   for (auto const & s : services)
      if (config.isEnabled(s))
      {
         // backup service s.
         std::string path = datefolder + "/" + timeutils::getArchiveName(s);
         
         logmsg(kLINFO, "----------------- " + s + " --------------------");
         service svc(s);
         svc.backup(path);
      }

   return purgeOldBackups(config);
}

cResult dbackup::configure(std::string path) const
{
   backupConfig config;
   config.load();

   // TODO: check path exists and make canonical.
   if (!utils::fileexists(path))
      logmsg(kLERROR, "The path " + path + " does not exist.");

   Poco::Path p(path);
   if (!p.isAbsolute())
      p.makeAbsolute();
   path = p.toString(Poco::Path::PATH_NATIVE);

   // create folders.
   utils::makedirectory(path + "/daily", S_700);
   utils::makedirectory(path + "/weekly", S_700);
   utils::makedirectory(path + "/monthly", S_700);

   if (0==Poco::icompare(config.getBackupPath(), path))
   {
      logmsg(kLINFO, "Path unchanged.");
      return kRNoChange;
   }

   config.update(path);
   if (!config.save())
      logmsg(kLERROR, "Unable to save backup configuration file.");

   logmsg(kLINFO, "The backup path has been changed to " + path);

   return kRSuccess;
}

cResult dbackup::info() const
{
   backupConfig config;
   if (!config.load())
   {
      logmsg(kLINFO, "Backups are not yet configured.");
      return kRNoChange;
   }

   std::vector<std::string> services;
   utils::getAllServices(services);

   logmsg(kLINFO, "SERVICES TO BE BACKED UP:");
   for (auto const & s : services)
   {
#ifdef _WIN32
         logmsg(kLINFO, (config.isEnabled(s) ? "[Y] " : "[N] ") + s);
#else
      if (config.isEnabled(s))
         logmsg(kLINFO, "\e[32m\u2714\e[0m " + s);
      else
         logmsg(kLINFO, "\e[31m\e[1m\u2718\e[0m " + s);
#endif
   }
   logmsg(kLINFO, " ");
   logmsg(kLINFO, "Backups will be saved to:");
   logmsg(kLINFO, " " + config.getBackupPath());

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

cResult dbackup::purgeOldBackups(backupConfig & config) const
{
   logmsg(kLINFO, "--------------------------------------------------");
   logmsg(kLINFO, "Managing older backups");
   //bool getFolders(const std::string & parent, std::vector<std::string> & folders)

   std::string dailyfolder = config.getBackupPath() + "/daily";
   std::string weeklyfolder = config.getBackupPath() + "/weekly";
   std::string monthlyfolder = config.getBackupPath() + "/monthly";

   shifty(dailyfolder, weeklyfolder, 24, 7);
   shifty(weeklyfolder, monthlyfolder, 24*7, 4);
   shifty(monthlyfolder, "", 24*7*30, 6);

   logmsg(kLINFO, "Done");

   return kRSuccess;
}

Poco::Path dbackup::configurationFilePath() const
{
   return drunnerPaths::getPath_Root().setFileName("dbackup.json");
}
