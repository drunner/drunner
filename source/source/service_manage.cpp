#include <sys/stat.h>
#include <fstream>

#include <Poco/String.h>
#include <Poco/File.h>

#include "utils.h"
#include "utils_docker.h"
#include "globallogger.h"
#include "globalcontext.h"
#include "exceptions.h"
#include "drunner_setup.h"
#include "service.h"
#include "validateimage.h"
#include "service_manage.h"
#include "utils_docker.h"
#include "service.h"
#include "drunner_paths.h"
#include "generate.h"
#include "dassert.h"
#include "service_vars.h"
#include "service_dservicedefinstall.h"


namespace service_manage
{
   void _createVolumes(std::vector<std::string> & volumes)
   {
      if (volumes.size() == 0)
         logmsg(kLDEBUG, "[No volumes declared to be managed by drunner]");
      else
         for (const auto & v : volumes)
         {
            logmsg(kLDEBUG, "Checking status of volume " + v);

            // each service may be running under a different userid.
            if (utils_docker::dockerVolExists(v))
               logmsg(kLINFO, "A docker volume already exists for " + v + ", reusing it.");
            else
               utils_docker::createDockerVolume(v);

            // set permissions on volume.
            CommandLine cl("docker", { "run", "--rm", "-v",
               v + ":" + "/tempmount", drunnerPaths::getdrunnerUtilsImage(),
               "chmod","0777","/tempmount" });

            std::string faily;
            int rval = utils::runcommand_stream(cl, GlobalContext::getParams()->supportCallMode(), "", {},&faily);
            if (rval != 0)
               fatal("Failed to set permissions on docker volume " + v + ":\n "+faily);

            logmsg(kLDEBUG, "Set permissions to allow access to volume " + v);
         }
      logmsg(kLDEBUG, "Finished checking volumes.");
   }

   void _ensureDirectoriesExist(std::string servicename)
   {
      servicePaths sp(servicename);
      // create service's drunner and temp directories on host.
      cResult r1 = utils::makedirectory(sp.getPathdService(), S_700);
      if (!r1.success()) fatal("Couldn't create directory.\nError: " + r1.what());

      cResult r2 = utils::makedirectory(sp.getPathServiceVars().parent(), S_700);
      if (!r2.success()) fatal("Couldn't create directory.\nError: " + r2.what());
   }

   // common elements of creating a service, used by _create and by service_restore.
   // everything except the dService definition and the service variables.
   cResult _create_common(std::string servicename)
   {
      // create the basic directories.
      _ensureDirectoriesExist(servicename);

      // create launch script
      return _createLaunchScript(servicename);
   }

   cResult _install_create(std::string servicename, std::string imagename, bool devMode)
   {
      drunner_assert(imagename.length() > 0, "Can't create service " + servicename + " - imagename could not be determined.");
      servicePaths sp(servicename);

      try
      {
         // nuke any existing dService files on host (but preserve volume containers!).
         if (utils::fileexists(sp.getPathdService()))
         {
            Poco::File spath(sp.getPathdService());
            spath.remove(true); // recursively delete.
         }

         // notice for hostVolumes.
         if (utils::fileexists(sp.getPathHostVolume()))
            logmsg(kLINFO, "A drunner hostVolume already exists for " + servicename + ", reusing it.");

         _create_common(servicename);

         // copy files to service directory on host.
         cResult r = sddi::copy_from_container(imagename, sp);
         if (!r.success()) fatal(r.what());

         // write out service configuration for the dService.
         serviceVars sv(servicename);

         // force the new imagename. (Imagename could be different on recreate - e.g. overridden at command line)
         sv.setImageName(imagename);
         sv.setDevMode(devMode);
         drunner_assert(sv.getServiceName() == servicename, "Service name mismatch: " + sv.getServiceName() + " vs " + servicename);
         sv.savevariables();
      }

      catch (const eExit & e) {
         // We failed. tidy up.
         if (utils::fileexists(sp.getPathdService()))
            utils::deltree(sp.getPathdService());

         throw (e);
      }

      return kRSuccess;
   }

   cResult install(std::string & servicename, std::string imagename, bool devMode)
   {
      if (servicename.length() == 0)
      {
         servicename = imagename;
         size_t found;
         while ((found = servicename.find("/")) != std::string::npos)
            servicename.erase(0, found + 1);
         while ((found = servicename.find(":")) != std::string::npos)
            servicename.erase(found);
      }

      servicePaths sp(servicename);
      drunner_assert(imagename.length() > 0, "Can't install service " + servicename + " - image name could not be determined.");

      logmsg(kLDEBUG, "Installing " + servicename + " at " + sp.getPathdService().toString() + ", using image " + imagename);
      if (utils::fileexists(sp.getPathdService()))
         logmsg(kLERROR, "Service already exists. Try:\n drunner update " + servicename);

      // make sure we have the latest version of the service for validation.
      if (!devMode)
         utils_docker::pullImage(imagename);

      logmsg(kLDEBUG, "Attempting to validate " + imagename);
      validateImage::validate(imagename);

      _install_create(servicename,imagename,devMode);

      serviceVars sv(servicename);
      servicelua::luafile lf(sv, CommandLine("install"));
      // run the recreate command
      if (lf.getResult() != kRSuccess)
         fatal("Failed to run install in service.lua:\n"+lf.getResult().what());

      logdbg("Installation of " + servicename + " complete.");
      return kRSuccess;
   }

   cResult uninstall(std::string servicename)
   {
      servicePaths sp(servicename);
      cResult rval = kRNoChange;

      if (!utils::fileexists(sp.getPathdService()))
         return cError("Can't uninstall " + servicename + " - it does not exist.");

      serviceVars sv(servicename);
      servicelua::luafile lf(sv, CommandLine("uninstall"));
      if (!lf.getResult().success())
         logmsg(kLWARN, "Uninstall service command failed.");

      // delete the service tree.
      logmsg(kLINFO, "Deleting all of the dService files");
      rval += utils::deltree(sp.getPathdService());

      if (utils::fileexists(sp.getPathdService()))
         return cError("Uninstall failed - couldn't delete " + sp.getPathdService().toString());

      // delete the launch script
      rval += _removeLaunchScript(servicename);

      if (!rval.error())
         logmsg(kLINFO, "Uninstalled " + servicename);
      return rval;
   }

   // -------------------------------------------------------------------------------------------------

   cResult obliterate(std::string servicename)
   {
      cResult rval = kRNoChange;
      servicePaths sp(servicename);

      if (utils::fileexists(sp.getPathServiceLua()))
      {
         try
         {
            serviceVars sv(servicename);
            
            servicelua::luafile lf(sv, CommandLine("obliterate"));
            if (!lf.getResult().success())
               logmsg(kLWARN, "Obliterate service command failed.");
         }
         catch (const eExit &)
         {
            logmsg(kLWARN, "Installation damaged, unable to delete docker volumes.");
         }
      }

      if (utils::fileexists(sp.getPathdService()))
      { // delete the service tree.
         logmsg(kLINFO, "Obliterating all of the dService files.");
         cResult result = utils::deltree(sp.getPathdService());
         rval += result;
         if (result != kRSuccess)
            logmsg(kLINFO, "Failed to delete the dService files.");
      }

      // delete the host volumes
      if (utils::fileexists(sp.getPathHostVolume()))
      {
         logdbg("Obliterating the hostVolume (includes configuration).");
         cResult result = utils::deltree(sp.getPathHostVolume());
         rval += result;
         if (result != kRSuccess)
            logmsg(kLINFO, "Failed to delete the hostVolume files.");
      }

      // delete the launch script
      rval += _removeLaunchScript(servicename);

      if (rval == kRNoChange)
         logmsg(kLWARN, "Couldn't find any trace of dService " + servicename + " - no changes made.");
      else if (rval.error())
         logmsg(kLWARN, "Obliterated what we could, but system is not clean:\n " + rval.what());
      else
         logmsg(kLINFO, "Obliterated " + servicename);
      return rval;
   }

   // -------------------------------------------------------------------------------------------------

   cResult update(std::string servicename)
   { // update the service (recreate it)
      serviceVars v(servicename);
      std::string imagename = v.getImageName();
      bool devmode = v.getIsDevMode();
      drunner_assert(imagename.length() > 0, "Imagename is empty!");

      try
      {
         uninstall(servicename);
      }
      catch (const eExit &)
      {
         logmsg(kLWARN, "Installation damaged, unable to uninstall dService. Attempting install...");
      }

      return install(servicename, imagename, devmode);
   }



   cResult _createLaunchScript(std::string servicename)
   {
#ifdef _WIN32
      Poco::Path target = drunnerPaths::getPath_Bin().setFileName(servicename + ".bat");
      std::string vdata = R"EOF(@echo off
__DRUNNER__ servicecmd __SERVICENAME__ %*
)EOF";
#else
      Poco::Path target = drunnerPaths::getPath_Bin().setFileName(servicename);
      std::string vdata = R"EOF(#!/bin/sh
__DRUNNER__ servicecmd "__SERVICENAME__" "$@"
)EOF";
#endif

      vdata = utils::replacestring(vdata, "__SERVICENAME__", servicename);
      vdata = utils::replacestring(vdata, "__DRUNNER__", drunnerPaths::getPath_Exe_Target().toString());

      generate(target, S_755, vdata);
      return kRSuccess;
   }

   cResult _removeLaunchScript(std::string servicename)
   {
      cResult rval = kRNoChange;

#ifdef _WIN32
      Poco::Path target = drunnerPaths::getPath_Bin().setFileName(servicename + ".bat");
#else
      Poco::Path target = drunnerPaths::getPath_Bin().setFileName(servicename);
#endif

      if (!utils::fileexists(target))
      {
         logmsg(kLDEBUG, "Launch script " + target.toString() + " does not exist.");
         return rval;
      }

      rval += utils::delfile(target);

      if (rval == kRSuccess)
         logmsg(kLDEBUG, "Deleted " + target.toString());
      else
         logmsg(kLWARN, "Couldn't remove launch script");
      return rval;
   }

} // service_manage