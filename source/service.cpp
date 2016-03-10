#include "service.h"
#include "logmsg.h"
#include "sh_variables.h"
#include "utils.h"
#include "command_install.h"

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

std::string service::getPathServiceCfg() const
{
   return getPathdRunner() + "/servicecfg.sh";
}

std::string service::getName() const
{
   return mName;
}

void service::setName(const std::string & servicename)
{
   mName = servicename;
}

void service::ensureDirectoriesExist() const
{
   // create service's drunner and temp directories on host.
   utils::makedirectory(getPath(), mParams, S_755);
   utils::makedirectory(getPathdRunner(), mParams, S_777);
   utils::makedirectory(getPathTemp(), mParams, S_777);
}

bool service::isValid() const
{
   if (!utils::fileexists(getPath()) || !utils::fileexists(getPathdRunner()) || !utils::fileexists(getPathTemp()))
      return false;

   return true;
}

void service::servicecmd()
{
   std::vector<std::string> cargs;
   cargs.push_back("servicerunner");

   if (mParams.numArgs() < 2 || utils::stringisame(mName, "help"))
   {
      cargs.push_back("help");
      utils::bashcommand(getPathServiceRunner(), cargs);
      return;
   }

   std::string command = mParams.getArgs()[1];
   std::string reservedwords = "install backupstart backupend backup restore update enter uninstall obliterate";
   if (utils::findStringIC(reservedwords, command))
      logmsg(kLERROR, command + " is a reserved word. You might want  drunner " + command + " " + mName + "  instead.", mParams);

   for (uint i = 1; i < mParams.getArgs().size(); ++i)
      cargs.push_back(mParams.getArgs()[i]);

   std::string lmsg;
   for (auto &entry : cargs)
      lmsg += "[" + entry + "] ";
   logmsg(kLDEBUG, lmsg, mParams);

   utils::bashcommand(getPathServiceRunner(), cargs);
}

void service::update()
{ // update the service (recreate it)
   sh_variables vars(mParams, *this);
   std::string imagename = vars.getImageName();

   command_install::recreateService(mParams, mSettings, imagename, *this, true);
}

