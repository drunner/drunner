#ifndef __DBACKUP_H
#define __DBACKUP_H

#include <string>
#include "plugins.h"
#include "backupConfig.h"

class dbackup : public pluginhelper
{
public:
   dbackup();
   std::string getName() const;
   cResult runCommand(const CommandLine & cl, const variables & v) const;
   Poco::Path configurationFilePath() const;

   cResult showHelp() const;

private:
   cResult include(std::string servicename) const;
   cResult exclude(std::string servicename) const;
   cResult run() const;
   cResult info() const;
   cResult configure(std::string path) const;

   cResult purgeOldBackups(backupConfig & config) const;
};

#endif
