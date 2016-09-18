#include <Poco/DirectoryIterator.h>
#include <Poco/String.h>

#include "ddev.h"
#include "globalcontext.h"
#include "utils.h"
#include "dassert.h"
#include "drunner_paths.h"
#include "service_manage.h"

ddev::ddev()
{
   addConfig("TAG", "The docker image tag (e.g. drunner/helloworld)", "", kCF_string, true,true);
   addConfig("DSERVICE", "Whether this is a dService [true/false]", "true", kCF_bool, true,true);
   addConfig("DSERVICENAME", "The name to install the dService as (blank = don't install)", "", kCF_string, false,true);
}

std::string ddev::getName() const
{
   return "ddev";
}

cResult ddev::runCommand(const CommandLine & cl, persistvariables & v) const
{
   switch (s2i(cl.command.c_str()))
   {
      case (s2i("build")):
      {
         return _build(cl, v.getVariables(), Poco::Path::current());
      }
      case (s2i("buildtree")):
      {
         return _buildtree(cl, v.getVariables(), Poco::Path::current());
      }
      case (s2i("test")):
      {
         return _test(cl, v.getVariables());
      }
      default:
         return cError("Unrecognised command " + cl.command);
   }
}

cResult ddev::runHook(std::string hook, std::vector<std::string> hookparams, const servicelua::luafile * lf, const serviceVars * sv) const
{
   return kRNoChange;
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
   std::string imagename = v.getVal("TAG");
   std::string dservicename = v.getVal("DSERVICENAME");

   if (imagename.length() == 0)
      return cError("You need to configure ddev with a tag first.");

   d.makeDirectory().setFileName("Dockerfile");
   if (!utils::fileexists(d))
      d.setFileName("dockerfile");
   if (!utils::fileexists(d))
      return cError("Couldn't find a dockerfile in the current directory.");

   CommandLine operation;
   operation.command = "docker";
   operation.args = { "build","-t",imagename,"." };
   int rval = utils::runcommand_stream(operation, kORaw, d.parent(), {},NULL);
   if (rval != 0)
      return cError("Build failed.");
   logmsg(kLINFO, "Built " + imagename);

   // install
   if (dservicename.length() > 0)
   {
      logmsg(kLINFO, "Installing " + imagename + " as " + dservicename);
      service_manage::uninstall(dservicename);
      rval+=service_manage::install(dservicename, imagename, true); // don't pull images.
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
      persistvariables pv(getName(), ddevjson, mConfiguration);
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

   if (r.success())
      r += _testcommand(dservicename, { "configure" });

   utils::tempfolder scratch(drunnerPaths::getPath_Temp().pushDirectory("test-" + dservicename));
   Poco::Path bfile = scratch.getpath().setFileName("backupfile.bak");
   std::string tempservice = dservicename + "__TEMP_TEST";

   if (r.success())
      r += _testcommand(CommandLine("drunner", { "backup",dservicename,bfile.toString() }));
   
   if (r.success())
      r += _testcommand(CommandLine("drunner", { "restore",bfile.toString(),tempservice }));
   
   if (r.success())
      r += _testcommand(tempservice, { "help" });
   
   if (r.success())
      r += _testcommand(tempservice, { "configure" });

   if (!r.success())
      logmsg(kLWARN, "Previous test failed. Obliterating then logging failure message.");

   cResult rr = _testcommand(CommandLine("drunner", { "obliterate", tempservice }));
   if (r.success())
      r += rr;

   if (r.success())
      logmsg(kLINFO, "Tests completed successfully.");
   else
      logmsg(kLERROR, "Tests failed: " + r.what());

   return r;
}
