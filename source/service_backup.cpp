#include <cstdlib>

#include <boost/filesystem.hpp>

#include "service.h"
#include "utils.h"
#include "sh_variables.h"
#include "compress.h"




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
   shv.writecopy(tempparent.getpath() + "/variables.sh"); // output.

   // path for docker volumes and for container custom backups (e.g. mysqldump)
   std::string tempf = tempparent.getpath() + "/drbackup";
   std::string tempc = tempparent.getpath() + "/containerbackup";
   utils::makedirectory(tempf, mParams, S_777); // random UID in container needs access.
   utils::makedirectory(tempc, mParams, S_777);

   // notify service we're starting our backup.
   utils::bashcommand(getPathServiceRunner() + " backupstart \"" + tempc + "\"", op);

   // back up volume containers
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
   utils::bashcommand(getPathServiceRunner() + " backupend \"" + tempc + "\"", op);

   // compress everything together
   boost::filesystem::path p(bf);
   bool ok=compress::compress_folder(password, tempparent.getpath(), p.parent_path().string(), p.filename().string(),mParams);
   if (!ok)
      logmsg(kLERROR, "Couldn't archive service " + getName());
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
   
   // for docker volumes
   std::string tempf = tempparent.getpath() + "/drbackup";
   // for container custom backups (e.g. mysqldump)
   std::string tempc = tempparent.getpath() + "/containerbackup";



}
