#ifndef __DPROXY_H
#define __DPROXY_H

#include "plugins.h"

class dproxy : public pluginhelper
{
public:
   dproxy();
   virtual std::string getName() const;
   virtual cResult runCommand(const CommandLine & cl, const variables & v) const;
   cResult showHelp() const;

   Poco::Path configurationFilePath() const;

private:
};


#endif
