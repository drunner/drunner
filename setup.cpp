#include <boost/filesystem.hpp>

#include "setup.h"
#include "utils.h"

int setup(params & p)
{
   if (p.mArgs.size()<1)
      utils::die(p,"Usage:\n   drunner setup ROOTPATH",1);

   std::string rootpath = utils::getabsolutepath(p.mArgs[0]);
   if (rootpath.length()==0)
      utils::die(p,"Couldn't determine path for "+p.mArgs[0]);
   std::cout << "Setting up to directory "<<rootpath << std::endl;

   utils::createordie(p,rootpath);

   return 0;
}
