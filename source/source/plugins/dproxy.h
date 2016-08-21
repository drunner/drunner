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
   static Poco::Path haproxyCfgPath();
private:
   cResult update(std::string ip) const;
   cResult start() const;
   cResult stop() const;
   cResult status(std::string ip) const;

   std::string generateconfig(std::string ip) const;

   bool configNeedsUpdated(std::string ip) const;
   bool dproxyRunning() const;
};


#endif
