#include "command_dev.h"
#include "utils.h"
#include "logmsg.h"
#include "settingsbash.h"

namespace command_dev
{

using namespace utils;


   bool isrepo(const params & p, const std::string & d,std::string & branch)
   {
      int r=bashcommand("git rev-parse --abbrev-ref HEAD "+d+" 2>/dev/null",branch);
      // drop everything after branch name
      branch.erase( branch.find_first_of("\r\n ") );
      logmsg(kLDEBUG,"Branch:           "+branch,p);
      return (r==0);
   }

   void build(const params & p, const sh_drunnercfg & settings,const std::string & thedir)
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
         if (!stringisame(branch,"master")) {
         logmsg(kLDEBUG,"Branch is "+branch,p);
            branchedimagename+=":"+branch;}
         logmsg(kLDEBUG, "Full name:        "+branchedimagename,p);
         }

      // build it
      std::string op;
      int r=bashcommand("docker build -t "+branchedimagename+" "+pwd,op);
      if (r!=0) logmsg(kLERROR,"docker build faild.\n"+op,p);

   }



}
