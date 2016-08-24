#include <cstdlib>

#include <Poco/File.h>
#include <Poco/String.h>

#include "service.h"
#include "utils.h"
#include "service_backupinfo.h"
#include "compress.h"
#include "servicehook.h"
#include "globallogger.h"
#include "globalcontext.h"
#include "timez.h"
#include "drunner_paths.h"
#include "service_manage.h"
#include "utils_docker.h"
#include "dassert.h"

// Back up this service to backupfile.
cResult service::backup(const std::string & backupfile)
{
   timez ttotal, tstep;

   std::string op;
   
   Poco::Path bf(backupfile);
   bf.makeAbsolute();
   if (utils::fileexists(bf))
      return cError("Backup file " + bf.toString() + " already exists. Aborting.");

   utils::tempfolder archivefolder(drunnerPaths::getPath_Temp().pushDirectory("archivefolder-" + getName()));
   utils::tempfolder tempparent(drunnerPaths::getPath_Temp().pushDirectory("backup-"+getName()));

   // write out variables that we need to decompress everything.
   backupinfo bvars(tempparent.getpath().setFileName(backupinfo::filename));
   bvars.create(getImageName());
   bvars.savevars();

   // path for docker volumes and for container custom backups (e.g. mysqldump)
   const Poco::Path tempf = tempparent.getpath().pushDirectory("drbackup");
   const Poco::Path tempc = tempparent.getpath().pushDirectory("containerbackup");
   if (!utils::makedirectory(tempf, S_777).success()) fatal("Failed to create temp dir."); // random UID in container needs access.
   if (!utils::makedirectory(tempc, S_777).success()) fatal("Failed to create temp dir.");

   logmsg(kLINFO, "Time for preliminaries:           " + tstep.getelpased());
   tstep.restart();

   // notify service we're starting our backup.
   tVecStr args;
   args.push_back(tempc.toString());
   servicehook hook(getName(), "backup", args);
   hook.starthook();

   logmsg(kLINFO, "Time for dService to self-backup: " + tstep.getelpased());
   tstep.restart();

   // back up volume containers
   logmsg(kLDEBUG, "Backing up all docker volumes.");
   std::string password = utils::getenv("PASS");
   std::vector<servicelua::BackupVol> dockervols;
   mServiceLua.getBackupDockerVolumeNames(dockervols);

   for (auto const & entry : dockervols)
   {
      if (utils_docker::dockerVolExists(entry.volumeName))
      {
         Poco::Path volarchive(tempf);
         volarchive.setFileName(entry.backupName + ".tar");
         compress::compress_volume(password, entry.volumeName, volarchive);
         logmsg(kLDEBUG, "Backed up docker volume " + entry.volumeName + " as "+entry.backupName);
      }
      else
         fatal("Couldn't find docker volume " + entry.volumeName + ".");
   }

   logmsg(kLINFO, "Time for containter backups:      " + tstep.getelpased());
   tstep.restart();

   // back up host vol (local storage)
   logmsg(kLDEBUG, "Backing up host volume.");
   Poco::Path hostvolp(tempf);
   hostvolp.setFileName("drunner_hostvol.tar");
   compress::compress_folder(password, getPathHostVolume(), hostvolp);
   
   logmsg(kLINFO, "Time for host volume backup:      " + tstep.getelpased());
   tstep.restart();

   // notify service we've finished our backup.
   hook.endhook();

   logmsg(kLINFO, "Time for dService to wrap up:     " + tstep.getelpased());
   tstep.restart();

   // compress everything together
   Poco::Path bigarchive(archivefolder.getpath());
   bigarchive.setFileName("backup.tar.enc");
   bool ok=compress::compress_folder(password, tempparent.getpath().toString(), bigarchive);
   if (!ok)
      logmsg(kLERROR, "Couldn't archive service " + getName());

   logmsg(kLINFO, "Time to compress and encrypt:     " + tstep.getelpased());
   tstep.restart();

   // move compressed file to target dir.
   Poco::File bigafile(bigarchive);
   if (!bigafile.exists())
      logmsg(kLERROR, "Expected archive not found at " + bigarchive.toString());

   logdbg("Moving " + bigarchive.toString() + " to " + bf.toString());
   bigafile.renameTo(bf.toString());
//      logmsg(kLERROR, "Couldn't move archive from "+source+" to " + bf);

   logmsg(kLINFO, "Time to move archive:             " + tstep.getelpased());
   tstep.restart();

   logmsg(kLINFO, "Archive of service " + getName() + " created at " + bf.toString());
   logmsg(kLINFO, "Total time taken:                 " + ttotal.getelpased());

   return kRSuccess;
}


// -------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------

void service_restore_fail(std::string servicename, std::string message)
{
   logmsg(kLWARN, message);
   service_manage::uninstall(servicename);
   logmsg(kLERROR, "Restore failed. Uninstalled the broken dService.");
}

cResult service_manage::service_restore(const std::string & backupfile, std::string servicename)
{ // restore from backup.
   Poco::Path bf(backupfile);
   bf.makeAbsolute();
   if (!utils::fileexists(bf))
      logmsg(kLERROR, "Backup file " + backupfile + " does not exist.");
   logmsg(kLDEBUG, "Restoring from " + bf.toString());

   utils::tempfolder tempparent(drunnerPaths::getPath_Temp().pushDirectory("restore-"+ servicename));
   utils::tempfolder archivefolder(drunnerPaths::getPath_Temp().pushDirectory("archivefolder-" + servicename));

   // for docker volumes
   const Poco::Path tempf = tempparent.getpath().pushDirectory("drbackup");
   // for container custom backups (e.g. mysqldump)
   const Poco::Path tempc = tempparent.getpath().pushDirectory("containerbackup");

   // decompress main backup
   Poco::File bff(bf);
   Poco::Path bigarchive(archivefolder.getpath());
   bigarchive.setFileName("backup.tar.enc");
   bff.copyTo(bigarchive.toString());

   std::string password = utils::getenv("PASS");
   compress::decompress_folder(password, tempparent.getpath(), bigarchive);

   if (!utils::fileexists(tempc))
      logmsg(kLERROR, "Backup corrupt - missing " + tempc.toString());

   // read in old variables, just need imagename and olddockervols from them.
   backupinfo bvars(tempparent.getpath().setFileName(backupinfo::filename));
   if (kRSuccess!=bvars.loadvars())
      logmsg(kLERROR, "Backup corrupt - "+backupinfo::filename+" couldn't be read.");

   // backup seems okay - lets go!
   std::string imagename = bvars.getImageName();
   drunner_assert(imagename.length() > 0, "Empty imagename in backup.");
   service_manage::install(servicename, imagename);

   // load in the new lua file.
   servicePaths paths(servicename);
   servicelua::luafile newluafile(servicename);
   if (kRSuccess != newluafile.loadlua())
      service_restore_fail(servicename,"Installation did not correctly create the service.lua file.");

   // check that nothing about the volumes has changed in the dService.
   std::vector<servicelua::BackupVol> dockervols;
   newluafile.getBackupDockerVolumeNames(dockervols);
   
   // restore all the volumes.
   for (auto vol : dockervols)
   {
      if (!utils_docker::dockerVolExists(vol.volumeName))
         service_restore_fail(servicename, "Installation should have created " + vol.volumeName + " but didn't!");

      Poco::Path volarchive(tempf);
      volarchive.setFileName(vol.backupName + ".tar");
      if (!utils::fileexists(volarchive))
         fatal("Backup set did not contain backup of volume " + volarchive.toString());
      compress::decompress_volume(password, vol.volumeName, volarchive);
   }

   // restore host vol (local storage)
   logmsg(kLDEBUG, "Restoring host volume.");
   Poco::Path hostvolp(tempf);
   hostvolp.setFileName("drunner_hostvol.tar");
   compress::decompress_folder(password, paths.getPathHostVolume(), hostvolp);

   // host volume on disk has the old settings. newluafile has the new settings. Need to merge!
   serviceVars oldvars(servicename, imagename, newluafile.getConfigItems());

   if (kRSuccess != oldvars.loadvariables()) // loads with new schema (updates).
      logmsg(kLWARN, "Backup configuration file could not be loaded. Using defaults for all configuration!");

   // clobber SERVICENAME and IMAGENAME, ensuring they are the latest.
   oldvars.setVal("SERVICENAME", servicename);
   oldvars.setVal("IMAGENAME", imagename);

   if (kRSuccess != oldvars.savevariables())
      logmsg(kLWARN, "Failed to save variables.");

   // tell the dService to do its restore_end action.
   tVecStr args;
   args.push_back(tempc.toString());
   servicehook hook(servicename, "restore", args);
   hook.endhook();

   logmsg(kLINFO, "The backup " + bf.toString() + " has been restored to service " + servicename + ". Try it!");
   return kRSuccess;
}

