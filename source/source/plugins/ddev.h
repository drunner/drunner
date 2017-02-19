#ifndef __DDEV_H
#define __DDEV_H

#include "plugins.h"

class ddev : public plugin
{
public:
   ddev();
   virtual std::string getName() const;
   virtual cResult runCommand(const CommandLine & cl) const;

   Poco::Path configurationFilePath() const;

private:
   cResult _showHelp() const;
   cResult _ddevtree(const CommandLine & cl, Poco::Path d) const;
   cResult _test(std::string dservicename) const;

   void _buildtree(const Poco::Path d) const;

   std::string load_ddev(const Poco::Path d) const;
};


#endif
