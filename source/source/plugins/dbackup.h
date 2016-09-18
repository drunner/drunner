#ifndef __DBACKUP_H
#define __DBACKUP_H

#include <string>
#include "plugins.h"

class dbackup : public configuredplugin
{
public:
   dbackup();
   std::string getName() const;
   cResult runCommand(const CommandLine & cl, persistvariables & v) const;
   cResult runHook(std::string hook, std::vector<std::string> hookparams, const servicelua::luafile * lf, const serviceVars * sv) const;
   
   cResult showHelp() const;

   Poco::Path configurationFilePath() const;

   // we do have a cron service for dbackup (running backups nightly).
   servicelua::CronEntry getCron() const;
   cResult runCron() const;


   // given a vector of backups, specified in days since today that the backup was taken (0=today), 
   // the oldest backup we want to keep (maxdays), the max number of backups (maxbackups), and
   // the number of most recent backups to always keep (alwayskeep), refine returns the position
   // of the backup to drop (or backupdays.length() if nothing should be dropped).
   //
   // Run until it returns backupdays.length(), deleting elements as you go.
   static unsigned int refine(const std::vector<int> & backupdays, int maxdays, int maxbackups, int alwayskeep); 

   // exposed only for unit testing.
   static double objfn(double di, double i, double maxdays, double n);
   static double droptest(const std::vector<int> & backupdays, unsigned int droppos, double maxdays, unsigned int n);

private:
   cResult _include(std::string servicename, persistvariables &v) const;
   cResult _exclude(std::string servicename, persistvariables &v) const;
   cResult _run(const persistvariables &v) const;
   cResult _info(persistvariables &v) const;

   Poco::Path _getPath(const persistvariables &v) const;
   void _getExcluded(std::vector<std::string> & vs, const persistvariables &v) const;
   void _setExcluded(const std::vector<std::string> & vs, persistvariables &v) const;

   cResult _purgeOldBackups(const persistvariables &v) const;
};

#endif
