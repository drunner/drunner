#include "dproxy.h"
#include "drunner_paths.h"

dproxy::dproxy() : configuredplugin("dproxy")
{
}

std::string dproxy::getName() const
{
   return "dproxy";
}

cResult dproxy::runCommand(const CommandLine & cl, const variables & v) const
{
   switch (s2i(cl.command.c_str()))
   {
   case (s2i("update")):
   {
      return update();
   }
   case (s2i("start")):
   {
      return start();
   }
   case (s2i("stop")):
   {
      return stop();
   }
   default:
      return cError("Unrecognised command " + cl.command);
   }
}

cResult dproxy::runHook(std::string hook, std::vector<std::string> hookparams, const servicelua::luafile * lf, const serviceVars * sv) const
{
   if (lf!=NULL && lf->getProxies().size()==0) // no proxy, so changes to this dService don't matter for us.
      return kRNoChange;

   switch (s2i(hook.c_str()))
   {
   case (s2i("configure_end")):
      if (hookparams.size() == 0)
         return kRNoChange;
      // fall through
   case (s2i("install_end")):
   case (s2i("update_end")):
   case (s2i("uninstall_end")):
   case (s2i("obliterate_end")):
   {
      logmsg(kLINFO, "Reconfiguring dproxy.");
      return update();
   }

   default:
      return kRNoChange;
   }

   return kRNotImplemented;
}


cResult dproxy::showHelp() const
{
   std::string help = R"EOF(
NAME
   dproxy

DESCRIPTION
   A dRunner plugin which makes it easy to map virtual hosts to services.

COMMANDS
   dproxy start
   dproxy stop
   dproxy update
   dproxy addcert SERVICENAME CERTIFICATE.PEM
)EOF";

   logmsg(kLINFO, help);

   return kRSuccess;
}

Poco::Path dproxy::configurationFilePath() const
{
   return drunnerPaths::getPath_Root().setFileName("dproxy.json");
}

cResult dproxy::update() const
{
   return cResult();
}

cResult dproxy::start() const
{
   return cResult();
}

cResult dproxy::stop() const
{
   return cResult();
}
