#ifndef __DBACKUP_H
#define __DBACKUP_H

#include <string>
#include "plugins.h"

class dbackup : public configuredplugin
{
public:
   dbackup();
   std::string getName() const;
   cResult runCommand(const CommandLine & cl, const variables & v) const;
   cResult runHook(std::string hook, std::vector<std::string> hookparams, const servicelua::luafile * lf, const serviceVars * sv) const;
   
   cResult showHelp() const;

private:
   cResult _include(std::string servicename, variables &v) const;
   cResult _exclude(std::string servicename, variables &v) const;
   cResult _run(variables &v) const;
   cResult _info(variables &v) const;
   cResult _configure(std::string path, variables &v) const; // hook command.

   cResult purgeOldBackups() const;
};

#endif
