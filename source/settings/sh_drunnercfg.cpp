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

bool sh_drunnercfg::readSettings(std::string settingspath)
{
   bool readokay = settingsbash::readSettings(settingspath);
   if (!readokay)
      return false;

   // migrate old settings.
   if (utils::findStringIC(getRootUtilImage(), "install-rootutils"))
   {
      setString("ROOTUTILIMAGE", "drunner/rootutils");
      if (!writeSettings(settingspath))
         ; // couldn't migrate settings.
   }
}

bool sh_drunnercfg::readSettings()
{
   return settingsbash::readSettings(getPath());
}
bool sh_drunnercfg::writeSettings()
{
   return settingsbash::writeSettings(getPath());
}

}