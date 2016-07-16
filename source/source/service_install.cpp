#include <sys/stat.h>

#include <Poco/String.h>
#include <Poco/File.h>

#include "command_dev.h"
#include "utils.h"
#include "utils_docker.h"
#include "globallogger.h"
#include "globalcontext.h"
#include "exceptions.h"
#include "settingsbash.h"
#include "drunner_setup.h"
#include "service.h"
#include "servicehook.h"
#include "sh_servicevars.h"
#include "chmod.h"
#include "validateimage.h"
#include "service_install.h"
#include "utils_docker.h"
#include "service.h"
#include "drunner_paths.h"

service_install::service_install(std::string servicename) : servicePaths(servicename)
{
   serviceConfig svc(getPathServiceConfig());
   if (kRSuccess != svc.loadconfig())
      fatal("No imagename specified and unable to read the service config.");
   mImageName = svc.getImageName();
}

service_install::service_install(std::string servicename, std::string imagename) : servicePaths(servicename), mImageName(imagename)
{
}

void service_install::_createVolumes(std::vector<std::string> & volumes)
{
   for (const auto & v : volumes)
   {
      // each service may be running under a different userid.
      if (utils_docker::dockerVolExists(v))
         logmsg(kLINFO, "A docker volume already exists for " + v + ", reusing it.");
      else
         utils_docker::createDockerVolume(v);

      // set permissions on volume.
      tVecStr args = { "run", "--rm", "-v",
         v + ":" + "/tempmount", drunnerPaths::getdrunnerUtilsImage(),
         "chmod","0777","/tempmount" };

      int rval = utils::runcommand_stream("docker", args, false);
      if (rval != 0)
         fatal("Failed to set permissions on docker volume " + v);

      logmsg(kLDEBUG, "Set permissions to allow access to volume " + v);
   }

}

void service_install::_ensureDirectoriesExist() const
{
   // create service's drunner and temp directories on host.
   utils::makedirectory(getPath(), S_755);
   utils::makedirectory(getPathdRunner(), S_777);
   utils::makedirectory(getPathServiceConfig().parent(), S_755);
}

cResult service_install::_recreate(bool updating)
{
   poco_assert(mImageName.length() > 0);

   if (updating)
   { // pull all containers used by the dService.
      serviceyml::simplefile syf(getPathServiceYml());
      if (kRSuccess!= syf.loadyml())
         for (auto c : syf.getContainers())
            utils_docker::pullImage(c);
   }
   
   try
   {
      // nuke any existing dService files on host (but preserve volume containers!).
      if (utils::fileexists(getPath()))
      {
         Poco::File spath(getPath());
         spath.remove(true); // recursively delete.
      }

      // notice for hostVolumes.
      if (utils::fileexists(getPathHostVolume()))
         logmsg(kLINFO, "A drunner hostVolume already exists for " + getName() + ", reusing it.");

      // create the basic directories.
      _ensureDirectoriesExist();

      // copy files to service directory on host.
      std::vector<std::string> args = { "run","--rm","-v",
         getPathdRunner().toString() + ":/tempcopy", mImageName, "/bin/bash", "-c" ,
         "cp -r /drunner/* /tempcopy/ && chmod a+rx /tempcopy/*" };
      std::string op;
      if (0 != utils::runcommand("docker", args, op, false))
         logmsg(kLERROR, "Couldn't copy the service files. You will need to reinstall the service.\nError:\n" + op);

      // write out service configuration for the dService.
      serviceConfig svccfg(getPathServiceConfig());
      { // use simple file temporarily.
         serviceyml::simplefile syf(getPathServiceYml());
         if (kRSuccess != syf.loadyml())
            fatal("Corrupt dService - missing service.yml");
         svccfg.create(syf);
      }
      svccfg.setImageName(mImageName);
      svccfg.setServiceName(mName);
      if (kRSuccess != svccfg.saveconfig())
         fatal("Could not save the service configuration!");

      // now can load full service.yml, using variable substitution via the defaults etc..
      serviceyml::file syfull(getPathServiceYml());
      if (kRSuccess != syfull.loadyml(svccfg.getVariables()))
         fatal("Corrupt dservice - couldn't read full service.yml");

      // make sure we have the latest of all containers.
      bool foundmain = false;
      for (const auto & entry : syfull.getContainers())
      {
         utils_docker::pullImage(entry);
         if (utils::stringisame(entry, mImageName))
            foundmain = true;
      }
      if (!foundmain)
         logmsg(kLWARN, "The main dService container " + mImageName + " was not present in the containers list in the service.yml file.");
      // create volumes, with variables substituted.
      std::vector<std::string> vols;
      syfull.getManageDockerVolumeNames(vols);
      _createVolumes(vols);
   }

   catch (const eExit & e) {
      // We failed. tidy up.
      if (utils::fileexists(getPath()))
         utils::deltree(getPath());

      throw (e);
   }

   return kRSuccess;
}

cResult service_install::install()
{
   logmsg(kLDEBUG, "Installing " + mName + " at " + getPath().toString() + ", using image " + mImageName);
	if (utils::fileexists(getPath()))
		logmsg(kLERROR, "Service already exists. Try:\n drunner update " + getName());

	// make sure we have the latest version of the service.
   utils_docker::pullImage(mImageName);

	logmsg(kLDEBUG, "Attempting to validate " + mImageName);
   validateImage::validate(mImageName);

   _recreate(false);

   service svc(mName);
   servicehook hook(&svc, "install");
   hook.endhook();

   logmsg(kLINFO, "Installation complete - try running " + mName+ " now!");
   return kRSuccess;
}

cResult service_install::uninstall()
{
   if (!utils::fileexists(getPath()))
      logmsg(kLERROR, "Can't uninstall " + mName + " - it does not exist.");

   try
   {
      service svc(mName);
      servicehook hook(&svc, "uninstall");
      hook.starthook();
   }
   catch (const eExit &)
   {
      logmsg(kLWARN, "Installation damaged, unable to use uninstall hook.");
   }

   // delete the service tree.
   logmsg(kLINFO, "Deleting all of the dService files");
   utils::deltree(getPath());

   if (utils::fileexists(getPath()))
      logmsg(kLERROR, "Uninstall failed - couldn't delete " + getPath().toString());

   logmsg(kLINFO, "Uninstalled " + getName());
   return kRSuccess;
}

cResult service_install::obliterate()
{
   cResult rval = kRNoChange;

   if (utils::fileexists(getPath()))
   {
      try
      {
         service svc(mName);
         servicehook hook(&svc, "obliterate");
         hook.starthook();

         logmsg(kLDEBUG, "Obliterating all the docker volumes - data will be gone forever.");
         {// [start] deleting docker volumes.
            std::vector<std::string> vols;
            svc.getServiceYml().getManageDockerVolumeNames(vols);
            for (const auto & entry : vols)
            {
               logmsg(kLINFO, "Obliterating docker volume " + entry);
               std::string op;
               std::vector<std::string> args = { "rm",entry };
               if (0 != utils::runcommand("docker", args, op, false))
               {
                  logmsg(kLWARN, "Failed to remove " + entry + ":");
                  logmsg(kLWARN, op);
               }
               else
                  rval = kRSuccess;
            }
         }
      }
      catch (const eExit &)
      {
         logmsg(kLWARN, "Installation damaged, unable to delete docker volumes.");
      }
   }

   if (utils::fileexists(getPath()))
   { // delete the service tree.
      logmsg(kLINFO, "Obliterating all of the dService files");
      utils::deltree(getPath());
      rval = kRSuccess;
   }

   // delete the host volumes
   if (utils::fileexists(getPathHostVolume()))
   {
      logmsg(kLINFO, "Obliterating the hostVolume (includes configuration)");
      utils::deltree(getPathHostVolume());
      rval = kRSuccess;
   }

   if (rval == kRNoChange)
      logmsg(kLWARN, "Couldn't find any trace of dService " + getName() + " - no changes made.");
   else
      logmsg(kLINFO, "Obliterated " + getName());
   return rval;
}



cResult service_install::recover()
{
   if (utils::fileexists(getPath()))
      uninstall();

   install();

   logmsg(kLINFO, getName() + " recovered.");
   return kRSuccess;
}

cResult service_install::update()
{ // update the service (recreate it)
   service svc(mName);
   servicehook hook(&svc, "update");
   hook.starthook();

   _recreate(true);

   hook.endhook();

   return kRSuccess;
}