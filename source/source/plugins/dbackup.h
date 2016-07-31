#ifndef __DBACKUP_H
#define __DBACKUP_H

#include <string>
#include "enums.h"
#include "plugins.h"
#include "backupConfig.h"

class dbackup : public plugin
{
public:
   dbackup();
   virtual std::string getName() const;
   virtual cResult runCommand() const;

private:
   cResult include(std::string servicename) const;
   cResult exclude(std::string servicename) const;
   cResult run() const;
   cResult info() const;
   cResult configure(std::string path) const;

   cResult purgeOldBackups(backupConfig & config) const;

   cResult showhelp() const;
};

#endif
