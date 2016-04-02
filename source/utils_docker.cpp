
#include "logmsg.h"
#include "utils.h"
#include "utils_docker.h"

namespace utils_docker
{

   void pullImage(const params & p, const sh_drunnercfg & s, const std::string & image)
   {
      if (p.isDevelopmentMode())
      {
         logmsg(kLDEBUG, "In developer mode - not pulling " + image, p);
         return;
      }

      if (s.getPullImages())
      {
         logmsg(kLINFO, "Pulling Docker image " + image + ".\n This may take some time...", p);
         eResult rslt = utils::pullimage(image);
         if (rslt == kRError)
            logmsg(kLERROR, "Couldn't pull " + image, p);
         if (rslt == kRNoChange)
         {
            if (utils::imageisbranch(image))
               logmsg(kLDEBUG, "No change to Docker image (it's not on the master branch, so assuming dev environment).", p);
            else
               logmsg(kLDEBUG, "No change to Docker image (it's already up to date).", p);
         }
         else
            logmsg(kLINFO, "Successfully pulled " + image, p);
      }
   }


} // namespace