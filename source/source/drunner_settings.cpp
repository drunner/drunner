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
drunnerSettings::drunnerSettings() 
{
   mVariables.setVal("DRUNNERINSTALLURL", R"EOF(https://drunner.s3.amazonaws.com/drunner)EOF");
   mVariables.setVal("DRUNNERINSTALLTIME", utils::getTime());
   mVariables.setVal("PULLIMAGES", "true");

   mReadOkay = false;   
   cResult r = readSettings();
   switch (r) {
   case kRSuccess:
      mReadOkay = true;
      logdbg("Read dRunner settings from " + drunnerPaths::getPath_drunnerSettings_json().toString());
      break;
   case kRError:
      fatal("The settings file is corrupt and could not be read: " + drunnerPaths::getPath_drunnerSettings_json().toString() + "\nSuggest deleting it.");
   default:
      logdbg("Couldn't read settings file from " + drunnerPaths::getPath_drunnerSettings_json().toString());
   }
}

cResult drunnerSettings::readSettings()
{
   Poco::Path spath = drunnerPaths::getPath_drunnerSettings_json();
   if (!utils::fileexists(spath))
      return kRNoChange;

   std::ifstream is(spath.toString());
   if (is.bad())
      return kRError;
      
   try
   {
      cereal::JSONInputArchive archive(is);
      archive(mVariables);
   }
   catch (const cereal::Exception &)
   {
      return kRError;
   }
   return kRSuccess;
}

cResult drunnerSettings::writeSettings() const
{
   Poco::Path spath = drunnerPaths::getPath_drunnerSettings_json();
   std::ofstream os(spath.toString());
   if (os.bad() || !os.is_open())
      return kRError;

   try
   {
      cereal::JSONOutputArchive archive(os);
      archive(mVariables);
   }
   catch (const cereal::Exception &)
   {
      return kRError;
   }
   return kRSuccess;
}



