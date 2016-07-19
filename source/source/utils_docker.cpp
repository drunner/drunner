#include <algorithm>
#include <iterator>

#include "basen.h"
#include "utils.h"
#include "utils_docker.h"
#include "globalcontext.h"
#include "globallogger.h"

namespace utils_docker
{

   static std::vector<std::string> S_PullList;


   void createDockerVolume(std::string name)
   {
      std::vector<std::string> args = { "volume","create","--name=" + name };
      std::string op;
      int rval = utils::runcommand("docker", args, op, 0);
      if (rval != 0)
         logmsg(kLERROR, "Unable to create docker volume " + name);
      logmsg(kLDEBUG, "Created docker volume " + name);
   }


   void pullImage(const std::string & image)
   {
#ifdef _DEBUG
      logmsg(kLDEBUG, "DEBUG BUILD - not pulling");
      return;
#endif

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
         S_PullList.push_back(image);
         logmsg(kLINFO, "Successfully pulled " + image);
         break;
      }
   }

   cResult runBashScriptInContainer(std::string data, std::string imagename, std::string & op)
   {
      std::string encoded_data;
      bn::encode_b64(data.begin(), data.end(), std::back_inserter(encoded_data));
      int n = encoded_data.length() % 4;
      if (n == 2) encoded_data += "==";
      if (n == 3) encoded_data += "=";
      poco_assert(n != 1);

      std::string command = "docker";
      std::vector<std::string> args = { "run","--rm",imagename,"/bin/bash","-c",
         "echo " + encoded_data + " | base64 -d > /tmp/_script ; /bin/bash /tmp/_script" };

      int rval=utils::runcommand(command, args, op, true);
      if (rval == 0)
         return kRSuccess;
      return kRError;
   }

   bool dockerVolExists(const std::string & vol)
   {
      std::vector<std::string> args = { "volume","ls","-f","name="+vol };
      std::string out;
      int rval = utils::runcommand("docker", args,out,true);
      if (rval != 0)
         return false;
      return utils::wordmatch(out, vol);
   }

} // namespace