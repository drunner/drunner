#include <cstdlib>

#include <Poco/File.h>

#include "service.h"
#include "utils.h"
#include "service_backupvars.h"
#include "compress.h"
#include "servicehook.h"
#include "globallogger.h"
#include "globalcontext.h"
#include "timez.h"
#include "drunner_paths.h"
#include "service_install.h"
#include "utils_docker.h"

// Back up this service to backupfile.
void service::backup(const std::string & backupfile)
{
   timez ttotal, tstep;

   std::string op;
   
   Poco::Path bf(backupfile);
   bf.makeAbsolute();
   if (utils::fileexists(bf))
      logmsg(kLERROR, "Backup file " + bf.toString() + " already exists. Aborting.");

   if (!isValid())
      logmsg(kLERROR, "Validation of " + getName() + " failed. Try drunner recover " + getName());

   utils::tempfolder archivefolder(drunnerPaths::getPath_Temp().pushDirectory("archivefolder-" + getName()));
   utils::tempfolder tempparent(drunnerPaths::getPath_Temp().pushDirectory("backup-"+getName()));

   // write out variables that we need to decompress everything.
   backupvars bvars(tempparent.getpath().setFileName(backupvars::filename));
   bvars.createFromServiceYml(mImageName,mServiceYml);
   bvars.savevars();

   // path for docker volumes and for container custom backups (e.g. mysqldump)
   const Poco::Path tempf = tempparent.getpath().pushDirectory("drbackup");
   const Poco::Path tempc = tempparent.getpath().pushDirectory("containerbackup");
   utils::makedirectory(tempf, S_777); // random UID in container needs access.
   utils::makedirectory(tempc, S_777);

   logmsg(kLINFO, "Time for preliminaries:           " + tstep.getelpased());
   tstep.restart();

   // notify service we're starting our backup.
   tVecStr args;
   args.push_back(tempc.toString());
   servicehook hook(this, "backup", args);
   hook.starthook();

   logmsg(kLINFO, "Time for dService to self-backup: " + tstep.getelpased());
   tstep.restart();

   // back up volume containers
   logmsg(kLDEBUG, "Backing up all docker volumes.");
   std::string password = utils::getenv("PASS");
   std::vector<std::string> dockervols;
   mServiceYml.getDockerVolumeNames(dockervols);

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
         logmsg(kLINFO, "Couldn't find docker volume " + entry + " ... skipping.");
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

   bigafile.renameTo(bf.toString());
//      logmsg(kLERROR, "Couldn't move archive from "+source+" to " + bf);

   logmsg(kLINFO, "Time to move archive:             " + tstep.getelpased());
   tstep.restart();

   logmsg(kLINFO, "Archive of service " + getName() + " created at " + bf.toString());
   logmsg(kLINFO, "Total time taken:                 " + ttotal.getelpased());
}


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
//      logmsg(kLERROR, "Couldn't copy archive to temp folder.");

   std::string password = utils::getenv("PASS");
   compress::decompress_folder(password, tempparent.getpath(), bigarchive);

   // read in old variables, just need imagename and olddockervols from them.
   backupvars bvars(tempparent.getpath().setFileName(backupvars::filename));
   if (kRSuccess!=bvars.loadvars())
      logmsg(kLERROR, "Backup corrupt - "+backupvars::filename+" couldn't be read.");

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
   service_install(mName, bvars.getImageName());


   // load in the new variables.
   service newservice(mName);

   // check that nothing about the volumes has changed in the dService.
   tVecStr dockervols;
   newservice.getServiceYml().getDockerVolumeNames(dockervols);
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
   compress::decompress_folder(password, newservice.getPathHostVolume(), hostvolp);

   // tell the dService to do its restore_end action.
   tVecStr args;
   args.push_back(tempc.toString());
   servicehook hook(&newservice, "restore", args);
   hook.endhook();

   logmsg(kLINFO, "The backup " + bf.toString() + " has been restored to service " + mName + ". Try it!");
   return kRSuccess;
}

