#include "ddev.h"
#include "globalcontext.h"
#include "utils.h"
#include "dassert.h"

ddev::ddev() : pluginhelper("ddev")
{
   addConfig("TAG", "The docker image tag (e.g. drunner/helloworld)", "", kCF_string, true);
   addConfig("DSERVICE", "Whether this is a dService [true/false]", "true", kCF_bool, true);
   addConfig("DSERVICENAME", "The name to install the dService as (blank = don't install)", "", kCF_string, false);
}

std::string ddev::getName() const
{
   return "ddev";
}

cResult ddev::runCommand(const CommandLine & cl, const variables & v) const
{
   switch (s2i(cl.command.c_str()))
   {
   case (s2i("build")):
   {
      Poco::Path dockerfile(Poco::Path::current());
      dockerfile.makeDirectory().setFileName("Dockerfile");
      if (!utils::fileexists(dockerfile))
         dockerfile.setFileName("dockerfile");
      if (!utils::fileexists(dockerfile))
         return cError("Couldn't find a dockerfile in the current directory.");

      CommandLine operation;
      operation.command = "docker";
      operation.args = { "build","-t",v.getVal("TAG"),"." };
      int rval = utils::runcommand_stream(operation, kORaw, dockerfile.parent());
      if (rval != 0)
         return cError("Build failed.");
      logmsg(kLINFO, "Built " + v.getVal("TAG"));
      return kRSuccess;
   }
   default:
      return cError("Unrecognised command " + cl.command);
   }
}

cResult ddev::showHelp() const
{
   std::string help = R"EOF(
NAME
   ddev

DESCRIPTION
   A dRunner plugin which makes it easier to develop dServices.

SYNOPSIS
   ddev [COMMAND] [ARGS] ...

   Operates in current directory.

COMMANDS
   ddev help
   ddev configure [OPTION=[VALUE]]

   ddev build
   ddev info
   ddev check
   ddev test

CONFIGURATION OPTIONS
   IMAGENAME        name of docker image to tag build as
   DSERVICE         whether it's a dService (true/false)
   DSERVICENAME     name of dservice to install as after builds, or blank
)EOF";

   logmsg(kLINFO, help);

   return kRSuccess;
}

Poco::Path ddev::configurationFilePath() const
{
   Poco::Path cfp = Poco::Path::current();
   cfp.makeDirectory();
   drunner_assert(cfp.isDirectory(), "Current directory is a file.");
   cfp.setFileName("ddev.json");
   return cfp;
}



//
//
//bool isrepo(const std::string & d, std::string & branch)
//{
//   CommandLine cl("git", { "rev-parse", "--abbrev-ref","HEAD",d });
//   int r = runcommand(cl, branch);
//   // drop everything after branch name
//   branch.erase(branch.find_first_of("\r\n "));
//   logmsg(kLDEBUG, "Branch:           " + branch);
//   return (r == 0);
//}
//
//void build(const std::string & thedir)
//{
//   std::string pwd = (thedir.length()>0 ? thedir : getPWD());
//   std::string dfile = pwd + "/Dockerfile";
//   if (!fileexists(dfile)) dfile = pwd + "/dockerfile";
//   if (!fileexists(dfile)) logmsg(kLERROR, "No Dockerfile in " + pwd + ", it's not a valid dService.");
//
//   // read in service settings.
//   //sh_ddev dd;
//   //if (!dd.readSettings(dd.getPathFromParent(pwd)))
//   //   logmsg(kLERROR,"No dService found at "+pwd);
//
//   //std::string baseimagename=dd.buildname;
//   //std::string branchedimagename=dd.buildname;
//
//   //std::string branch;
//   //if (isrepo(pwd,branch))
//   //   {
//   //   if (!stringisame(branch,"master")) {
//   //   logmsg(kLDEBUG,"Branch is "+branch);
//   //      branchedimagename+=":"+branch;}
//   //   logmsg(kLDEBUG, "Full name:        "+branchedimagename);
//   //   }
//
//   //// build it
//   //CommandLine cl("docker", { "build", "-t",branchedimagename,pwd });
//   //std::string oout;
//   //int r = runcommand(cl, oout, kRC_Defaults);
//   //if (r!=0) logmsg(kLERROR,"docker build faild.\n"+ oout);
//}
