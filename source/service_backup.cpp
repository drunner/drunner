#include "service.h"
#include "utils.h"
#include "sh_variables.h"

class tempfolder
{
public:
   tempfolder(const std::string & s, const params & p) : mPath(s), mP(p) {
      utils::makedirectory(mPath, mP, S_777);
   }

   ~tempfolder() {
      utils::deltree(mPath, mP);
   }

   const std::string & getpath() { return mPath; }

private:
   std::string mPath;
   const params & mP;
};


void service::backup(const std::string & backupfile)
{
   std::string op, bf = utils::getabsolutepath(backupfile);
   if (utils::fileexists(bf))
      logmsg(kLERROR, "Backup file " + bf + " already exists. Aborting.");

   if (!isValid())
      logmsg(kLERROR, "Validation of " + getName() + " failed. Try drunner recover " + getName());

   tempfolder tempparent(mSettings.getPath_Temp() + "/backup-"+getName(), mParams);

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
   const std::vector<std::string> & dockervols(shv.getDockerVols());
   for (auto const & entry : dockervols)
   {
      // check if docker volume exists, if so back it up.
   }
}


void service::restore(const std::string & backupfile)
{ // restore from backup.
   std::string bf = utils::getcanonicalpath(backupfile);
   if (bf.length()==0 || !utils::fileexists(bf))
      logmsg(kLERROR, "Backup file " + backupfile + " does not exist.");
   logmsg(kLDEBUG, "Restoring from " + bf);

   if (utils::fileexists(getPath()))
      logmsg(kLERROR, "Service " + getName() + " already exists. Uninstall it before restoring from backup.");

   tempfolder tempparent(mSettings.getPath_Temp() + "/restore-"+getName(), mParams);
   
   // for docker volumes
   std::string tempf = tempparent.getpath() + "/drbackup";
   // for container custom backups (e.g. mysqldump)
   std::string tempc = tempparent.getpath() + "/containerbackup";



}
