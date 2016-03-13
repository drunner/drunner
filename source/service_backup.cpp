#include <cstdlib>

#include <boost/filesystem.hpp>

#include "service.h"
#include "utils.h"
#include "sh_variables.h"
#include "compress.h"
#include "servicehook.h"



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
   sh_variables shv(getPathVariables()); // loads variables.sh from the service.
   shv.writecopy(tempparent.getpath() + "/oldvariables.sh"); // output.

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
   const std::vector<std::string> & dockervols(shv.getDockerVols());
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





void service::restore(const std::string & backupfile)
{ // restore from backup.
   std::string bf = utils::getcanonicalpath(backupfile);
   if (bf.length()==0 || !utils::fileexists(bf))
      logmsg(kLERROR, "Backup file " + backupfile + " does not exist.");
   logmsg(kLDEBUG, "Restoring from " + bf);

   if (utils::fileexists(getPath()))
      logmsg(kLERROR, "Service " + getName() + " already exists. Uninstall it before restoring from backup.");

   utils::tempfolder tempparent(mSettings.getPath_Temp() + "/restore-"+getName(), mParams);
   utils::tempfolder archivefolder(mSettings.getPath_Temp() + "/archivefolder-" + getName(), mParams);

   // for docker volumes
   std::string tempf = tempparent.getpath() + "/drbackup";
   // for container custom backups (e.g. mysqldump)
   std::string tempc = tempparent.getpath() + "/containerbackup";

   // decompress main backup
   if (!utils::copyfile(bf, archivefolder.getpath() + "/backup.tar.7z"))
      logmsg(kLERROR, "Couldn't copy archive to temp folder.");

   std::string password = utils::getenv("PASS");
   compress::decompress_folder(password, tempparent.getpath(), archivefolder.getpath(), "backup.tar.7z", mParams);

   // read in old variables, just need imagename and olddockervols from them.
   sh_variables oldvars(tempparent.getpath() + "/oldvariables.sh");
   if (!oldvars.readOkay())
      logmsg(kLERROR, "Backup corrupt - oldvariables.sh missing.");

   if (!utils::fileexists(tempc))
      logmsg(kLERROR, "Backup corrupt - missing " + tempc);
   for (auto entry : oldvars.getDockerVols())
      if (!utils::fileexists(tempf + "/" + entry + ".tar.7z"))
         logmsg(kLERROR, "Backup corrupt - missing backup of volume " + entry);

   // backup seems okay - lets go!
   setImageName(oldvars.getImageName());
   install();

   // load in the new variables.
   sh_variables newvars(getPathVariables());
   if (!newvars.readOkay())
      logmsg(kLERROR, "Installation failed - variables.sh broken.");

   // sort out all the volumes. Allow names to change between backup and restore, but not number of
   // volumes or order.
   if (oldvars.getDockerVols().size() != newvars.getDockerVols().size())
      logmsg(kLERROR, "Number of docker volumes stored does not match what we expect. Can't restore backup.");
   for (uint i = 0; i < oldvars.getDockerVols().size(); ++i)
   {
      if (utils::dockerVolExists(newvars.getDockerVols()[i]))
         logmsg(kLERROR, "Restore failed - there's an existing volume in the way: " + newvars.getDockerVols()[i] + " ... install then obliterate to clear it.");
      compress::decompress_volume(password, newvars.getDockerVols()[i], tempf, oldvars.getDockerVols()[i] + ".tar.7z", mParams);
   }

   // tell the dService to do its restore_end action.
   tVecStr args;
   args.push_back(tempc);
   servicehook hook(this, "restore", args, mParams);
   hook.endhook();

   logmsg(kLINFO, "The backup " + bf + " has been restored to service " + getName() + ". Try it!");
}

