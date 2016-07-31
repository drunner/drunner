#include "command_dev.h"
#include "globallogger.h"
#include "globalcontext.h"
#include "utils.h"

namespace command_dev
{

using namespace utils;


   bool isrepo(const std::string & d,std::string & branch)
   {
      CommandLine cl("git", { "rev-parse", "--abbrev-ref","HEAD",d });
      int r = runcommand(cl, branch, kRC_Defaults);
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
      //sh_ddev dd;
      //if (!dd.readSettings(dd.getPathFromParent(pwd)))
      //   logmsg(kLERROR,"No dService found at "+pwd);

      //std::string baseimagename=dd.buildname;
      //std::string branchedimagename=dd.buildname;

      //std::string branch;
      //if (isrepo(pwd,branch))
      //   {
      //   if (!stringisame(branch,"master")) {
      //   logmsg(kLDEBUG,"Branch is "+branch);
      //      branchedimagename+=":"+branch;}
      //   logmsg(kLDEBUG, "Full name:        "+branchedimagename);
      //   }

      //// build it
      //CommandLine cl("docker", { "build", "-t",branchedimagename,pwd });
      //std::string oout;
      //int r = runcommand(cl, oout, kRC_Defaults);
      //if (r!=0) logmsg(kLERROR,"docker build faild.\n"+ oout);
   }



}
