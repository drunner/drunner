#ifndef __PLUGINS_H
#define __PLUGINS_H

#include <deque>
#include <vector>
#include <string>
#include <memory>

#include <Poco/Path.h>

#include "cresult.h"
#include "utils.h"
#include "variables.h"
#include "service_lua.h"
#include "service_vars.h"

// ----------------------------------------------------------------------------------------

class plugin
{
public:
   virtual std::string getName() const = 0;
   virtual cResult runCommand(const CommandLine & cl) const = 0;
};

// ----------------------------------------------------------------------------------------

// a plugin that has stored configuration (can be per service, or for the entire plugin)
class configuredplugin : public plugin
{
public:
   configuredplugin();
   virtual ~configuredplugin();

   // runCommand may wish to update system variables and save them, so we pass a non-const reference.
   virtual cResult runCommand(const CommandLine & cl, persistvariables & v) const = 0;
   
   virtual Poco::Path configurationFilePath() const = 0;

public:
   virtual cResult runCommand(const CommandLine & cl) const;
   cResult addConfig(envDef config);

protected:
   const persistvariables getPersistVariables() const;
   cResult setAndSaveVariable(std::string key, std::string val) const;

   std::vector<envDef> mConfiguration;
};

// ----------------------------------------------------------------------------------------

class plugins
{
public:
   plugins();

   void generate_plugin_scripts() const;
   cResult runcommand() const;

   void getPluginNames(std::vector<std::string> & names) const;

private:
   std::deque< std::unique_ptr<plugin> > mPlugins;
};

#endif
