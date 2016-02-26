#include "command_dev.h"
#include "utils.h"
#include "logmsg.h"
#include "settingsbash.h"

namespace command_dev
{

using namespace utils;

   class ddevsh : public settingsbash
   {
   public:
      std::string buildname, dservicename;
      bool isdService;

      ddevsh(std::string pwd) : settingsbash(pwd+"/ddev.sh")
      {
         setsetting("BUILDNAME","undefined");
         setsettingb("DSERVICE",false);
         setsetting("DSERVICENAME","undefined");

         isdService=readsettings();
         buildname=getSetting("BUILDNAME");
         dservicename=getSetting("DSERVICENAME");
      }

   }



   void build(const params & p, const drunner_settings & settings)
   {
      std::string pwd=getPWD();
      std::string dfile=pwd+"/Dockerfile";
      if (!fileexists(dfile)) dfile=pwd+"/dockerfile";
      if (!fileexists(dfile)) logmsg(kLERROR,"No Dockerfile in "+pwd+", it's not a valid dService.",p);
      std::string op,branchedimagename;
      int r=bashcommand("docker build -t "+branchedimagename+" .",op);

      // read in service settings.
      ddevsh dd(pwd);
      if (!dd.isdService) logmsg(kLERROR,"No dService found at "+pwd,p);
   }



}
