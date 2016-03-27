#include <cstdlib>

#include <boost/filesystem.hpp>

#include "service.h"
#include "utils.h"
#include "sh_backupvars.h"
#include "compress.h"
#include "servicehook.h"
#include "logmsg.h"


// Back up this service to backupfile.
void service::backup(const std::string & backupfile)
{
   std::string op, bf = utils::getabsolutepath(backupfile);
   if (utils::fileexists(bf))
      logmsg(kLERROR, "Backup file " + bf + " already exists. Aborting.");

   if (!isValid())
      logmsg(kLERROR, "Validation of " + getName() + " failed. Try drunner recover " + getName());

   utils::tempfolder archivefolder(mSettings.getPath_Temp() + "/archivefolder-" + getName(), mParams);
   utils::tempfolder tempparent(mSettings.getPath_Temp() + "/backup-"+getName(), mParams);

   // write out variables that we need to decompress everything.
   sh_backupvars shb(tempparent.getpath());
   shb.createFromdrunnerCompose(drunnerCompose(*this, mParams));
   shb.write();

   // path for docker volumes and for container custom backups (e.g. mysqldump)
   std::string tempf = tempparent.getpath() + "/drbackup";
   std::string tempc = tempparent.getpath() + "/containerbackup";
   utils::makedirectory(tempf, mParams, S_777); // random UID in container needs access.
   utils::makedirectory(tempc, mParams, S_777);

   // notify service we're starting our backup.
   tVecStr args;
   args.push_back(tempc);
   servicehook hook(this, "backup", args, mParams);
   hook.starthook();

   // back up volume containers
   logmsg(kLDEBUG, "Backing up all docker volumes.");
   std::string password = utils::getenv("PASS");
   const std::vector<std::string> & dockervols(shb.getDockerVols());
   for (auto const & entry : dockervols)
   {
      if (utils::dockerVolExists(entry))
      {
         compress::compress_volume(password, entry, tempf, entry + ".tar.7z",mParams);
         logmsg(kLDEBUG, "Backed up docker volume " + entry);
      }
      else
         logmsg(kLINFO, "Couldn't find docker volume " + entry + " ... skipping.");
   }

   // back up host vol (local storage)
   logmsg(kLDEBUG, "Backing up host volume.");
   compress::compress_folder(password, getPathHostVolume(), tempf, "drunner_hostvol.tar.7z", mParams);

   // notify service we've finished our backup.
   hook.endhook();

   // compress everything together
   boost::filesystem::path fullpath(bf);
   bool ok=compress::compress_folder(password, tempparent.getpath(), archivefolder.getpath(), "backup.tar.7z",mParams);
   if (!ok)
      logmsg(kLERROR, "Couldn't archive service " + getName());

   // move compressed file to target dir.
   std::string source = utils::getcanonicalpath(archivefolder.getpath() + "/backup.tar.7z");
   std::string dest = fullpath.string();
   if (!utils::fileexists(source))
      logmsg(kLERROR, "Expected archive not found at " + source);
   if (0 != rename(source.c_str(), dest.c_str()))
      //exit(0);
      logmsg(kLERROR, "Couldn't move archive from "+source+" to " + dest);

   logmsg(kLINFO, "Archive of service " + getName() + " created at " + dest);
}




cResult service_restore(const params & prms, const sh_drunnercfg & settings, const std::string & servicename, const std::string & backupfile)
{ // restore from backup.
   std::string bf = utils::getcanonicalpath(backupfile);
   if (bf.length()==0 || !utils::fileexists(bf))
      logmsg(kLERROR, "Backup file " + backupfile + " does not exist.", prms);
   logmsg(kLDEBUG, "Restoring from " + bf, prms);

   utils::tempfolder tempparent(settings.getPath_Temp() + "/restore-"+servicename, prms);
   utils::tempfolder archivefolder(settings.getPath_Temp() + "/archivefolder-" + servicename, prms);

   // for docker volumes
   std::string tempf = tempparent.getpath() + "/drbackup";
   // for container custom backups (e.g. mysqldump)
   std::string tempc = tempparent.getpath() + "/containerbackup";

   // decompress main backup
   if (!utils::copyfile(bf, archivefolder.getpath() + "/backup.tar.7z"))
      logmsg(kLERROR, "Couldn't copy archive to temp folder.", prms);

   std::string password = utils::getenv("PASS");
   compress::decompress_folder(password, tempparent.getpath(), archivefolder.getpath(), "backup.tar.7z", prms);

   // read in old variables, just need imagename and olddockervols from them.
   sh_backupvars shb(tempparent.getpath());
   if (!shb.readOkay())
      logmsg(kLERROR, "Backup corrupt - backupvars.sh missing.", prms);

   if (!utils::fileexists(tempc))
      logmsg(kLERROR, "Backup corrupt - missing " + tempc, prms);
   for (auto entry : shb.getDockerVols())
      if (!utils::fileexists(tempf + "/" + entry + ".tar.7z"))
         logmsg(kLERROR, "Backup corrupt - missing backup of volume " + entry, prms);

   // backup seems okay - lets go!
   service svc(prms, settings, servicename, shb.getImageName());
   if (utils::fileexists(svc.getPath()))
      logmsg(kLERROR, "Service " + servicename + " already exists. Uninstall it before restoring from backup.", prms);

   svc.install();

   // load in the new variables.
   drunnerCompose drc(svc, prms);
   if (!drc.readOkay())
      logmsg(kLERROR, "Installation failed - variables.sh broken.", prms);

   // check that nothing about the volumes has changed in the dService.
   tVecStr dockervols;
   drc.getDockerVols(dockervols);
   if (shb.getDockerVols().size() != dockervols.size())
   {
      logmsg(kLWARN, "Number of docker volumes stored does not match what we expect. Restored backup is in unknown state.", prms);
      svc.uninstall();
      logmsg(kLERROR, "Restore failed. Uninstalled the broken dService.", prms);
   }

   // restore all the volumes.
   for (unsigned int i = 0; i < dockervols.size(); ++i)
   {
      if (!utils::dockerVolExists(dockervols[i]))
         logmsg(kLERROR, "Installation should have created " + dockervols[i] + " but didn't!", prms);
      compress::decompress_volume(password, dockervols[i],
         tempf, shb.getDockerVols()[i] + ".tar.7z", prms);
   }

   // restore host vol (local storage)
   logmsg(kLDEBUG, "Restoring host volume.",prms);
   compress::decompress_folder(password, svc.getPathHostVolume(), tempf, "drunner_hostvol.tar.7z", prms);

   // tell the dService to do its restore_end action.
   tVecStr args;
   args.push_back(tempc);
   servicehook hook(&svc, "restore", args, prms);
   hook.endhook();

   logmsg(kLINFO, "The backup " + bf + " has been restored to service " + servicename + ". Try it!", prms);
   return kRSuccess;
}

