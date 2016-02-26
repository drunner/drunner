#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <cstdio>

#include "drunner_settings.h"
#include "utils.h"


drunner_settings::drunner_settings(std::string rootpath) :
   settingsbash(rootpath+"/"+"config.sh")
{
//   std::cerr << "Setting defaults."<<std::endl;
   // set defaults.
   setSetting("ROOTPATH",rootpath);
   setSetting("ROOTUTILIMAGE","drunner/install-rootutils");
   setSetting("DRUNNERINSTALLURL",R"EOF(https://drunner.s3.amazonaws.com/drunner-install)EOF");
   setSetting("DRUNNERINSTALLTIME",utils::getTime());
   setSetting("PULLIMAGES","yes");
   mRead=readSettings();
}
