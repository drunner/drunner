#include <cstdlib>

#include <boost/filesystem.hpp>

#include "service.h"
#include "utils.h"
#include "sh_backupvars.h"
#include "compress.h"
#include "servicehook.h"
#include "globallogger.h"
#include "globalcontext.h"
#include "timez.h"

// Back up this service to backupfile.
void service::backup(const std::string & backupfile)
{
   timez ttotal, tstep;

   std::string op, bf = utils::getabsolutepath(backupfile);
   if (utils::fileexists(bf))
      logmsg(kLERROR, "Backup file " + bf + " already exists. Aborting.");

   if (!isValid())
      logmsg(kLERROR, "Validation of " + getName() + " failed. Try drunner recover " + getName());

   utils::tempfolder archivefolder(GlobalContext::getSettings()->getPath_Temp() + "/archivefolder-" + getName());
   utils::tempfolder tempparent(GlobalContext::getSettings()->getPath_Temp() + "/backup-"+getName());

   // write out variables that we need to decompress everything.
   sh_backupvars shb;
   shb.createFromdrunnerCompose(drunnerCompose(*this));
   shb.writeSettings(shb.getPathFromParent(tempparent.getpath()));

   // path for docker volumes and for container custom backups (e.g. mysqldump)
   std::string tempf = tempparent.getpath() + "/drbackup";
   std::string tempc = tempparent.getpath() + "/containerbackup";
   utils::makedirectory(tempf, S_777); // random UID in container needs access.
   utils::makedirectory(tempc, S_777);

   logmsg(kLINFO, "Time for preliminaries:           " + tstep.getelpased());
   tstep.restart();

   // notify service we're starting our backup.
   tVecStr args;
   args.push_back(tempc);
   servicehook hook(this, "backup", args);
   hook.starthook();

   logmsg(kLINFO, "Time for dService to self-backup: " + tstep.getelpased());
   tstep.restart();

   // back up volume containers
   logmsg(kLDEBUG, "Backing up all docker volumes.");
   std::string password = utils::getenv("PASS");
   std::vector<std::string> dockervols;
   shb.getDockerVolumeNames(dockervols);
   for (auto const & entry : dockervols)
   {
      if (utils::dockerVolExists(entry))
      {
         compress::compress_volume(password, entry, tempf, entry + ".tar",true);
         logmsg(kLDEBUG, "Backed up docker volume " + entry);
      }
      else
         logmsg(kLINFO, "Couldn't find docker volume " + entry + " ... skipping.");
   }

   logmsg(kLINFO, "Time for containter backups:      " + tstep.getelpased());
   tstep.restart();

   // back up host vol (local storage)
   logmsg(kLDEBUG, "Backing up host volume.");
   compress::compress_folder(password, getPathHostVolume(), tempf, "drunner_hostvol.tar",true);

   logmsg(kLINFO, "Time for host volume backup:      " + tstep.getelpased());
   tstep.restart();

   // notify service we've finished our backup.
   hook.endhook();

   logmsg(kLINFO, "Time for dService to wrap up:     " + tstep.getelpased());
   tstep.restart();

   // compress everything together
   boost::filesystem::path fullpath(bf);
   bool ok=compress::compress_folder(password, tempparent.getpath(), archivefolder.getpath(), "backup.tar.enc",false);
   if (!ok)
      logmsg(kLERROR, "Couldn't archive service " + getName());

   logmsg(kLINFO, "Time to collate everything:       " + tstep.getelpased());
   tstep.restart();

   // move compressed file to target dir.
   std::string source = utils::getcanonicalpath(archivefolder.getpath() + "/backup.tar.enc");
   std::string dest = fullpath.string();
   if (!utils::fileexists(source))
      logmsg(kLERROR, "Expected archive not found at " + source);
   if (0 != rename(source.c_str(), dest.c_str()))
      //exit(0);
      logmsg(kLERROR, "Couldn't move archive from "+source+" to " + dest);

   logmsg(kLINFO, "Time to move archive:             " + tstep.getelpased());
   tstep.restart();

   logmsg(kLINFO, "Archive of service " + getName() + " created at " + dest);
   logmsg(kLINFO, "Total time taken:                 " + ttotal.getelpased());
}




cResult service_restore(const std::string & servicename, const std::string & backupfile)
{ // restore from backup.
   std::string bf = utils::getcanonicalpath(backupfile);
   if (bf.length()==0 || !utils::fileexists(bf))
      logmsg(kLERROR, "Backup file " + backupfile + " does not exist.");
   logmsg(kLDEBUG, "Restoring from " + bf);

   utils::tempfolder tempparent(GlobalContext::getSettings()->getPath_Temp() + "/restore-"+servicename);
   utils::tempfolder archivefolder(GlobalContext::getSettings()->getPath_Temp() + "/archivefolder-" + servicename);

   // for docker volumes
   std::string tempf = tempparent.getpath() + "/drbackup";
   // for container custom backups (e.g. mysqldump)
   std::string tempc = tempparent.getpath() + "/containerbackup";

   // decompress main backup
   if (!utils::copyfile(bf, archivefolder.getpath() + "/backup.tar.enc"))
      logmsg(kLERROR, "Couldn't copy archive to temp folder.");

   std::string password = utils::getenv("PASS");
   compress::decompress_folder(password, tempparent.getpath(), archivefolder.getpath(), "backup.tar.enc",false);

   // read in old variables, just need imagename and olddockervols from them.
   sh_backupvars shb;
   if (!shb.readSettings(shb.getPathFromParent(tempparent.getpath())))
      logmsg(kLERROR, "Backup corrupt - backupvars.sh missing.");

   if (!utils::fileexists(tempc))
      logmsg(kLERROR, "Backup corrupt - missing " + tempc);
   std::vector<std::string> shb_dockervolumenames;
   shb.getDockerVolumeNames(shb_dockervolumenames);
   for (auto entry : shb_dockervolumenames)
      if (!utils::fileexists(tempf + "/" + entry + ".tar"))
         logmsg(kLERROR, "Backup corrupt - missing backup of volume " + entry);

   // backup seems okay - lets go!
   service svc(servicename, shb.getImageName());
   if (utils::fileexists(svc.getPath()))
      logmsg(kLERROR, "Service " + servicename + " already exists. Uninstall it before restoring from backup.");

   svc.install();

   // load in the new variables.
   drunnerCompose drc(svc);
   if (drc.readOkay()==kRError)
      logmsg(kLERROR, "Installation failed - drunner-compose.yml broken.");

   // check that nothing about the volumes has changed in the dService.
   tVecStr dockervols;
   drc.getDockerVolumeNames(dockervols);
   if (shb_dockervolumenames.size() != dockervols.size())
   {
      logmsg(kLWARN, "Number of docker volumes stored does not match what we expect. Restored backup is in unknown state.");
      svc.uninstall();
      logmsg(kLERROR, "Restore failed. Uninstalled the broken dService.");
   }

   // restore all the volumes.
   for (unsigned int i = 0; i < dockervols.size(); ++i)
   {
      if (!utils::dockerVolExists(dockervols[i]))
         logmsg(kLERROR, "Installation should have created " + dockervols[i] + " but didn't!");
      compress::decompress_volume(password, dockervols[i],
         tempf, shb_dockervolumenames[i] + ".tar",true);
   }

   // restore host vol (local storage)
   logmsg(kLDEBUG, "Restoring host volume.");
   compress::decompress_folder(password, svc.getPathHostVolume(), tempf, "drunner_hostvol.tar",true);

   // tell the dService to do its restore_end action.
   tVecStr args;
   args.push_back(tempc);
   servicehook hook(&svc, "restore", args);
   hook.endhook();

   logmsg(kLINFO, "The backup " + bf + " has been restored to service " + servicename + ". Try it!");
   return kRSuccess;
}

