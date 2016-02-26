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

      ddevsh(const params & p, std::string pwd) : settingsbash(pwd+"/ddev.sh")
      {
         setSetting("BUILDNAME","undefined");
         setSettingb("DSERVICE",false);
         setSetting("DSERVICENAME","undefined");

         isdService=readSettings();
         buildname=getSetting("BUILDNAME");
         dservicename=getSetting("DSERVICENAME");
         if (isdService)
         {
            logmsg(kLDEBUG, "DIRECTORY:        "+pwd,p);
            logmsg(kLDEBUG, "DDEV COMPATIBLE:  yes",p);
            logmsg(kLDEBUG, "BUILDNAME:        "+buildname,p);
            logmsg(kLDEBUG, "DSERVICENAME:     "+dservicename,p);
         }
      } // ctor
   }; //class

   bool isrepo(const params & p, const std::string & d,std::string branch)
   {
      int r=bashcommand("git rev-parse --abbrev-ref HEAD "+d+" 2>/dev/null",branch);
      return (r==0);
   }

   void build(const params & p, const drunner_settings & settings,const std::string & thedir)
   {
      std::string pwd=(thedir.length()>0 ? thedir : getPWD());
      std::string dfile=pwd+"/Dockerfile";
      if (!fileexists(dfile)) dfile=pwd+"/dockerfile";
      if (!fileexists(dfile)) logmsg(kLERROR,"No Dockerfile in "+pwd+", it's not a valid dService.",p);

      // read in service settings.
      ddevsh dd(p,pwd);
      if (!dd.isdService) logmsg(kLERROR,"No dService found at "+pwd,p);

      std::string baseimagename=dd.buildname;
      std::string branchedimagename=dd.buildname;

      std::string branch;
      if (isrepo(p,pwd,branch))
         {
         if (!stringisame(branch,"master"))
            branchedimagename+=":"+branch;
         logmsg(kLDEBUG, "FULLNAME:         "+branchedimagename,p);
         }

      // build it
      std::string op;
      int r=bashcommand("docker build -t "+branchedimagename+" "+pwd,op);
      if (r!=0) logmsg(kLERROR,"docker build faild.\n"+op,p);

   }



}
