#include "sh_drunnercfg.h"
#include "utils.h"

// can't put this in header because circular
// dependency then with utils::getTime.
sh_drunnercfg::sh_drunnercfg(const params & p, std::string rootpath) :
   settingsbash(p,rootpath+"/"+"drunnercfg.sh")
{
   setSetting(sb_string("ROOTPATH",rootpath));
   setSetting(sb_string("ROOTUTILIMAGE","drunner/install-rootutils"));
   setSetting(sb_string("DRUNNERINSTALLURL",R"EOF(https://drunner.s3.amazonaws.com/drunner-install)EOF"));
   setSetting(sb_string("DRUNNERINSTALLTIME",utils::getTime()));
   setSetting(sb_bool("PULLIMAGES",true));
   mRead=readSettings();
}
