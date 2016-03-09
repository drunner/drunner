#include "sh_drunnercfg.h"
#include "utils.h"

// can't put this in header because circular
// dependency then with utils::getTime.
sh_drunnercfg::sh_drunnercfg(const params & p, std::string rootpath) :
   settingsbash(p,rootpath+"/"+"drunnercfg.sh")
{
   setString("ROOTPATH",rootpath);
   setString("ROOTUTILIMAGE","drunner/install-rootutils");
   setString("DRUNNERINSTALLURL",R"EOF(https://drunner.s3.amazonaws.com/drunner-install)EOF");
   setString("DRUNNERINSTALLTIME",utils::getTime());
   setBool("PULLIMAGES",true);
   mRead=readSettings();
}
