#include <fstream>
#include <Poco/DirectoryIterator.h>
#include <Poco/String.h>

#include "ddev.h"
#include "globalcontext.h"
#include "utils.h"
#include "dassert.h"
#include "drunner_paths.h"
#include "service_manage.h"
#include "timez.h"
#include "sourcecopy.h"
#include "utils_docker.h"

ddev::ddev()
{
}

std::string ddev::getName() const
{
   return "ddev";
}

cResult ddev::runCommand(const CommandLine & cl) const
{
   if (cl.command.length() == 0)
   { // default
      logmsg(kLINFO, "Building tree (default settings).");
      return _ddevtree(cl, Poco::Path::current());
   }

   switch (s2i(cl.command.c_str()))
   {
      case (s2i("test")):
      {
         if (cl.args.size() == 0)
            fatal("ddev test requires the service name of an installed dService.\n  ddev test SERVICENAME");
         return _test(cl.args[0]);
      }
      case (s2i("help")):
         return _showHelp();

      case (s2i("build")):
      {
         std::string servicename;
         if (cl.args.size() > 0)
            servicename = cl.args[0];
         return _ddevtree(cl, Poco::Path::current(), servicename);
      }

      default:
         break;
   }
   return cError("Unknown ddev command: " + cl.command);
}

cResult ddev::_showHelp() const
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
   ddev
      install the dService in the current folder,
      includes building docker images in any subfolders.

   ddev help
      this help   

   ddev test SERVICENAME
      test an already installed dService 

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

//cResult ddev::_build(const CommandLine & cl, Poco::Path d) const
//{
//   std::string imagename = v.getVal("TAG");
//
//   if (imagename.length() == 0)
//      return cError("You need to configure ddev with a tag first.");
//
//   d.makeDirectory().setFileName("Dockerfile");
//   if (!utils::fileexists(d))
//      d.setFileName("dockerfile");
//   if (!utils::fileexists(d))
//      return cError("Couldn't find a dockerfile in the current directory.");
//
//   CommandLine operation;
//   operation.command = "docker";
//   operation.args = { "build","-t",imagename,"." };
//   int rval = utils::runcommand_stream(operation, kORaw, d.parent(), {},NULL);
//   if (rval != 0)
//      return cError("Build failed.");
//   logmsg(kLINFO, "Built " + imagename);
//
//   return rval;
//
//   //dservicename = timeutils::getDateTimeStr();
//   //r += _testcommand(CommandLine("drunner", { "-d","install",".",dservicename }));
//
//}


void ddev::_buildtree(const Poco::Path d) const
{
   drunner_assert(d.isDirectory(), "d not a directory:  "+d.toString());

   Poco::DirectoryIterator di(d), end;

   // process all subdirectories
   while (di != end)
   {
      if (di->isDirectory())
      {
         Poco::Path child(di->path());
         child.makeDirectory(); // it's a directory.
         _buildtree(child);
      }
      ++di;
   }

   // not a docker definition.
   if (!utils::fileexists(d, "dockerfile") && !utils::fileexists(d, "Dockerfile"))
      return;

   std::string tag = load_ddev(d);
   if (tag.length()==0)
   {
      logmsg(kLINFO, " ");
      logmsg(kLINFO, "Dockerfile found, but no corresponding ddev file. Skipping:");
      logmsg(kLINFO, " " + d.toString());
      logmsg(kLINFO, "(Create the ddev file containg the tag to auto build this image)");
      return;
   }

   logmsg(kLINFO, "Building " + tag + "...");
   CommandLine operation;
   operation.command = "docker";
   operation.args = { "build","-t",tag,"." };
   int rval = utils::runcommand_stream(operation, kORaw, d, {}, NULL);
   if (rval != 0)
      fatal("Build failed.");
   logmsg(kLINFO, "Built " + tag + ".");
}

std::string ddev::load_ddev(const Poco::Path d) const
{
   Poco::Path ddev(d);
   ddev.setFileName("ddev");
   if (!utils::fileexists(ddev))
      ddev.setFileName(".ddev");
   if (!utils::fileexists(ddev))
      return "";

   // read ddev file to see tag for docker container.
   std::ifstream t(ddev.toString());
   std::stringstream buffer;
   buffer << t.rdbuf();
   std::string tag = buffer.str();
   Poco::trimInPlace(tag);
   return tag;
}

cResult ddev::_ddevtree(const CommandLine & cl, Poco::Path d, std::string servicename) const
{
   cResult rval;

   // build all docker images contained in tree.
   _buildtree(d);

   // find service.lua
   Poco::Path luaparent(d);
   rval = sourcecopy::getServiceLuaParent(luaparent);
   if (!rval.success())
   {
      logmsg(kLINFO, "Couldn't find a dService, exiting.");
      return kRSuccess;
   }

   // then process the service itself.
   std::string dservicename = servicename;
   if (dservicename.length()==0)
      dservicename = load_ddev(luaparent);
   if (dservicename.length() == 0)
      dservicename = load_ddev(d);
   if (dservicename.length() == 0)
   {
      logmsg(kLINFO, " ");
      logmsg(kLINFO, "Service.lua found, but no corresponding ddev file and no servicename specified.");
      logmsg(kLINFO, "(Create the ddev file containing the service name to install the dService)");
      return rval;
   }

   servicePaths sp(dservicename);
   if (utils::fileexists(sp.getPathdService()))
   {
      logmsg(kLINFO, "Uninstalling old " + dservicename);
      cResult r= service_manage::uninstall(dservicename);
      if (r.error())
         return cError("Failed to uninstall " + dservicename+"\n"+r.what());
   }

   logmsg(kLINFO, "Installing as " + dservicename);
   std::string imagename = ".";
   GlobalContext::getParams()->setDevelopmentMode(true);
   cResult r = service_manage::install(dservicename, imagename);
   if (!r.success())
      return cError("Failed to install " + dservicename);

   // intalled! 
   logmsg(kLINFO, dservicename + " successfully installed.");
   logmsg(kLINFO, "Run   ddev test " + dservicename + "   to test it.");
   return rval;
}

// ----------------------------------------------------------------------------------

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

cResult ddev::_test(std::string dservicename) const
{
   if (dservicename.length() == 0)
      return cError("ddev test requires an installed dService to test!");

   cResult r;
   r = _testcommand(dservicename, { "help" });

   if (r.success())
      r += _testcommand(dservicename, { "configure" });

   utils::tempfolder scratch(drunnerPaths::getPath_Temp().pushDirectory("test-" + dservicename));
   Poco::Path bfile = scratch.getpath().setFileName("backupfile.bak");
   std::string tempservice = dservicename + "__TEMP_TEST";

   try
   {
      if (r.error()) fatal(r.what());

      r += _testcommand(CommandLine("drunner", { "backup",dservicename,bfile.toString() }));
      if (r.error()) fatal(r.what());

      r += _testcommand(CommandLine("drunner", { "restore",bfile.toString(),tempservice }));
      if (r.error()) fatal(r.what());

      r += _testcommand(tempservice, { "help" });
      if (r.error()) fatal(r.what());

      r += _testcommand(tempservice, { "configure" });
      if (r.error()) fatal(r.what());

      r += _testcommand(tempservice, { "selftest" });
      if (r.error()) fatal(r.what());

   }
   catch (const eExit & e)
   {
      logmsg(kLWARN, "Test failed.");
   }

   logmsg(kLINFO, "Tidying up (obliterating " + tempservice + ")");
   cResult rr = _testcommand(CommandLine("drunner", { "obliterate", tempservice }));
   if (rr.error())
      logmsg(kLWARN, rr.what());

   if (r.success())
      logmsg(kLINFO, "Result: Tests on " + dservicename + " succeeded.");
   else
      logmsg(kLERROR, "Result: Tests on "+dservicename+" failed.");

   return r;
}

