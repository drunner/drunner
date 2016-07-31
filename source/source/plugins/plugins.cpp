#include <Poco/String.h>

#include "plugins.h"
#include "generate_plugin_script.h"
#include "globalcontext.h"
#include "globallogger.h"
#include "enums.h"

#include "dbackup.h"

plugins::plugins()
{
   mPlugins.push_back( std::unique_ptr<plugin>( new dbackup() ));
}

void plugins::generate_plugin_scripts() const
{
   for (auto p = mPlugins.begin() ; p != mPlugins.end() ; ++p)
      generate_plugin_script(p->get()->getName());
}

eResult plugins::runcommand() const
{
   std::string pluginname = GlobalContext::getParams()->getCommandStr();

   for (auto p = mPlugins.begin(); p != mPlugins.end(); ++p)
      if (0==Poco::icompare(p->get()->getName(), pluginname))
        return p->get()->runCommand();

   logmsg(kLERROR, "Unknown plugin '" + pluginname+"'");
   return kRError;
}

// -----------------------------------


