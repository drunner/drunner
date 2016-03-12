#include <unistd.h>

#include "service.h"
#include "logmsg.h"
#include "sh_variables.h"
#include "utils.h"

service::service(const params & prms, const sh_drunnercfg & settings, const std::string & servicename, std::string imagename /*= ""*/) :
   mName(servicename),
   mImageName(imagename),
   mSettings(settings),
   mParams(prms)
{
   if (utils::fileexists(getPathVariables()))
   {
      if (mImageName.length() == 0)
      { // if imagename override not provided and variables.sh exists then load it from variables.sh
         sh_variables shv(getPathVariables());
         if (!shv.readOkay())
            logmsg(kLERROR, "Couldn't read " + getPathVariables());
         mImageName = shv.getImageName();
      }
      else
         logmsg(kLERROR, "Coding logic error - ImageName provided as override to service class, but the service already exists with a variable.sh file.");
   }
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

void service::setImageName(std::string imagename)
{
   mImageName = imagename;
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
      utils::bashcommand(getPathServiceRunner(), cargs, true,true);
      return;
   }

   std::string command = mParams.getArgs()[1];
   std::string reservedwords = "install backupstart backupend backup restore update enter uninstall obliterate";
   if (utils::findStringIC(reservedwords, command))
      logmsg(kLERROR, command + " is a reserved word. You might want  drunner " + command + " " + mName + "  instead.");

   for (uint i = 1; i < mParams.getArgs().size(); ++i)
      cargs.push_back(mParams.getArgs()[i]);

   std::string lmsg;
   for (auto &entry : cargs)
      lmsg += "[" + entry + "] ";
   logmsg(kLDEBUG, lmsg);

   utils::bashcommand(getPathServiceRunner(), cargs,mParams);
}

void service::update()
{ // update the service (recreate it)
   recreate(true);
}

const params & service::getParams() const
{
   return mParams;
}

const std::string service::getImageName() const
{
   return mImageName;
}

void service::enter()
{
   execl(getPathServiceRunner().c_str(), "servicerunner", "enter", NULL);
}

int service::status()
{
   if (!utils::fileexists(getPath()))
   {
      logmsg(kLINFO, getName() + " is not installed.");
      return 1;
   }
   if (!isValid())
   {
      logmsg(kLINFO, getName() + " is not a valid service. Try  drunner recover "+getName());
      return 1;
   }
   logmsg(kLINFO, getName() + " is installed and valid.");
   return 0;
}
