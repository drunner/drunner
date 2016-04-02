#include <unistd.h>

#include "service.h"
#include "logmsg.h"
#include "utils.h"
#include "servicehook.h"
#include "sh_servicevars.h"
#include "drunnercompose.h"

service::service(const params & prms, const sh_drunnercfg & settings, const std::string & servicename, std::string imagename /*= ""*/) :
   servicepaths(settings,servicename),
   mImageName( loadImageName(prms,settings,servicename,imagename) ),
   mParams(prms),
   mEnvironment(*this)
{
}

servicepaths::servicepaths(const sh_drunnercfg & settings, const std::string & servicename) :
   mName(servicename),
   mSettings(settings)
{
}


std::string service::loadImageName(const params & prms, const sh_drunnercfg & settings, const std::string & servicename, std::string imagename)
{
   std::string v = servicepaths(settings, servicename).getPath();
   if (utils::fileexists(v))
   {
      if (imagename.length() == 0)
      { // if imagename override not provided and variables.sh exists then load it from variables.sh
         sh_servicevars shv(v);
         if (!shv.readOkay())
         {
            ::logmsg(kLWARN, "Couldn't read servicevars.sh from " + v, prms);
            ::logmsg(kLWARN, "Service old/broken. Recover with:", prms);
            ::logmsg(kLWARN, "   drunner recover " + servicename + " IMAGENAME", prms);
            ::logmsg(kLERROR, "Unable to determine the image name. Exiting.", prms);
         }
         imagename = shv.getImageName();
      }
      else
         ::logmsg(kLDEBUG, "Forcing dService main image to be " + imagename,prms);
   }

   if (imagename.length() == 0)
      ::logmsg(kLERROR, "Couldn't determine imagename.",prms);

   return imagename;
}


std::string servicepaths::getPath() const
{
   return mSettings.getPath_dServices() + "/" + mName;
}

std::string servicepaths::getPathdRunner() const
{
   return getPath() + "/" + "drunner";
}

std::string servicepaths::getPathTemp() const
{
   return getPath() + "/" + "temp";
}

std::string servicepaths::getPathHostVolume() const
{
   return mSettings.getPath_HostVolumes() + "/" + mName;
}

std::string servicepaths::getPathHostVolume_servicerunner() const
{
   return getPathHostVolume() + "/servicerunner";
}

std::string servicepaths::getPathHostVolume_environment() const
{
   return getPathHostVolume() + "/environment";
}

std::string servicepaths::getPathServiceRunner() const
{
   return getPathdRunner() + "/servicerunner";
}

std::string servicepaths::getPathDockerCompose() const
{
   return getPathdRunner() + "/docker-compose.yml";
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
   utils::makedirectory(getPathHostVolume_environment(), mParams, S_700);
   utils::makedirectory(getPathHostVolume_servicerunner(), mParams, S_777);
}

bool service::isValid() const
{
   if (!utils::fileexists(getPath()) || !utils::fileexists(getPathdRunner()) || !utils::fileexists(getPathTemp()))
      return false;

   drunnerCompose drc(*this, mParams);
   if (drc.readOkay()==kRError)
   {
      logmsg(kLDEBUG, "docker-compose.yml is broken for service "+getName());
      return false;
   }
   sh_servicevars shv(getPath());
   if (!shv.readOkay())
   {
      logmsg(kLDEBUG, "Couldn't read servicevars.sh from service " + getName());
      return false;
   }

   return true;
}

cResult service::servicecmd()
{
   if (mParams.numArgs() < 1)
      fatal("Programming error - number of arguments should never be 0 in service::servicecmd");

   std::vector<std::string> cargs( mParams.getArgs().begin() + 1, mParams.getArgs().end() );

   if (mParams.numArgs() < 2 || utils::stringisame(mName, "help"))
   {
      cargs.push_back("help");
      serviceRunnerCommand(cargs);
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

   servicehook hook(this, "servicecmd", cargs, mParams);
   hook.starthook();

   cResult rval( serviceRunnerCommand(cargs) );

   hook.endhook();

   return rval;
}

void service::update()
{ // update the service (recreate it)
   servicehook hook(this, "update", mParams);
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
   servicehook hook(this, "enter", mParams);
   hook.starthook();

   execl(getPathServiceRunner().c_str(), "servicerunner", "enter", NULL);
}

int service::status()
{
   servicehook hook(this, "status", mParams);
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


cResult service::serviceRunnerCommand(const std::vector<std::string> & args) const
{
   // set environment.
   drunnerCompose drc(*this, mParams);
   if (drc.readOkay()==kRError)
      logmsg(kLWARN, "Couldn't set up drunnerCompose. servicerunner will not have environment set correctly.");
   else
      drc.setServiceRunnerEnv();
 
   if (args.size() > 0 && utils::stringisame(args[0],"servicerunner"))
      logmsg(kLERROR, "Programming error - someone gave serviceRunnerCommand the first arg: servicerunner :/");
   auto newargs(args);
   newargs.insert(newargs.begin(), std::string("servicerunner"));

   cResult rval(utils::dServiceCmd(getPathServiceRunner(), newargs, mParams, true));
   return rval;
}

cServiceEnvironment & service::getEnvironment()
{
   return mEnvironment;
}

const cServiceEnvironment & service::getEnvironmentConst() const
{
   return mEnvironment;
}

cServiceEnvironment::cServiceEnvironment(const servicepaths & paths) :
   settingsbash(true)
{
   mPath = paths.getPathHostVolume_environment() + "/servicerunner_env.sh";
   if (utils::fileexists(mPath))
   {
      if (!readSettings(mPath))
         fatal("servicerunner_env.sh is corrupt.");
   }
}

void cServiceEnvironment::save_environment(std::string key, std::string value)
{
   setString(key, value);
   if (!writeSettings(mPath))
      fatal("Failed to write environment file " + mPath);
}

int cServiceEnvironment::getNumVars() const
{
   return mElements.size();
}

void cServiceEnvironment::getVar(const int position, std::string & key, std::string & value) const
{
   if (position >= getNumVars())
      fatal("cServiceEnvironment::getVar - position out of range.");
   auto const & el(mElements.at(position));
   const auto b = dynamic_cast<const sb_string *>(mElements.at(position).get());
   if (!b) 
      fatal("Couldn't interpret environment element as string.");
   key = b->getKey();
   value = b->get();
}
