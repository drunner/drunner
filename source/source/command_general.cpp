#include <sys/stat.h>

#include "command_general.h"
#include "globallogger.h"
#include "globalcontext.h"
#include "enums.h"
#include "utils.h"
#include "utils_docker.h"
#include "drunner_setup.h"
#include "exceptions.h"

namespace command_general
{

   void showservices()
   {
      std::vector<std::string> services;
      utils::getAllServices(services);
      if (services.size()>0)
      {
         logmsg(kLINFO,"Installed dServices:");
         for (auto const & entry : services)
            logmsg(kLINFO,"  "+entry);
      } else
         logmsg(kLERROR,"No dServices are installed.");
   }


   void clean()
   {
      std::string op;
      utils_docker::pullImage("spotify/docker-gc");

      logmsg(kLINFO,"Cleaning.");
      CommandLine cl("docker", { "run","--rm","-v","/var/run/docker.sock:/var/run/docker.sock","spotify/docker-gc" });
      if (utils::runcommand_stream(cl,kORaw)!=0)
         logmsg(kLERROR,"Unable to run spotify/docker-gc to clean docker images.");

      logmsg(kLINFO,"Cleaning is complete.");
   }

}
