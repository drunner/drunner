#include <fstream>
#include <Poco/Path.h>
#include <cereal/archives/json.hpp>

#include "drunner_settings.h"
#include "utils.h"
#include "globallogger.h"
#include "drunner_setup.h"
#include "drunner_paths.h"


// can't put this in header because circular
// dependency then with utils::getTime.
drunnerSettings::drunnerSettings() : persistvariables("drunner", drunnerPaths::getPath_drunnerSettings_json())
{
   std::vector<Configuration> config;
   Configuration c;
   c.name = "INSTALLURL"; c.defaultval = R"EOF(https://drunner.s3.amazonaws.com/drunner)EOF"; c.description = "The URL to download drunner from."; c.type = kCF_URL; c.required = true;
   config.push_back(c);
   c.name = "INSTALLTIME"; c.defaultval = utils::getTime(); c.description = "Time installed."; c.type = kCF_string; c.required = false;
   config.push_back(c);
   c.name = "PULLIMAGES"; c.defaultval = "true"; c.description = "Whether to pull docker images"; c.type = kCF_bool; c.required = true;
   config.push_back(c);

   setConfiguration(config);

   mReadOkay = false;   

   if (!utils::fileexists(mPath))
      logdbg("The dRunner settings file does not exist: " + mPath.toString());
   else
   {
      cResult r = loadvariables();

      if (r.success())
      {
         mReadOkay = true;
         logdbg("Read dRunner settings from " + drunnerPaths::getPath_drunnerSettings_json().toString());
      }
      else if (r.error())
         fatal("The settings file is corrupt and could not be read: " + drunnerPaths::getPath_drunnerSettings_json().toString() + "\n" + r.what() + "\nSuggest deleting it.");
      else
         logdbg("Couldn't read settings file from " + drunnerPaths::getPath_drunnerSettings_json().toString());
   }
}

