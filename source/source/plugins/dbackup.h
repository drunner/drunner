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

private:
   cResult _include(std::string servicename, persistvariables &v) const;
   cResult _exclude(std::string servicename, persistvariables &v) const;
   cResult _run(persistvariables &v) const;
   cResult _info(persistvariables &v) const;

   Poco::Path _getPath(persistvariables &v) const;
   void _getExcluded(std::vector<std::string> & vs, persistvariables &v) const;
   void _setExcluded(const std::vector<std::string> & vs, persistvariables &v) const;

   cResult _purgeOldBackups(persistvariables &v) const;
};

#endif
