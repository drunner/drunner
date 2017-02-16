#include <fstream>
#include <Poco/Path.h>

#include "drunner_settings.h"
#include "utils.h"
#include "globallogger.h"
#include "drunner_setup.h"
#include "drunner_paths.h"



static std::string sBadURL = R"EOF(https://drunner.s3.amazonaws.com/drunner)EOF";

// can't put this in header because circular
// dependency then with utils::getTime.
drunnerSettings::drunnerSettings() : persistvariables("drunner", drunnerPaths::getPath_drunnerSettings_json(),_getConfig())
{
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

std::string drunnerSettings::getdrunnerInstallURL() 
{
#ifdef _WIN32
   return R"EOF(https://drunner.s3.amazonaws.com/10/win/drunner)EOF";
#elif defined(__APPLE__)
   return R"EOF(https://drunner.s3.amazonaws.com/10/mac/drunner)EOF";
#else
   return R"EOF(https://drunner.s3.amazonaws.com/10/lin/drunner)EOF";
#endif
}

const std::vector<envDef> drunnerSettings::_getConfig()
{
   std::vector<envDef> config;
   config.push_back(envDef("INSTALLTIME", utils::getTime(), "Time installed.",ENV_PERSISTS ));
   config.push_back(envDef("PULLIMAGES", "true", "Set to false to never pull docker images",ENV_PERSISTS | ENV_USERSETTABLE));
   return config;
}
