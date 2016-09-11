#include "globalcontext.h"
#include "globallogger.h"
#include "utils.h"
#include "drunner_setup.h"
#include "dassert.h"

std::shared_ptr<const params> GlobalContext::s_params = 0;
std::shared_ptr<const drunnerSettings> GlobalContext::s_settings = 0;
std::shared_ptr<const plugins> GlobalContext::s_plugins = 0;
//bool GlobalContext::s_ForceDeveloperMode = false;

GlobalContext::GlobalContext()
{
}

void GlobalContext::init(int argc, char * const * argv)
{
   // parse the command line parameters.
   s_params = std::make_shared<const params>(argc,argv);

   // create the plugins
   s_plugins = std::make_shared<const plugins>();

   // load the dRunner settings from the config file.
   s_settings = std::make_shared<const drunnerSettings>();
}

void GlobalContext::init(const std::vector<std::string>& args)
{
   drunner_assert(!args.size() == 0, "Empty arguments passed to init.");
   std::vector<char*> cstrings;
   for (size_t i = 0; i < args.size(); ++i)
      cstrings.push_back(const_cast<char*>(args[i].c_str()));
   drunner_assert(!cstrings.empty(), "Error.");

   init(cstrings.size(), &cstrings[0]);
}

bool GlobalContext::hasParams()
{
   return s_params.get() != NULL;
}
bool GlobalContext::hasSettings()
{
   return s_settings.get() != NULL;
}

bool GlobalContext::hasPlugins()
{
   return s_plugins.get() != NULL;
}

//bool GlobalContext::isDeveloperMode()
//{
//   if (s_ForceDeveloperMode)
//      return true;
//
//   if (hasParams() && getParams()->isDevelopmentMode())
//      return true;
//
//   return false;
//}

//void GlobalContext::forceDeveloperMode()
//{
//   s_ForceDeveloperMode = true;
//}

std::shared_ptr<const params> GlobalContext::getParams()
{
   if (!hasParams())
      fatal("Attempted to retrieve parameters from GlobalContext when not yet initialised.");
   return s_params;
}
std::shared_ptr<const drunnerSettings> GlobalContext::getSettings()
{
   if (!hasSettings())
      fatal("Attempted to retrieve settings from GlobalContext when settings have not been read.");
   return s_settings;
}

std::shared_ptr<const plugins> GlobalContext::getPlugins()
{
   if (!hasPlugins())
      fatal("Attempted to retrieve plugins from GlobalContext when it doesn't exist yet.");
   return s_plugins;
}

