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

class backupConfig
{
public:
   backupConfig() 
   {
      mPath = drunnerPaths::getPath_Settings().setFileName("dbackup.json");
   }
   cResult loadconfig() 
   {
      std::ifstream ifs(mPath.toString());
      if (ifs.bad())
         return cError("Bad input stream to backupinfo::loadvars. :/");
      cereal::JSONInputArchive archive(ifs);
      archive(*this);
      return kRSuccess;
   }
   cResult saveconfig()
   {
      std::ofstream os(mPath.toString());
      if (os.bad())
         return cError("Bad output stream.");
      cereal::JSONOutputArchive archive(os);
      archive(*this);
      return kRSuccess;
   }

   bool isEnabled(std::string servicename)
   {
      auto result = std::find(mDisabledServices.begin(), mDisabledServices.end(), servicename);
      return (result == mDisabledServices.end());
   }

   cResult enable(std::string servicename)
   {
      if (isEnabled(servicename))
         return kRNoChange;
      auto result = std::find(mDisabledServices.begin(), mDisabledServices.end(), servicename);
      drunner_assert(result != mDisabledServices.end(),"Result of isEnabled is broken?");
      mDisabledServices.erase(result);
      return kRSuccess;
   }

   cResult disable(std::string servicename)
   {
      if (!isEnabled(servicename))
         return kRNoChange;
      mDisabledServices.push_back(servicename);
      return kRSuccess;
   }

   void update(std::string backuppath)
   {
      mBackupPath = backuppath;
      mIsConfigured = true;
   }

   std::string getBackupPath()
   {
      return mBackupPath;
   }

private:
   std::vector<std::string> mDisabledServices;
   std::string mBackupPath;
   bool mIsConfigured;
   Poco::Path mPath;

   // --- serialisation --
   friend class cereal::access;
   template <class Archive> void save(Archive &ar, std::uint32_t const version) const { ar(mBackupPath, mDisabledServices); }
   template <class Archive> void load(Archive &ar, std::uint32_t const version) { ar(mBackupPath, mDisabledServices); }
   // --- serialisation --
};

// -----------------------------------------------------------------------------------------------------------

dbackup::dbackup() 
{
}

std::string dbackup::getName() const
{
   return std::string("dbackup");
}

cResult dbackup::runCommand() const
{
   CommandLine cl;
   if (GlobalContext::getParams()->numArgs() == 0)
      cl.command = "help";
   else
   {
      cl.args = GlobalContext::getParams()->getArgs();
      cl.command = cl.args[0];
      cl.args.erase(cl.args.begin());
   }

   switch (s2i(cl.command.c_str()))
   {
   case s2i("help") :
      return showHelp();

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

cResult dbackup::runHook(std::string hook, std::vector<std::string> hookparams, const servicelua::luafile * lf, const serviceVars * sv) const
{
   return kRNoChange;
}


// -----------------------------------------------------------------------------------------------------------

cResult dbackup::include(std::string servicename) const
{
   backupConfig config;
   if (!config.loadconfig())
      logmsg(kLERROR, "Backups are not yet configured. Run dbackup configure first.");

   if (config.enable(servicename) == kRNoChange)
      return kRNoChange;

   if (!config.saveconfig())
      logmsg(kLERROR, "Failed to write to configuration file.");

   return kRSuccess;
}

cResult dbackup::exclude(std::string servicename) const
{
   backupConfig config;
   if (!config.loadconfig())
      logmsg(kLERROR, "Backups are not yet configured. Run dbackup configure first.");

   if (config.disable(servicename) == kRNoChange)
      return kRNoChange;

   if (!config.saveconfig())
      logmsg(kLERROR, "Failed to write to configuration file.");

   return kRSuccess;
}

cResult dbackup::run() const
{
   backupConfig config;
   if (!config.loadconfig())
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

   return purgeOldBackups();
}

cResult dbackup::configure(std::string path) const
{
   backupConfig config;
   config.loadconfig();

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
   if (!config.saveconfig())
      logmsg(kLERROR, "Unable to save backup configuration file.");

   logmsg(kLINFO, "The backup path has been changed to " + path);

   return kRSuccess;
}

cResult dbackup::info() const
{
   backupConfig config;
   if (!config.loadconfig())
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

cResult dbackup::purgeOldBackups() const
{
   backupConfig config;
   if (!config.loadconfig())
      return cError("Backups are not yet configured.  Run dbackup configure first.");

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