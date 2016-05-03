#include <sys/stat.h>

#include "command_general.h"
#include "logmsg.h"
#include "enums.h"
#include "utils.h"
#include "utils_docker.h"
#include "command_setup.h"
#include "exceptions.h"

namespace command_general
{

   void showservices(const params & p, const sh_drunnercfg & settings)
   {
      std::string parent = settings.getPath_dServices();
      std::vector<std::string> services;
      if (!utils::getFolders(parent,services))
         logmsg(kLERROR,"Couldn't get subfolders of "+parent,p);
      if (services.size()>0)
      {
         logmsg(kLINFO,"Installed dServices:",p);
         for (auto const & entry : services)
            logmsg(kLINFO,"  "+entry,p);
      } else
         logmsg(kLERROR,"No dServices are installed.",p);
   }


   void clean(const params & p, const sh_drunnercfg & settings)
   {
      std::string op;
      utils_docker::pullImage(p, settings, "spotify/docker-gc");

      logmsg(kLINFO,"Cleaning.",p);
      if (utils::bashcommand("docker run --rm -v /var/run/docker.sock:/var/run/docker.sock spotify/docker-gc",op) != 0)
         logmsg(kLERROR,"Unable to run spotify/docker-gc to clean docker images.",p);

      logmsg(kLINFO,"Cleaning is complete.",p);
   }

}
