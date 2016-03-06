#include "sh_drunnercfg.h"
#include "utils.h"

// can't put this in header because circular
// dependency then with utils::getTime.
sh_drunnercfg::sh_drunnercfg(const params & p, std::string rootpath) :
   settingsbash(p,rootpath+"/"+"drunnercfg.sh")
{
   setSetting(sbelement("ROOTPATH",rootpath));
   setSetting(sbelement("ROOTUTILIMAGE","drunner/install-rootutils"));
   setSetting(sbelement("DRUNNERINSTALLURL",R"EOF(https://drunner.s3.amazonaws.com/drunner-install)EOF"));
   setSetting(sbelement("DRUNNERINSTALLTIME",utils::getTime()));
   setSetting(sbelement("PULLIMAGES","yes"));
   mRead=readSettings();
}
