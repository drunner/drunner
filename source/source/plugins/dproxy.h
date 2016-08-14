#ifndef __DPROXY_H
#define __DPROXY_H

#include "plugins.h"

class dproxy : public configuredplugin
{
public:
   dproxy();
   virtual std::string getName() const;
   virtual cResult runCommand(const CommandLine & cl, const variables & v) const;
   virtual cResult runHook(std::string hook, std::vector<std::string> hookparams, const servicelua::luafile * lf, const serviceVars * sv) const;

   cResult showHelp() const;

   Poco::Path configurationFilePath() const;

private:
   cResult update() const;
   cResult start() const;
   cResult stop() const;
};


#endif
