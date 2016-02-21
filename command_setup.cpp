#include <boost/filesystem.hpp>

#include "command_setup.h"
#include "utils.h"
#include "drunner_settings.h"

int command_setup(const params::params & p)
{
   std::string SUPPORTIMAGE="drunner/install-support";
   std::string ROOTUTILIMAGE="drunner/install-rootutils";
   std::string DRUNNERINSTALLURL="https://raw.githubusercontent.com/drunner/install/master/drunner-install";

   if (p.getArgs().size()<1)
      utils::die(p,"Usage:\n   drunner setup ROOTPATH",1);

   std::string rootpath = utils::getabsolutepath(p.getArgs()[0]);
   if (rootpath.length()==0)
      utils::die(p,"Couldn't determine path for "+p.getArgs()[0]);

   if (p.isVerbose())
      std::cout << "Setting up to directory "<<rootpath << std::endl;

   if (!utils::mkdirp(rootpath))
      utils::die(p,"Couldn't create directory "+rootpath);

   rootpath = utils::getcanonicalpath(rootpath);

   drunner_settings settings(rootpath);
   if (!settings.writeSettings())
      utils::die(p,"Couldn't write settings file!");
   
   // move this executable to the directory.
   int result = rename( utils::get_exefullpath().c_str(), (rootpath+"/drunner").c_str());
   if (result!=0)
      utils::die(p,"Couldn't move drunner executable to "+rootpath+".");
   
   return 0;
}
