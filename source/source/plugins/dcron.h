#ifndef __DCRON_H
#define __DCRON_H

#include "plugins.h"

class dcron : public configuredplugin
{
public:
   dcron();
   virtual std::string getName() const;
   virtual cResult runCommand(const CommandLine & cl, const variables & v) const;
   virtual cResult runHook(std::string hook, std::vector<std::string> hookparams, const servicelua::luafile * lf, const serviceVars * sv) const;
   cResult showHelp() const;

   Poco::Path configurationFilePath() const;

private:
   cResult _runcron(const CommandLine & cl, const variables & v, Poco::Path d) const;
};



#endif
