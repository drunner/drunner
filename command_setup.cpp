#include <boost/filesystem.hpp>

#include "command_setup.h"
#include "utils.h"
#include "drunner_settings.h"

int command_setup(params & p)
{
   std::string SUPPORTIMAGE="drunner/install-support";
   std::string ROOTUTILIMAGE="drunner/install-rootutils";
   std::string DRUNNERINSTALLURL="https://raw.githubusercontent.com/drunner/install/master/drunner-install";

   if (p.mArgs.size()<1)
      utils::die(p,"Usage:\n   drunner setup ROOTPATH",1);

   std::string rootpath = utils::getabsolutepath(p.mArgs[0]);
   if (rootpath.length()==0)
      utils::die(p,"Couldn't determine path for "+p.mArgs[0]);

   if (p.mOMode == om_verbose)
      std::cout << "Setting up to directory "<<rootpath << std::endl;

   if (!utils::mkdirp(rootpath))
      utils::die(p,"Couldn't create directory "+rootpath);

   rootpath = utils::getcanonicalpath(rootpath);

   drunner_settings settings(rootpath);
   if (!settings.writeSettings())
      utils::die(p,"Couldn't write settings file!");
   
   return 0;
}
