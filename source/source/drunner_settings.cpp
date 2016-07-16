#include "drunner_settings.h"
#include "utils.h"
#include "globallogger.h"
#include "drunner_setup.h"
#include "drunner_paths.h"
#include <Poco/Path.h>


// can't put this in header because circular
// dependency then with utils::getTime.
drunnerSettings::drunnerSettings() :
   settingsbash(false)
{
   setString("DRUNNERINSTALLURL", R"EOF(https://drunner.s3.amazonaws.com/drunner)EOF");
   setString("DRUNNERINSTALLTIME", utils::getTime());
   setBool("PULLIMAGES", true);

   mReadOkay = false;   
   readSettings();
}

bool drunnerSettings::readSettings()
{
   mReadOkay = settingsbash::readSettings(drunnerPaths::getPath_drunnerSettings_sh());
   if (!mReadOkay)
   {
      logmsg(kLDEBUG, "Couldn't find settings at " + drunnerPaths::getPath_drunnerSettings_sh().toString());
      return false;
   }

   logmsg(kLDEBUG, "Read settings from " + drunnerPaths::getPath_drunnerSettings_sh().toString());
   return mReadOkay;
}

bool drunnerSettings::writeSettings() const
{
   return settingsbash::writeSettings(drunnerPaths::getPath_drunnerSettings_sh());
}



