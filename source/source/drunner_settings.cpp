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

   if (r.success())
   {
      mReadOkay = true;
      logdbg("Read dRunner settings from " + drunnerPaths::getPath_drunnerSettings_json().toString());
   } 
   else if (r.error())
      fatal("The settings file is corrupt and could not be read: " + drunnerPaths::getPath_drunnerSettings_json().toString() + "\nSuggest deleting it.");
   else
      logdbg("Couldn't read settings file from " + drunnerPaths::getPath_drunnerSettings_json().toString());
}

cResult drunnerSettings::readSettings()
{
   Poco::Path spath = drunnerPaths::getPath_drunnerSettings_json();
   if (!utils::fileexists(spath))
      return kRNoChange;

   std::ifstream is(spath.toString());
   if (is.bad())
      return cError("Unable to open " + spath.toString() + " for reading.");
      
   try
   {
      cereal::JSONInputArchive archive(is);
      archive(mVariables);
   }
   catch (const cereal::Exception & e)
   {
      return cError("Cereal exception on reading settings: " + std::string(e.what()));
   }
   return kRSuccess;
}

cResult drunnerSettings::writeSettings() const
{
   Poco::Path spath = drunnerPaths::getPath_drunnerSettings_json();
   std::ofstream os(spath.toString());
   if (os.bad() || !os.is_open())
      return cError("Unable to open "+spath.toString()+" for writing.");

   try
   {
      cereal::JSONOutputArchive archive(os);
      archive(mVariables);
   }
   catch (const cereal::Exception & e)
   {
      return cError("Cereal exception on writing settings: "+std::string(e.what()));
   }
   return kRSuccess;
}



