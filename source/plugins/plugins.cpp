#include "plugins.h"
#include "generate_plugin_script.h"
#include "globalcontext.h"
#include "globallogger.h"
#include "enums.h"

plugins::plugins()
{
}

void plugins::generate_plugin_scripts() const
{
   for (auto p : mPlugins)
      generate_plugin_script(p->getName());
}

eResult plugins::runcommand() const
{
   std::string pluginname = GlobalContext::getParams()->getCommandStr();

   for (auto p : mPlugins)
      if (utils::stringisame(p->getName(), pluginname))
        return p->runCommand();

   logmsg(kLERROR, "Unknown plugin '" + pluginname+"'");
   return kRError;
}

plugin::plugin()
{
}
