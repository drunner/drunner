#include "command_services.h"
#include "logmsg.h"
#include "utils.h"

namespace command_services
{


   void servicecmd(const params & p, const sh_drunnercfg & settings)
   {
      std::string servicename = p.getArgs()[0];
      std::string reservedwords = "install backupstart backupend backup restore update enter uninstall obliterate";

      if (p.numArgs() < 2 || utils::stringisame(p.getArgs()[0], "help"))
         logmsg(kLERROR, "E_NOTIMPL", p); // todo: show service help!

      std::string command = p.getArgs()[1];
      if (utils::findStringIC(reservedwords, command))
         logmsg(kLERROR, command + " is a reserved word. You might want  drunner " + command + " " + servicename + "  instead.", p);

      std::string cmd(settings.getPath_Services() + "/" + servicename + "/drunner/servicerunner");
      std::vector<std::string> c;
      c.push_back("servicerunner");
      for (uint i = 1; i < p.getArgs().size(); ++i)
         c.push_back(p.getArgs()[i]);

      std::string lmsg;
      for (auto &entry : c)
         lmsg += "[" + entry + "] ";
      logmsg(kLDEBUG, lmsg, p);

      utils::bashcommand(cmd, c);
   }

   void asdf(const params & p, const sh_drunnercfg & settings)
   {
      logmsg(kLERROR, R"EOF(

                     /-------------------------------------------------------------\
            |    That command isn't fully implemented and I am sad. :,(   |
            \-------------------------------------------------------------/
      )EOF", p);
   }



}
