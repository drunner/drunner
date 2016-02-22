#include "command_general.h"
#include "logmsg.h"
#include "utils.h"

namespace command_general
{

   void showservices(const params & p, const drunner_settings & settings)
   {
      std::string parent = settings.getPath_Services();
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


   void clean(const params & p, const drunner_settings settings)
   {
      std::string op;
      logmsg(kLINFO,"Pulling latest spotify/docker-gc.",p);
      utils::pullimage("spotify/docker-gc");

      logmsg(kLINFO,"Cleaning.",p);
      if (utils::bashcommand("docker run --rm -v /var/run/docker.sock:/var/run/docker.sock spotify/docker-gc",op) != 0)
         logmsg(kLERROR,"Unable to run spotify/docker-gc to clean docker images.",p);

      logmsg(kLINFO,"Cleaning is complete.",p);
   }

   void update(const params & p, const drunner_settings settings)
   {
      
      logmsg(kLERROR,"Soon!",p);
   }


   void asdf(const params & p, const drunner_settings settings)
   {
      logmsg(kLERROR,R"EOF(

            /-------------------------------------------------------------\
            |    That command isn't fully implemented and I am sad. :,(   |
            \-------------------------------------------------------------/
      )EOF",p);   
   }

}