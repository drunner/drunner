#include <unistd.h>

#include "service.h"
#include "logmsg.h"
#include "sh_variables.h"
#include "utils.h"
#include "servicehook.h"

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
         logmsg(kLDEBUG, "variables.sh exists, but forcing imagename to be " + imagename);
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

   sh_servicecfg svc(getPathServiceCfg());
   if (!svc.readOkay())
   {
      logmsg(kLDEBUG, "Couldn't read servicecfg.sh from service "+getName());
      return false;
   }
   sh_variables shv(getPathVariables());
   if (!shv.readOkay())
   {
      logmsg(kLDEBUG, "Couldn't read variables.sh from service " + getName());
      return false;
   }

   if (svc.getVersion() < 2)
      logmsg(kLDEBUG, "dService "+getImageName()+" is old (version 1) and should be updated.");

   return true;
}

eResult service::servicecmd()
{
   std::vector<std::string> cargs;
   cargs.push_back("servicerunner");

   if (mParams.numArgs() < 2 || utils::stringisame(mName, "help"))
   {
      cargs.push_back("help");
      utils::dServiceCmd(getPathServiceRunner(), cargs, mParams);
      return kRError;
   }

   std::string command = mParams.getArgs()[1];
   std::string reservedwords = "install backupstart backupend backup restore update enter uninstall obliterate";
   if (utils::findStringIC(reservedwords, command))
      logmsg(kLERROR, command + " is a reserved word. You might want  drunner " + command + " " + mName + "  instead.");

   for (uint i = 1; i < mParams.getArgs().size(); ++i)
      cargs.push_back(mParams.getArgs()[i]);

   std::string lmsg;
   for (auto &entry : cargs)
      lmsg += utils::doquote(entry) + " ";
   utils::trim(lmsg);
   logmsg(kLDEBUG, lmsg);

   servicehook hook(this, "servicecmd", lmsg, mParams);
   hook.starthook();

   utils::dServiceCmd(getPathServiceRunner(), cargs, mParams);

   hook.endhook();

   return kRSuccess;
}

void service::update()
{ // update the service (recreate it)
   servicehook hook(this, "update", "", mParams);
   hook.starthook();

   recreate(true);
   
   hook.endhook();
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
   servicehook hook(this, "enter", "", mParams);
   hook.starthook();

   execl(getPathServiceRunner().c_str(), "servicerunner", "enter", NULL);
}

int service::status()
{
   servicehook hook(this, "status", "", mParams);
   hook.starthook();

   if (!utils::fileexists(getPath()))
   {
      logmsg(kLINFO, getName() + " is not installed.");
      hook.endhook();
      return 1;
   }
   if (!isValid())
   {
      logmsg(kLINFO, getName() + " is not a valid service. Try  drunner recover "+getName());
      hook.endhook();
      return 1;
   }
   logmsg(kLINFO, getName() + " is installed and valid.");
   hook.endhook();
   return 0;
}

