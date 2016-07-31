#include <algorithm>
#include <iterator>

#include <Poco/String.h>

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
      CommandLine cl("docker", { "volume","create","--name=" + name });
      std::string op;
      int rval = utils::runcommand(cl, op);
      if (rval != 0)
         logmsg(kLERROR, "Unable to create docker volume " + name);
      logmsg(kLDEBUG, "Created docker volume " + name);
   }

   void stopContainer(std::string name)
   {
      CommandLine cl("docker", { "stop",name });
      std::string op;
      int rval = utils::runcommand(cl, op);
      if (rval != 0)
         logmsg(kLERROR, "Unable to stop docker container " + name+"\n"+op);
      logmsg(kLDEBUG, "Stopped docker container " + name);
   }

   void removeContainer(std::string name)
   {
      CommandLine cl("docker", { "rm",name });
      std::string op;
      int rval = utils::runcommand(cl, op);
      if (rval != 0)
         logmsg(kLERROR, "Unable to remove docker container " + name + "\n" + op);
      logmsg(kLDEBUG, "Removed docker container " + name);
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
         logmsg(kLDEBUG, "Successfully pulled " + image);
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

      CommandLine cl("docker", {"run","--rm",imagename,"/bin/bash","-c",
         "echo " + encoded_data + " | base64 -d > /tmp/_script ; /bin/bash /tmp/_script"});

      int rval=utils::runcommand(cl, op);
      return (rval == 0 ? kRSuccess : kRError);
   }

   bool dockerVolExists(const std::string & vol)
   {
      CommandLine cl("docker", { "volume","ls","-f","name=" + vol });
      std::string out;
      int rval = utils::runcommand(cl,out);
      return (rval != 0 ? false : utils::wordmatch(out, vol));
   }


   bool dockerContainerExists(const std::string & container)
   {
      CommandLine cl("docker", { "ps","-f","name=" + container });
      std::string out;
      int rval = utils::runcommand(cl, out);
      return (rval != 0 ? false : utils::wordmatch(out, container));
   }

} // namespace