#include "validateimage.h"
#include "utils.h"
#include "drunner_paths.h"
#include "globallogger.h"

namespace validateImage
{


   void validate(std::string imagename)
   {
      if (!utils::fileexists(drunnerPaths::getPath_Root()))
         logmsg(kLERROR, "ROOTPATH not set.");

      if (utils::imageisbranch(imagename))
         logmsg(kLDEBUG, imagename + " looks like a development branch (won't be pulled).");
      else
         logmsg(kLDEBUG, imagename + " should be a production image.");

      std::string op;
      std::vector<std::string> args = {
         "run",
         "--rm",
         "-v",
         drunnerPaths::getPath_Support().toString() + ":/support",
         imagename ,
         "/support/validator-image"
      };
      int rval = utils::runcommand("docker", args, op, false);

      if (rval != 0)
      {
         if (utils::findStringIC(op, "Unable to find image"))
            logmsg(kLERROR, "Couldn't find image " + imagename);
         else
            logmsg(kLERROR, op);
      }

#ifdef _WIN32
      logmsg(kLINFO, "[Y] " + imagename + " is dRunner compatible.");
#else
      logmsg(kLINFO, "\u2714  " + imagename + " is dRunner compatible.");
#endif      
   }

}