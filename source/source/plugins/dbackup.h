#ifndef __DBACKUP_H
#define __DBACKUP_H

#include <string>
#include "plugins.h"

class dbackup : public plugin
{
public:
   dbackup();
   std::string getName() const;
   cResult runCommand() const;
   cResult runHook(std::string hook, std::vector<std::string> hookparams, const servicelua::luafile * lf, const serviceVars * sv) const;
   
   cResult showHelp() const;

private:
   cResult include(std::string servicename) const;
   cResult exclude(std::string servicename) const;
   cResult run() const;
   cResult info() const;
   cResult configure(std::string path) const;

   cResult purgeOldBackups() const;
};

#endif
