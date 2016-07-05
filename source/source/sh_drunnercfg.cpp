#include "sh_drunnercfg.h"
#include "utils.h"
#include "globallogger.h"

// can't put this in header because circular
// dependency then with utils::getTime.
sh_drunnercfg::sh_drunnercfg(const Poco::Path & rootpath) :
   settingsbash(false)
{
   setString("ROOTPATH", rootpath.toString());
   setString("ROOTUTILIMAGE", "drunner/rootutils");
   setString("DRUNNERINSTALLURL", R"EOF(https://drunner.s3.amazonaws.com/drunner-install)EOF");
   setString("DRUNNERINSTALLTIME", utils::getTime());
   setBool("PULLIMAGES", true);

   mReadOkay = false;
   readSettings();
}

bool sh_drunnercfg::readSettings()
{
   mReadOkay = settingsbash::readSettings(getPath_drunnercfg_sh());
   if (!mReadOkay)
      return false;

   // migrate old settings.
   if (utils::findStringIC(getRootUtilImage(), "install-rootutils"))
   {
      setString("ROOTUTILIMAGE", "drunner/rootutils");
      if (!writeSettings())
         fatal("Couldn't migrate old dRunner settings."); // couldn't migrate settings.
   }
   return mReadOkay;
}

bool sh_drunnercfg::writeSettings() const
{
   return settingsbash::writeSettings(getPath_drunnercfg_sh());
}

Poco::Path sh_drunnercfg::getPath_Root() const { 
   Poco::Path rootpath(getString("ROOTPATH"));
   rootpath.makeDirectory();
   poco_assert(rootpath.isDirectory());
   return rootpath;
}
