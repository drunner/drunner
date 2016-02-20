#include "setup.h"
#include "utils.h"

int setup(params & p)
{
   if (p.mArgs.size()<1)
      utils::die(p,"Usage:\n   drunner setup ROOTPATH",1);

   return 0;
}
