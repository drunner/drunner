#include <unistd.h>

#include "service.h"
#include "logmsg.h"
#include "sh_variables.h"
#include "utils.h"
#include "servicehook.h"

service::service(const params & prms, const sh_drunnercfg & settings, const std::string & servicename, std::string imagename /*= ""*/) :
   servicepaths(settings,servicename),
   mImageName( loadImageName(prms,settings,servicename,imagename) ),
   mParams(prms)
{
}

servicepaths::servicepaths(const sh_drunnercfg & settings, const std::string & servicename) :
   mName(servicename),
   mSettings(settings)
{
}


std::string service::loadImageName(const params & prms, const sh_drunnercfg & settings, const std::string & servicename, std::string imagename)
{
   std::string v = servicepaths(settings, servicename).getPathVariables();
   if (utils::fileexists(v))
   {
      if (imagename.length() == 0)
      { // if imagename override not provided and variables.sh exists then load it from variables.sh
         sh_variables shv(v);
         if (!shv.readOkay())
            ::logmsg(kLERROR, "Couldn't read " + v,prms);
         imagename = shv.getImageName();
      }
      else
         ::logmsg(kLDEBUG, "variables.sh exists, but forcing imagename to be " + imagename,prms);
   }

   if (imagename.length() == 0)
      ::logmsg(kLERROR, "Couldn't determine imagename.",prms);

   return imagename;
}


std::string servicepaths::getPath() const
{
   return mSettings.getPath_Services() + "/" + mName;
}

std::string servicepaths::getPathdRunner() const
{
   return getPath() + "/" + "drunner";
}

std::string servicepaths::getPathTemp() const
{
   return getPath() + "/" + "temp";
}

std::string servicepaths::getPathServiceRunner() const
{
   return getPathdRunner() + "/servicerunner";
}

std::string servicepaths::getPathVariables() const
{
   return getPathdRunner() + "/variables.sh";
}

std::string servicepaths::getPathServiceCfg() const
{
   return getPathdRunner() + "/servicecfg.sh";
}

std::string servicepaths::getName() const
{
   return mName;
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

cResult service::servicecmd()
{
   std::vector<std::string> cargs( mParams.getArgs().begin(), mParams.getArgs().end() );
   cargs[0] = "servicerunner";

   if (mParams.numArgs() < 2 || utils::stringisame(mName, "help"))
   {
      cargs.push_back("help");
      utils::dServiceCmd(getPathServiceRunner(), cargs, mParams,true);
      return kRSuccess;
   }

   std::string command = mParams.getArgs()[1];
   std::string reservedwords = " install backupstart backupend backup restore update enter uninstall obliterate recover ";
   if (utils::findStringIC(reservedwords, " " + command + " ")) // spaces are to ensure whole word match.
   {
      if (command != "enter")
         logmsg(kLERROR, command + " is a reserved word. You might want  drunner " + command + " " + mName + "  instead.");
      enter(); // uses execl so never returns.
      logmsg(kLERROR, "Should never get here. Enter command shouldn't return.");
   }

   std::vector<std::string> hookargs(cargs.begin() + 1, cargs.end());
   servicehook hook(this, "servicecmd", hookargs, mParams);
   hook.starthook();

   cResult rval( utils::dServiceCmd(getPathServiceRunner(), cargs, mParams,true) );

   hook.endhook();

   return rval;
}

void service::update()
{ // update the service (recreate it)
   tVecStr args;
   servicehook hook(this, "update", args, mParams);
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
   tVecStr args;
   servicehook hook(this, "enter", args, mParams);
   hook.starthook();

   execl(getPathServiceRunner().c_str(), "servicerunner", "enter", NULL);
}

int service::status()
{
   tVecStr args;
   servicehook hook(this, "status", args, mParams);
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



void validateImage(const params & prms, const sh_drunnercfg & settings, std::string imagename)
{
   if (!utils::fileexists(settings.getPath_Root())) logmsg(kLERROR, "ROOTPATH not set.",prms);

   std::string op;
   int rval = utils::bashcommand(
      "docker run --rm -v \"" + settings.getPath_Support() +
      ":/support\" \"" + imagename + "\" /support/validator-image 2>&1", op);

   if (rval != 0)
   {
      if (utils::findStringIC(op, "Unable to find image"))
         logmsg(kLERROR, "Couldn't find image " + imagename,prms);
      else
         logmsg(kLERROR, op,prms);
   }
   logmsg(kLINFO, "\u2714  " + imagename + " is dRunner compatible.",prms);
}
