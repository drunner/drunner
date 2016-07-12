#include <sstream>

#ifndef _WIN32
#include <unistd.h>
#endif

#include "service.h"
#include "globallogger.h"
#include "utils.h"
#include "servicehook.h"
#include "sh_servicevars.h"
#include "drunner_compose.h"
#include "globalcontext.h"
#include "drunner_paths.h"
#include "service_yml.h"

// --------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------


service::service(const std::string & servicename, std::string imagename /*= ""*/) :
   servicePaths(servicename),
   mImageName( loadImageName(servicename,imagename) ),
   mServiceYml( getPathServiceYml() )
   //mEnvironment(*this)
{
}

servicePaths::servicePaths(const std::string & servicename) :
   mName(servicename)
{
}

std::string service::loadImageName(const std::string & servicename, std::string imagename)
{
   Poco::Path v = servicePaths(servicename).getPath();
   if (utils::fileexists(v))
   {
      if (imagename.length() == 0)
      { // if imagename override not provided and variables.sh exists then load it from variables.sh
         sh_servicevars shv;
         if (!shv.readSettings( shv.getPathFromParent(v) ))
         {
            ::logmsg(kLWARN, "Couldn't read servicevars.sh from " + v.toString());
            ::logmsg(kLWARN, "Service old/broken. Recover with:");
            ::logmsg(kLWARN, "   drunner recover " + servicename + " IMAGENAME");
            ::logmsg(kLERROR, "Unable to determine the image name. Exiting.");
         }
         imagename = shv.getImageName();
      }
      else
         ::logmsg(kLDEBUG, "Forcing dService main image to be " + imagename);
   }

   if (imagename.length() == 0)
      ::logmsg(kLERROR, "Couldn't determine imagename.");

   return imagename;
}


void service::ensureDirectoriesExist() const
{
   // create service's drunner and temp directories on host.
   utils::makedirectory(getPath(), S_755);
   utils::makedirectory(getPathdRunner(), S_777);
   //utils::makedirectory(getPathTemp(), S_777);
   //utils::makedirectories(getPathHostVolume_environment(), S_700);
   //utils::makedirectory(getPathHostVolume_servicerunner(), S_777);
}

bool service::isValid() const
{
   if (!utils::fileexists(getPath()) || !utils::fileexists(getPathdRunner())) // || !utils::fileexists(getPathTemp()))
      return false;

   drunnerCompose drc(*this);
   if (drc.readOkay()==kRError)
   {
      logmsg(kLDEBUG, "docker-compose.yml is broken for service "+getName());
      return false;
   }
   sh_servicevars shv;
   if (!shv.readSettings(getPath()))
   {
      logmsg(kLDEBUG, "Couldn't read servicevars.sh from service " + getName());
      return false;
   }

   return true;
}

cResult service::servicecmd()
{
   const params & p(*GlobalContext::getParams());
   poco_assert(p.numArgs() > 0); // should never have 0 args to servicecmd!

   std::vector<std::string> cargs( p.getArgs().begin() + 1, p.getArgs().end() );

   if (p.numArgs() < 2 || utils::stringisame(mName, "help"))
   {
      cargs.push_back("help");
      serviceRunnerCommand(cargs);
      return kRSuccess;
   }

   std::string command = p.getArgs()[1];
   std::string reservedwords = " install backupstart backupend backup restore update enter uninstall obliterate recover ";
   if (utils::findStringIC(reservedwords, " " + command + " ")) // spaces are to ensure whole word match.
   {
      if (command != "enter")
         logmsg(kLERROR, command + " is a reserved word. You might want  drunner " + command + " " + mName + "  instead.");
      enter(); // uses execl so never returns.
      logmsg(kLERROR, "Should never get here. Enter command shouldn't return.");
   }

   servicehook hook(this, "servicecmd", cargs);
   hook.starthook();

   cResult rval( serviceRunnerCommand(cargs) );

   hook.endhook();

   return rval;
}

void service::update()
{ // update the service (recreate it)
   servicehook hook(this, "update");
   hook.starthook();

   recreate(true);
   
   hook.endhook();
}

const std::string service::getImageName() const
{
   return mImageName;
}

void service::enter()
{
#ifdef _WIN32
   logmsg(kLERROR, "Enter is not implemented on Windows.");
#else
   servicehook hook(this, "enter");
   hook.starthook();

   execl(getPathServiceRunner().toString().c_str(), "servicerunner", "enter", NULL);
#endif
}

int service::status()
{
   servicehook hook(this, "status");
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


cResult service::serviceRunnerCommand(const std::vector<std::string> & args) const
{
   // set environment.
   //drunnerCompose drc(*this);
   //if (drc.readOkay()==kRError)
   //   logmsg(kLWARN, "Couldn't set up drunnerCompose. servicerunner will not have environment set correctly.");
   //else
   //   drc.setServiceRunnerEnv();
   if (!
   serviceyml::file 

   variables

   poco_assert(args.size() == 0 || !utils::stringisame(args[0], "servicerunner")); // first arg shouldn't be 'servicerunner'.

   fatal("Need to switch over to running command via service_yml.");
   //cResult rval(utils::dServiceCmd(getPathServiceRunner().toString(), args, true));
   return kRNotImplemented;
}

//cServiceEnvironment & service::getEnvironment()
//{
//   return mEnvironment;
//}
//
//const cServiceEnvironment & service::getEnvironmentConst() const
//{
//   return mEnvironment;
//}

//-----------------------------------------------------------------------------

cServiceEnvironment::cServiceEnvironment(const servicePaths & paths) :
   settingsbash(true)
{
   fatal("Not implemented : cServiceEnvironment::cServiceEnvironment(const servicePaths & paths)");
   //mPath = paths.getPathHostVolume_environment().setFileName("servicerunner_env.sh");
   //if (utils::fileexists(mPath))
   //{
   //   if (!readSettings(mPath))
   //      fatal("servicerunner_env.sh is corrupt.");
   //}
}

void cServiceEnvironment::save_environment(std::string key, std::string value)
{
   setString(key, value);
   if (!writeSettings(mPath))
      fatal("Failed to write environment file " + mPath.toString());
}

unsigned int cServiceEnvironment::getNumVars() const
{
   return mElements.size();
}

std::string cServiceEnvironment::index2key(unsigned int i) const
{
   if (i >= getNumVars()) fatal("Invalid index into cServiceEnvironment.");
   return mElements[i].getkey();
}

std::string cServiceEnvironment::get_value(const std::string & key) const
{ 
   return getString(key); 
}

service_obliterate::service_obliterate(const std::string & servicename) : servicePaths(servicename)
{
}
