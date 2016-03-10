#include <sys/stat.h>

#include "command_general.h"
#include "logmsg.h"
#include "enums.h"
#include "utils.h"
#include "command_setup.h"
#include "exceptions.h"

namespace command_general
{

   void showservices(const params & p, const sh_drunnercfg & settings)
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


   void clean(const params & p, const sh_drunnercfg & settings)
   {
      std::string op;
      logmsg(kLINFO,"Pulling latest spotify/docker-gc.",p);
      if (utils::pullimage("spotify/docker-gc")==kRError)
         logmsg(kLERROR,"Failed to pull spotify/docker-gc");

      logmsg(kLINFO,"Cleaning.",p);
      if (utils::bashcommand("docker run --rm -v /var/run/docker.sock:/var/run/docker.sock spotify/docker-gc",op) != 0)
         logmsg(kLERROR,"Unable to run spotify/docker-gc to clean docker images.",p);

      logmsg(kLINFO,"Cleaning is complete.",p);
   }

   void update(const params & p, const sh_drunnercfg & settings)
   {

      logmsg(kLERROR,"Soon!",p);
   }


   void service(const params & p, const sh_drunnercfg & settings)
   {
      std::string servicename = p.getArgs()[0];
      std::string reservedwords = "install backupstart backupend backup restore update enter uninstall obliterate";

      if (p.numArgs() < 2 || utils::stringisame(p.getArgs()[0], "help"))
         logmsg(kLERROR, "E_NOTIMPL", p); // todo: show service help!

      std::string command = p.getArgs()[1];
      if (utils::findStringIC(reservedwords, command))
         logmsg(kLERROR, command + " is a reserved word. You might want  drunner " + command + " " + servicename + "  instead.", p);

      std::string cmd( settings.getPath_Services() + "/" + servicename + "/drunner/servicerunner");
      std::vector<std::string> c;
      c.push_back("servicerunner");
      for (uint i = 1; i < p.getArgs().size();++i)
         c.push_back( p.getArgs()[i] );

      std::string lmsg;
      for (auto &entry : c)
         lmsg += "[" + entry + "] ";
      logmsg(kLDEBUG, lmsg , p);

      utils::bashcommand(cmd,c);
   }

   void asdf(const params & p, const sh_drunnercfg & settings)
   {
      logmsg(kLERROR,R"EOF(

            /-------------------------------------------------------------\
            |    That command isn't fully implemented and I am sad. :,(   |
            \-------------------------------------------------------------/
      )EOF",p);
   }


}
