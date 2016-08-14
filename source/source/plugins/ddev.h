#ifndef __DDEV_H
#define __DDEV_H

#include "plugins.h"

class ddev : public configuredplugin
{
public:
   ddev();
   virtual std::string getName() const;
   virtual cResult runCommand(const CommandLine & cl, const variables & v) const;
   virtual cResult runHook(std::string hook, std::vector<std::string> hookparams, const servicelua::luafile & lf, const serviceVars &sv) const;
   cResult showHelp() const;

   Poco::Path configurationFilePath() const;

private:
   cResult _build(const CommandLine & cl, const variables & v, Poco::Path d) const;
   cResult _buildtree(const CommandLine & cl, const variables & v, Poco::Path d) const;
   cResult _test(const CommandLine & cl, const variables & v) const;
};


#endif
