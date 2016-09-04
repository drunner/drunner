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
#include "servicehook.h"
#include "validateimage.h"
#include "service_manage.h"
#include "utils_docker.h"
#include "service.h"
#include "drunner_paths.h"
#include "generate.h"
#include "dassert.h"
#include "service_vars.h"


namespace service_manage
{

   std::string _loadImageName(std::string servicename)
   { // no imagename specified. Load from service variables if we can.
      std::string iname;
      servicelua::luafile lf(servicename);
      if (kRSuccess == lf.loadlua())
      {
         serviceVars v(servicename, lf.getConfigItems());
         v.loadvariables();
         iname = v.getImageName();
      }

      if (iname.length() == 0)
         logdbg("No imagename specified and unable to read it from the service configuration.");
      return iname;
   }

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

   cResult _recreate(std::string servicename, std::string imagename)
   {
      drunner_assert(imagename.length() > 0, "Can't recreate service " + servicename + " - image name could not be determined.");
      servicePaths sp(servicename);

      utils_docker::pullImage(imagename);

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

         // create the basic directories.
         _ensureDirectoriesExist(servicename);

         // copy files to service directory on host.
         logdbg("Copying across drunner files from " + imagename);
         CommandLine cl("docker", { "run","--rm","-u","root","-v",
            sp.getPathdService().toString() + ":/tempcopy", imagename, "/bin/bash", "-c" });
#ifdef _WIN32
         cl.args.push_back("cp /drunner/* /tempcopy/ && chmod a+rw /tempcopy/*");
#else
         uid_t uid = getuid();
         cl.args.push_back("cp /drunner/* /tempcopy/ && chmod u+rw /tempcopy/* && chown -R " + std::to_string(uid) + " /tempcopy/*");
#endif
         std::string op;
         if (0 != utils::runcommand(cl, op))
            logmsg(kLERROR, "Could not copy the service files. You will need to reinstall the service.\nError:\n" + op);
         drunner_assert(utils::fileexists(sp.getPathServiceLua()), "The dService service.lua file was not copied across.");


         // load the lua file
         servicelua::luafile syf(servicename);
         if (syf.loadlua() != kRSuccess)
            fatal("Corrupt dservice - couldn't read service.lua.");

         // write out service configuration for the dService.
         serviceVars sv(servicename, imagename, syf.getConfigItems());
         if (kRSuccess == sv.loadvariables()) // in case there's an existing file.
            logdbg("Loaded existing service variables.");

         // force the new imagename. (Imagename could be different on recreate - e.g. overridden at command line)
         sv.setVal("IMAGENAME", imagename);
         drunner_assert(sv.getServiceName() == servicename, "Service name mismatch: " + sv.getServiceName() + " vs " + servicename);
         sv.savevariables();

         // pull all containers.
         for (const auto & entry : syf.getContainers())
            utils_docker::pullImage(entry);

         // create volumes, with variables substituted.
         std::vector<std::string> vols;
         syf.getManageDockerVolumeNames(vols);
         _createVolumes(vols);

         // create launch script
         _createLaunchScript(servicename);
      }

      catch (const eExit & e) {
         // We failed. tidy up.
         if (utils::fileexists(sp.getPathdService()))
            utils::deltree(sp.getPathdService());

         throw (e);
      }

      return kRSuccess;
   }

   cResult install(std::string & servicename, std::string imagename)
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

      // make sure we have the latest version of the service.
      utils_docker::pullImage(imagename);

      logmsg(kLDEBUG, "Attempting to validate " + imagename);
      validateImage::validate(imagename);

      _recreate(servicename,imagename);

      servicehook hook(servicename, "install");
      hook.endhook();

      logdbg("Installation of " + servicename + " complete.");
      return kRSuccess;
   }

   cResult uninstall(std::string servicename)
   {
      servicePaths sp(servicename);
      cResult rval = kRNoChange;

      if (!utils::fileexists(sp.getPathdService()))
         return cError("Can't uninstall " + servicename + " - it does not exist.");

      try
      {
         servicehook hook(servicename, "uninstall");
         hook.starthook();
      }
      catch (const eExit &)
      {
         logmsg(kLWARN, "Installation damaged, unable to use uninstall hook.");
      }

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

   cResult obliterate(std::string servicename)
   {
      cResult rval = kRNoChange;
      servicePaths sp(servicename);

      if (utils::fileexists(sp.getPathServiceLua()))
      {
         try
         {
            servicehook hook(servicename, "obliterate");
            hook.starthook();

            logmsg(kLDEBUG, "Obliterating all the docker volumes - data will be gone forever.");
            // [start] deleting docker volumes.
            std::vector<std::string> vols;

            servicelua::luafile lf(servicename);
            if (lf.loadlua() == kRSuccess)
            {
               lf.getManageDockerVolumeNames(vols);
               for (const auto & entry : vols)
               {
                  logmsg(kLINFO, "Obliterating docker volume " + entry);
                  std::string op;
                  CommandLine cl("docker", { "volume", "rm",entry });
                  if (0 != utils::runcommand(cl, op))
                  {
                     logmsg(kLWARN, "Failed to remove " + entry + ":");
                     logmsg(kLWARN, op);
                     rval += cError("Failed to remove " + entry + ":" + op);
                  }
                  else
                     rval += kRSuccess;
               }
            }
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

   cResult update(std::string servicename)
   { // update the service (recreate it)
      std::string imagename = _loadImageName(servicename);
      drunner_assert(imagename.length() > 0, "Can't update service " + servicename + " - image name could not be determined.");

      servicehook hook(servicename, "update");
      cResult rslt = hook.starthook();
      rslt += _recreate(servicename, imagename);
      rslt += hook.endhook();

      return rslt;
   }



   cResult _createLaunchScript(std::string servicename)
   {
#ifdef _WIN32
      Poco::Path target = drunnerPaths::getPath_Bin().setFileName(servicename + ".bat");
      std::string vdata = R"EOF(@echo off
drunner servicecmd __SERVICENAME__ %*
)EOF";
#else
      Poco::Path target = drunnerPaths::getPath_Bin().setFileName(servicename);
      std::string vdata = R"EOF(#!/bin/bash
drunner servicecmd "__SERVICENAME__" "$@"
)EOF";
#endif

      vdata = utils::replacestring(vdata, "__SERVICENAME__", servicename);
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