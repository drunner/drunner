#include "sh_drunnercfg.h"
#include "utils.h"

// can't put this in header because circular
// dependency then with utils::getTime.
sh_drunnercfg::sh_drunnercfg(const std::string & rootpath) :
   settingsbash(false)
{
   setString("ROOTPATH", rootpath);
   setString("ROOTUTILIMAGE", "drunner/rootutils");
   setString("DRUNNERINSTALLURL", R"EOF(https://drunner.s3.amazonaws.com/drunner-install)EOF");
   setString("DRUNNERINSTALLTIME", utils::getTime());
   setBool("PULLIMAGES", true);
}
bool sh_drunnercfg::readSettings()
{
   bool readokay = settingsbash::readSettings(getPath_drunnercfg_sh());
   if (!readokay)
      return false;

   // migrate old settings.
   if (utils::findStringIC(getRootUtilImage(), "install-rootutils"))
   {
      setString("ROOTUTILIMAGE", "drunner/rootutils");
      if (!writeSettings())
         ; // couldn't migrate settings.
   }

   return readokay;
}

bool sh_drunnercfg::writeSettings()
{
   return settingsbash::writeSettings(getPath_drunnercfg_sh());
}
