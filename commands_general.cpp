#include "commands_general.h"
#include "logmsg.h"
#include "utils.h"

namespace commands_general
{

   void showservices(const params & p, const drunner_settings settings)
   {
      logmsg(kLERROR,R"EOF(

            /-------------------------------------------------------------\
            |    That command isn't fully implemented and I am sad. :,(   |
            \-------------------------------------------------------------/
   )EOF",p);   
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

}