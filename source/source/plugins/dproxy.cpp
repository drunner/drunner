#include "dproxy.h"
#include "drunner_paths.h"

dproxy::dproxy() : pluginhelper("dproxy")
{
   addConfig("ENABLED", "Whether the proxy service (HAProxy) is enabled.", "false", kCF_bool, true);
}

std::string dproxy::getName() const
{
   return "dproxy";
}

cResult dproxy::runCommand(const CommandLine & cl, const variables & v) const
{
   return cResult();
}

cResult dproxy::showHelp() const
{
   std::string help = R"EOF(
NAME
   dproxy

DESCRIPTION
   A dRunner plugin which makes it easy to map virtual hosts to services.

COMMANDS
   dproxy configure enabled=[true/false]

)EOF";

   logmsg(kLINFO, help);

   return kRSuccess;
}

Poco::Path dproxy::configurationFilePath() const
{
   return drunnerPaths::getPath_Root().setFileName("dproxy.json");
}
