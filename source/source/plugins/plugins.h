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

// ----------------------------------------------------------------------------------------

class plugin
{
public:
   virtual std::string getName() const = 0;
   virtual cResult runCommand() const = 0;
};

// ----------------------------------------------------------------------------------------

class pluginhelper : public plugin
{
public:
   pluginhelper(std::string name);
   virtual ~pluginhelper();

   virtual cResult runCommand(const CommandLine & cl, const variables & v) const = 0;
   virtual cResult showHelp() const = 0;
   virtual Poco::Path configurationFilePath() const = 0;

public:
   virtual cResult runCommand() const;
   cResult addConfig(std::string name, std::string description, std::string defaultval, configtype type, bool required);

protected:
   std::string mName;
   std::vector<Configuration> mConfiguration;
};

// ----------------------------------------------------------------------------------------

class plugins
{
public:
   plugins();

   void generate_plugin_scripts() const;
   cResult runcommand() const;

private:
   std::deque< std::unique_ptr<plugin> > mPlugins;
};






#endif
