
#include "utils.h"
#include "utils_docker.h"
#include "globalcontext.h"
#include "globallogger.h"

namespace utils_docker
{

   static std::vector<std::string> S_PullList;


   void createDockerVolume(std::string name)
   {
      std::vector<std::string> args = { "volume","create","--name=\"" + name + "\"" };
      int rval = utils::runcommand("docker", args);
      if (rval != 0)
         logmsg(kLERROR, "Unable to create docker volume " + name);
      logmsg(kLDEBUG, "Created docker volume " + name);
   }


   void pullImage(const std::string & image)
   {
      if (GlobalContext::getParams()->isDevelopmentMode())
      {
         logmsg(kLDEBUG, "In developer mode - not pulling " + image);
         return;
      }

      if (!GlobalContext::getSettings()->getPullImages())
      {
         logmsg(kLDEBUG, "Pulling images disabled in global options.");
         return;
      }

      if (utils::imageisbranch(image))
      {
         logmsg(kLDEBUG, image+" is not on the master branch, so assuming dev environment and not pulling.");
         return;
      }

      if (std::find(S_PullList.begin(), S_PullList.end(), image) != S_PullList.end())
      { // pulling is slow. Never do it twice in one command.
         logmsg(kLDEBUG, "Already pulled " + image + " so not pulling again.");
         return;
      }


      logmsg(kLINFO, "Pulling Docker image " + image + ".\n This may take some time...");
      eResult rslt = utils::pullimage(image);

      switch (rslt) 
      {
      case kRError:
         logmsg(kLERROR, "Couldn't pull " + image);
         break;
      case kRNoChange:
         logmsg(kLDEBUG, "No change to Docker image (it's already up to date).");
         break;
      default:
         logmsg(kLINFO, "Successfully pulled " + image);
      }
   }


   bool dockerVolExists(const std::string & vol)
   { // this could be better - will match substrings rather than whole volume name. :/ 
     // We name with big unique names so unlikely to be a problem for us.
      logmsg(kLERROR, "can't use bashcommand!");
      int rval = utils::bashcommand("docker volume ls | grep \"" + vol + "\"");
      return (rval == 0);
   }

} // namespace