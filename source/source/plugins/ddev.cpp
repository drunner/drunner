#include <Poco/DirectoryIterator.h>
#include <Poco/String.h>

#include "ddev.h"
#include "globalcontext.h"
#include "utils.h"
#include "dassert.h"
#include "drunner_paths.h"
#include "service_install.h"

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
         return _build(cl, v, Poco::Path::current());
      }
      case (s2i("buildtree")):
      {
         return _buildtree(cl, v, Poco::Path::current());
      }
      case (s2i("test")):
      {
         return _test(cl, v);
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

cResult ddev::_build(const CommandLine & cl, const variables & v,Poco::Path d) const
{
   if (v.getVal("TAG").length() == 0)
      return cError("You need to configure ddev with a tag first.");

   d.makeDirectory().setFileName("Dockerfile");
   if (!utils::fileexists(d))
      d.setFileName("dockerfile");
   if (!utils::fileexists(d))
      return cError("Couldn't find a dockerfile in the current directory.");

   CommandLine operation;
   operation.command = "docker";
   operation.args = { "build","-t",v.getVal("TAG"),"." };
   int rval = utils::runcommand_stream(operation, kORaw, d.parent());
   if (rval != 0)
      return cError("Build failed.");
   logmsg(kLINFO, "Built " + v.getVal("TAG"));

   // install
   std::string dservicename = v.getVal("DSERVICENAME");
   if (dservicename.length() > 0)
   {
      logmsg(kLINFO, "Installing " + v.getVal("TAG") + " as " + dservicename);

      service_install si(dservicename, v.getVal("TAG"));
      return si.recover();
   }
   else
      logmsg(kLINFO,"Not a dService so not installing.");
   return rval;
}

cResult ddev::_buildtree(const CommandLine & cl, const variables & v, Poco::Path d) const
{
   cResult rval;
   Poco::DirectoryIterator di(d), end;
   Poco::Path ddevjson;
   bool foundjson = false;

   //logdbg("Checking " + d.toString());

   // process all subdirectories
   while (di != end)
   {
      if (di->isDirectory())
         rval += _buildtree(cl,v,di->path());
      else
         if (di->isFile())
            if (0 == Poco::icompare(Poco::Path(di->path()).getFileName(), "ddev.json"))
            {
               foundjson = true;
               ddevjson = di->path();
            }
      ++di;
   }

   // then process this directory
   if (foundjson)
   {
      logmsg(kLINFO,"Building " + ddevjson.toString());
      persistvariables pv(mName, ddevjson, mConfiguration);
      cResult r = pv.loadvariables();
      if (!r.success())
         return r;
      r = _build(cl, pv.getVariables(),ddevjson.parent());
      if (r.success())
         logmsg(kLINFO, "Successfully built " + ddevjson.toString()+"\n----------------------------------------------------------");
      else
         logmsg(kLERROR, "Build failed for " + ddevjson.toString() + ":\n " + r.what());
      rval += r;
   }

   return rval;
}

cResult _testcommand(CommandLine operation)
{
   std::string out;
   cResult r = kRSuccess;

   operation.logcommand("Running: ",kLINFO);

   int rci = utils::runcommand(operation, out);
   if (0 != rci && 127 != rci)
      r = cError(out);
   return r;
}

cResult _testcommand(std::string dservicename, std::vector<std::string> args)
{
   CommandLine cl;
   cl.command = "drunner";
   cl.args = { "servicecmd",dservicename };
   cl.args.insert(cl.args.end(),args.begin(),args.end());
   return _testcommand(cl);
}

cResult ddev::_test(const CommandLine & cl, const variables & v) const
{
   std::string dservicename;
   if (cl.args.size() == 0)
   {
      dservicename = v.getVal("DSERVICENAME");
      if (dservicename.length() == 0)
         return cError("You need to configure ddev with a tag first.");
   }
   else
      dservicename = cl.args[0];

   cResult r;
   r += _testcommand(dservicename, { "help" });
   r += _testcommand(dservicename, { "configure" });

   utils::tempfolder scratch(drunnerPaths::getPath_Temp().pushDirectory("test-" + dservicename));
   Poco::Path bfile = scratch.getpath().setFileName("backupfile.bak");
   std::string tempservice = dservicename + "__TEMP_TEST";

   r += _testcommand(CommandLine("drunner", { "backup",dservicename,bfile.toString() }));
   r += _testcommand(CommandLine("drunner", { "restore",bfile.toString(),tempservice }));
   r += _testcommand(tempservice, { "help" });
   r += _testcommand(tempservice, { "configure" });
   r += _testcommand(CommandLine("drunner", { "obliterate", tempservice }));

   if (r.success())
      logmsg(kLINFO, "Tests completed successfully.");
   else
      logmsg(kLERROR, "Tests failed: " + r.what());

   return r;
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
