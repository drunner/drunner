#include "sh_drunnercfg.h"
#include "utils.h"

// can't put this in header because circular
// dependency then with utils::getTime.
sh_drunnercfg::sh_drunnercfg(const params & p, std::string rootpath) :
   settingsbash(p,rootpath+"/"+"drunnercfg.sh")
{
   setSetting("ROOTPATH",rootpath);
   setSetting("ROOTUTILIMAGE","drunner/install-rootutils");
   setSetting("DRUNNERINSTALLURL",R"EOF(https://drunner.s3.amazonaws.com/drunner-install)EOF");
   setSetting("DRUNNERINSTALLTIME",utils::getTime());
   setSetting("PULLIMAGES","yes");
   mRead=readSettings();
}
