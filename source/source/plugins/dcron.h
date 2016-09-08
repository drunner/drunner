#ifndef __DCRON_H
#define __DCRON_H

#include "plugins.h"

class dcron : public configuredplugin
{
public:
   dcron();
   ~dcron();
   virtual std::string getName() const;
   virtual cResult runCommand(const CommandLine & cl, const variables & v) const;
   virtual cResult runHook(std::string hook, std::vector<std::string> hookparams, const servicelua::luafile * lf, const serviceVars * sv) const;
   cResult showHelp() const;

   Poco::Path configurationFilePath() const;
   Poco::Path lockFilePath() const;

private:
   bool _runjob(std::string uniquename, const servicelua::CronEntry & c) const;
   cResult _runcron(const CommandLine & cl, const variables & v) const;
};



#endif
