#include "command_dev.h"
#include "globallogger.h"
#include "globalcontext.h"
#include "settingsbash.h"
#include "utils.h"
#include "sh_ddev.h"

namespace command_dev
{

using namespace utils;


   bool isrepo(const std::string & d,std::string & branch)
   {
      std::vector<std::string> args = { "rev-parse", "--abbrev-ref","HEAD",d };
      int r = runcommand("git", args, branch, true);
      // drop everything after branch name
      branch.erase( branch.find_first_of("\r\n ") );
      logmsg(kLDEBUG,"Branch:           "+branch);
      return (r==0);
   }

   void build(const std::string & thedir)
   {
      std::string pwd=(thedir.length()>0 ? thedir : getPWD());
      std::string dfile=pwd+"/Dockerfile";
      if (!fileexists(dfile)) dfile=pwd+"/dockerfile";
      if (!fileexists(dfile)) logmsg(kLERROR,"No Dockerfile in "+pwd+", it's not a valid dService.");

      // read in service settings.
      sh_ddev dd;
      if (!dd.readSettings(dd.getPathFromParent(pwd)))
         logmsg(kLERROR,"No dService found at "+pwd);

      std::string baseimagename=dd.buildname;
      std::string branchedimagename=dd.buildname;

      std::string branch;
      if (isrepo(pwd,branch))
         {
         if (!stringisame(branch,"master")) {
         logmsg(kLDEBUG,"Branch is "+branch);
            branchedimagename+=":"+branch;}
         logmsg(kLDEBUG, "Full name:        "+branchedimagename);
         }

      // build it
      std::vector<std::string> args = { "build", "-t",branchedimagename,pwd };
      std::string oout;
      int r = runcommand("docker", args, oout, false);
      if (r!=0) logmsg(kLERROR,"docker build faild.\n"+ oout);
   }



}
