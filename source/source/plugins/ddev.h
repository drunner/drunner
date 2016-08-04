#ifndef __DDEV_H
#define __DDEV_H

#include "plugins.h"

class ddev : public pluginhelper
{
public:
   ddev();
   virtual std::string getName() const;
   virtual cResult runCommand(const CommandLine & cl, const variables & v) const;
   cResult showHelp() const;

   Poco::Path configurationFilePath() const;

private:
   cResult _build(const CommandLine & cl, const variables & v) const;
   cResult _test(const CommandLine & cl, const variables & v) const;
};


#endif
