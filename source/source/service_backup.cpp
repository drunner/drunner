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
#include "service_install.h"
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
   bvars.createFromServiceLua(mImageName,mServiceLua);
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
   std::vector<std::string> dockervols;
   mServiceLua.getBackupDockerVolumeNames(dockervols);

   for (auto const & entry : dockervols)
   {
      if (utils_docker::dockerVolExists(entry))
      {
         Poco::Path volarchive(tempf);
         volarchive.setFileName(entry + ".tar");
         compress::compress_volume(password, entry, volarchive);
         logmsg(kLDEBUG, "Backed up docker volume " + entry);
      }
      else
         fatal("Couldn't find docker volume " + entry + ".");
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
   service_install sinst(servicename);
   sinst.uninstall();
   logmsg(kLERROR, "Restore failed. Uninstalled the broken dService.");
}

cResult service_install::service_restore(const std::string & backupfile)
{ // restore from backup.
   Poco::Path bf(backupfile);
   bf.makeAbsolute();
   if (!utils::fileexists(bf))
      logmsg(kLERROR, "Backup file " + backupfile + " does not exist.");
   logmsg(kLDEBUG, "Restoring from " + bf.toString());

   utils::tempfolder tempparent(drunnerPaths::getPath_Temp().pushDirectory("restore-"+mName));
   utils::tempfolder archivefolder(drunnerPaths::getPath_Temp().pushDirectory("archivefolder-" + mName));

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

   // read in old variables, just need imagename and olddockervols from them.
   backupinfo bvars(tempparent.getpath().setFileName(backupinfo::filename));
   if (kRSuccess!=bvars.loadvars())
      logmsg(kLERROR, "Backup corrupt - "+backupinfo::filename+" couldn't be read.");

   if (!utils::fileexists(tempc))
      logmsg(kLERROR, "Backup corrupt - missing " + tempc.toString());
   const std::vector<std::string> & shb_dockervolumenames(bvars.getDockerVolumeNames());

   for (auto entry : shb_dockervolumenames)
   {
      Poco::Path entrypath = tempf;
      entrypath.setFileName(entry + ".tar");
      if (!utils::fileexists(entrypath))
         logmsg(kLERROR, "Backup corrupt - missing backup of volume " + entry);
   }

   // backup seems okay - lets go!
   std::string imagename = bvars.getImageName();
   drunner_assert(imagename.length() > 0, "Empty imagename in backup.");
   service_install si(mName, imagename);
   si.install();

   // load in the new lua file.
   servicePaths paths(mName);
   servicelua::luafile newluafile(mName);
   if (kRSuccess != newluafile.loadlua())
      service_restore_fail(mName,"Installation did not correctly create the service.lua file.");

   // check that nothing about the volumes has changed in the dService.
   tVecStr dockervols;
   newluafile.getBackupDockerVolumeNames(dockervols);
   if (shb_dockervolumenames.size() != dockervols.size())
      service_restore_fail(mName, "Number of docker volumes stored does not match what we expect.");
   
   // restore all the volumes.
   for (unsigned int i = 0; i < dockervols.size(); ++i)
   {
      if (!utils_docker::dockerVolExists(dockervols[i]))
         service_restore_fail(mName, "Installation should have created " + dockervols[i] + " but didn't!");

      Poco::Path volarchive(tempf);
      volarchive.setFileName(shb_dockervolumenames[i] + ".tar");
      compress::decompress_volume(password, dockervols[i], volarchive);
   }

   // restore host vol (local storage)
   logmsg(kLDEBUG, "Restoring host volume.");
   Poco::Path hostvolp(tempf);
   hostvolp.setFileName("drunner_hostvol.tar");
   compress::decompress_folder(password, paths.getPathHostVolume(), hostvolp);

   // host volume on disk has the old settings. newluafile has the new settings. Need to merge!
   serviceVars oldvars(mName, imagename, newluafile.getConfigItems());

   if (kRSuccess != oldvars.loadvariables()) // loads with new schema (updates).
      logmsg(kLWARN, "Backup configuration file could not be loaded. Using defaults for all configuration!");

   // clobber SERVICENAME and IMAGENAME, ensuring they are the latest.
   oldvars.setVal("SERVICENAME", mName);
   oldvars.setVal("IMAGENAME", imagename);

   if (kRSuccess != oldvars.savevariables())
      logmsg(kLWARN, "Failed to save variables.");

   // tell the dService to do its restore_end action.
   tVecStr args;
   args.push_back(tempc.toString());
   servicehook hook(mName, "restore", args);
   hook.endhook();

   logmsg(kLINFO, "The backup " + bf.toString() + " has been restored to service " + mName + ". Try it!");
   return kRSuccess;
}

