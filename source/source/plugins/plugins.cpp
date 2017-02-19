#include <Poco/String.h>



#include "plugins.h"
#include "generate_plugin_script.h"
#include "globalcontext.h"
#include "globallogger.h"
#include "enums.h"
#include "dassert.h"

#include "dbackup.h"
#include "ddev.h"

plugins::plugins()
{
   mPlugins.push_back(std::unique_ptr<plugin>(new dbackup()));
   mPlugins.push_back(std::unique_ptr<plugin>(new ddev()));
}

void plugins::generate_plugin_scripts() const
{
   for (auto p = mPlugins.begin() ; p != mPlugins.end() ; ++p)
      generate_plugin_script(p->get()->getName());
}

cResult plugins::runcommand() const
{
   std::string pluginname = GlobalContext::getParams()->getCommandStr();

   for (auto p = mPlugins.begin(); p != mPlugins.end(); ++p)
      if (0==Poco::icompare(p->get()->getName(), pluginname))
        return p->get()->runCommand();

   return cError("Unknown plugin '" + pluginname + "'");
}

cResult plugins::runhook(std::string hook, std::vector<std::string> hookparams, const servicelua::luafile * lf, const serviceVars * sv) const
{
   cResult rval;
   for (auto p = mPlugins.begin(); p != mPlugins.end(); ++p)
      rval += p->get()->runHook(hook, hookparams, lf, sv);

   return rval;
}

void plugins::getPluginNames(std::vector<std::string> & names) const
{
   for (auto p = mPlugins.begin(); p != mPlugins.end(); ++p)
      names.push_back(p->get()->getName());
}


// -----------------------------------


configuredplugin::configuredplugin() 
{
}

configuredplugin::~configuredplugin()
{
}

cResult configuredplugin::runCommand() const
{
   CommandLine cl;

   if (GlobalContext::getParams()->numArgs() > 0)
   {
      cl.args = GlobalContext::getParams()->getArgs();
      cl.command = cl.args[0];
      cl.args.erase(cl.args.begin());
   }
   
   persistvariables pv(getPersistVariables());

   if (cl.command == "configure")
      return pv.handleConfigureCommand(cl);

   return runCommand(cl, pv);
}

cResult configuredplugin::addConfig(envDef def)
{
   mConfiguration.push_back(def);
   return kRSuccess;
}

const persistvariables configuredplugin::getPersistVariables() const
{
   Poco::Path spath = configurationFilePath();
   persistvariables pv(getName(), spath, mConfiguration);

   // if the variables file does not exist then create it (will pick up defaults from pv ctor).
   if (!pv.exists())
   {
      if (!pv.savevariables().success())
         fatal("Couldn't create " + spath.toString());
      else
         logdbg("Created " + spath.toString());
   }

   cResult r = pv.loadvariables();
   drunner_assert(r.success(), "Failed to save empty variables.");

   return pv;
}

cResult configuredplugin::setAndSaveVariable(std::string key, std::string val) const
{
   Poco::Path spath = configurationFilePath();
   persistvariables pv(getName(), spath, mConfiguration);
   cResult r = pv.loadvariables();
   if (r.success())
   {
      r += pv.setVal(key, val);
      if (r.success())
         r += pv.savevariables();
   }
   return r;
}

