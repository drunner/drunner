#include "params.h"
#include "sh_drunnercfg.h"
#include "service.h"
#include "utils.h"
#include "logmsg.h"

service::service(const std::string & servicename, const sh_drunnercfg & settings, const params & prms) :
   mName(servicename),
   mSettings(settings),
   mParams(prms)
{

}

std::string service::getPath() const
{
   return mSettings.getPath_Services() + "/" + mName;
}

std::string service::getPathdRunner() const
{
   return getPath() + "/" + "drunner";
}

std::string service::getPathTemp() const
{
   return getPath() + "/" + "temp";
}

std::string service::getPathServiceRunner() const
{
   return getPathdRunner() + "/servicerunner";
}

std::string service::getPathVariables() const
{
   return getPathdRunner() + "/variables.sh";
}

std::string service::getName() const
{
   return mName;
}

void service::setName(const std::string & servicename)
{
   mName = servicename;
}

void service::ensureDirectoriesExist()
{
   // create service's drunner and temp directories on host.
   utils::makedirectory(getPathdRunner(), mParams, S_777);
   utils::makedirectory(getPathTemp(), mParams, S_777);
}

bool service::isValid() const
{
   if (!utils::fileexists(getPath()))
      return false;
   if (!utils::fileexists(getPathdRunner()))
      return false;
   if (!utils::fileexists(getPathTemp()))
      return false;

   return true;
}

void service::servicecmd()
{
   if (mParams.numArgs() < 2 || utils::stringisame(mName, "help"))
      logmsg(kLERROR, "E_NOTIMPL", mParams); // todo: show service help!

   std::string command = mParams.getArgs()[1];
   std::string reservedwords = "install backupstart backupend backup restore update enter uninstall obliterate";
   if (utils::findStringIC(reservedwords, command))
      logmsg(kLERROR, command + " is a reserved word. You might want  drunner " + command + " " + mName + "  instead.", mParams);

   std::vector<std::string> c;
   c.push_back("servicerunner");
   for (uint i = 1; i < mParams.getArgs().size(); ++i)
      c.push_back(mParams.getArgs()[i]);

   std::string lmsg;
   for (auto &entry : c)
      lmsg += "[" + entry + "] ";
   logmsg(kLDEBUG, lmsg, mParams);

   utils::bashcommand(getPathServiceRunner(), c);
}

void service::update()
{ // update the service (recreate it).
   logmsg(kLERROR, "E_NOTIMPL", mParams); // todo: show service help!
}

