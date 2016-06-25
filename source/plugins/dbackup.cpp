#include <string>
#include <vector>
#include <fstream>
#include <algorithm>

#include "utils.h"
#include "globallogger.h"
#include "globalcontext.h"
#include "timez.h"
#include "service.h"

#include "dbackup.h"
#include "backupConfig.h"

// -----------------------------------------------------------------------------------------------------------

dbackup::dbackup()
{
}

std::string dbackup::getName() const
{
   return std::string("dbackup");
}

eResult dbackup::runCommand() const
{
   if (GlobalContext::getParams()->numArgs() == 0)
      return showhelp();

   std::vector<std::string> args = GlobalContext::getParams()->getArgs();
   std::string cmd = args[0];
   args.erase( args.begin() );

   logmsg(kLDEBUG, "Running command " + cmd);

   switch (str2int(cmd.c_str()))
   {
   case str2int("help") :
      return showhelp();

   case str2int("config") :
   case str2int("configure") :
      if (args.size() == 0)
         fatal("Usage:  dbackup configure BACKUPPATH");
      return configure(args[0]);

   case str2int("exclude") :
      if (args.size() == 0)
         fatal("Usage:  dbackup exclude SERVICENAME");
      return exclude(args[0]);

   case str2int("include"):
      if (args.size() == 0)
         fatal("Usage:  dbackup include SERVICENAME");
      return include(args[0]);

   case str2int("run"):
      return run();

   case str2int("info"):
   case str2int("list"):
      return info();

   default:
      fatal("Unrecognised command " + cmd);
      return kRError;
   }
}

// -----------------------------------------------------------------------------------------------------------

eResult dbackup::include(std::string servicename) const
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

eResult dbackup::exclude(std::string servicename) const
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

eResult dbackup::run() const
{
   backupConfig config;
   if (!config.load())
   {
      logmsg(kLINFO, "Backups are not yet configured.  Run dbackup configure first.");
      return kRError;
   }

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

eResult dbackup::configure(std::string path) const
{
   backupConfig config;
   config.load();

   // TODO: check path exists and make canonical.
   if (!utils::fileexists(path))
      logmsg(kLERROR, "The path " + path + " does not exist.");
   path = utils::getcanonicalpath(path);

   if (utils::stringisame(config.getBackupPath(), path))
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

eResult dbackup::info() const
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
      if (config.isEnabled(s))
         logmsg(kLINFO, "\e[32m\u2714\e[0m " + s);
      else
         logmsg(kLINFO, "\e[31m\e[1m\u2718\e[0m " + s);
   }
   logmsg(kLINFO, " ");
   logmsg(kLINFO, "Backups will be saved to:");
   logmsg(kLINFO, " " + config.getBackupPath());

   return kRNoChange;
}

eResult dbackup::showhelp() const
{
   std::string help = R"EOF(
NAME
   dbackup

DESCRIPTION
   A dRunner plugin which provides backups across all installed dServices.

SYNOPSIS
   dbackup [COMMAND] [ARGS] ...

COMMANDS
   dbackup configure BACKUPPATH
   dbackup include SERVICENAME
   dbackup exclude SERVICENAME
   dbackup list
   dbackup run
)EOF";

   logmsg(kLINFO, help);

   return kRError;
}

void shifty(std::string src, std::string dst, int n)
{
   std::vector<std::string> folders;
   utils::getFolders(src, folders);
   std::sort(folders.begin(), folders.end());
   if (folders.size() > n)
   {

   }
}

eResult dbackup::purgeOldBackups(backupConfig & config) const
{
   logmsg(kLINFO, "--------------------------------------------------");
   logmsg(kLINFO, "Pruning old backups");
   //bool getFolders(const std::string & parent, std::vector<std::string> & folders)

   std::string dailyfolder = config.getBackupPath() + "/daily";
   std::string weeklyfolder = config.getBackupPath() + "/weekly";
   std::string monthlyfolder = config.getBackupPath() + "/monthly";

   shifty(dailyfolder, weeklyfolder, 7);
   shifty(weeklyfolder, monthlyfolder, 4);
   shifty(monthlyfolder, "", 6);
}
