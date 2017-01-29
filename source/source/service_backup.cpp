#include <cstdlib>

#include <Poco/File.h>
#include <Poco/String.h>

#include "service.h"
#include "utils.h"
#include "compress.h"
#include "globallogger.h"
#include "globalcontext.h"
#include "timez.h"
#include "drunner_paths.h"
#include "service_manage.h"
#include "utils_docker.h"
#include "dassert.h"
#include "service_lua.h"

// Back up this service to backupfile.
cResult service::backup(std::string backupfile)
{
   timez ttotal, tstep;
   std::string op;
   
   // -----------------------------------------
   // Preliminaries
   if (backupfile.length() == 0)
   {
      backupfile = getName() + "_";
      backupfile += timeutils::getDateTimeStr();
      backupfile += ".dbk";
   }
   
   Poco::Path bf(backupfile);
   bf.makeAbsolute();
   if (utils::fileexists(bf))
      return cError("Backup file " + bf.toString() + " already exists. Aborting.");

   backupPathManager paths(mName);

   logmsg(kLINFO, "Time for preliminaries:           " + tstep.getelpased());
   tstep.restart();

   // -----------------------------------------
   // back up whatever the dService tells us to
   mServiceVars.setTempBackupFolder(paths.getPathSubArchives().toString());
   servicelua::luafile lf(mServiceVars, CommandLine("backup"));
   if (!lf.getResult().success())
      fatal("Failed to run backup command in the dService's service.lua.");

   logmsg(kLINFO, "Time for volume backups:      " + tstep.getelpased());
   tstep.restart();

   // -----------------------------------------
   // back up host vol (local storage) - password not used.
   logmsg(kLDEBUG, "Backing up host volume.");
   compress::compress_folder("", getPathHostVolume(), paths.getPathHostVolArchiveFile());
   
   logmsg(kLINFO, "Time for host volume backup:  " + tstep.getelpased());
   tstep.restart();

   // ------------------------------------------
   // back up dservice definition files - password not used.

   logmsg(kLDEBUG, "Backing up dService definition files.");
   compress::compress_folder("", getPathdService(), paths.getPathdServiceDefArchiveFile());

   logmsg(kLINFO, "Time for dService def backup: " + tstep.getelpased());
   tstep.restart();



   // -----------------------------------------
   // compress everything together
   std::string password = utils::getenv("PASS");
   Poco::Path bigarchive(paths.getPathArchiveFile());
   bool ok=compress::compress_folder(password, paths.getPathSubArchives(), bigarchive);
   if (!ok)
      logmsg(kLERROR, "Couldn't archive service " + getName());

   logmsg(kLINFO, "Time to compress and encrypt:     " + tstep.getelpased());
   tstep.restart();

   // -----------------------------------------
   // move compressed file to target dir.
   Poco::File bigafile(bigarchive);
   if (!bigafile.exists())
      logmsg(kLERROR, "Expected archive not found at " + bigarchive.toString());

   logdbg("Moving " + bigarchive.toString() + " to " + bf.toString());
   bigafile.copyTo(bf.toString());
   bigafile.remove();

   logmsg(kLINFO, "Time to move archive:             " + tstep.getelpased());
   tstep.restart();

   // -----------------------------------------
   // Output results.
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

// servicename can be empty, in which case it's determined from the imagename.
cResult service_manage::service_restore(const std::string & backupfile, std::string servicename)
{ // restore from backup.
   timez ttotal, tstep;

   Poco::Path bf(backupfile);
   bf.makeAbsolute();
   if (!utils::fileexists(bf))
      fatal("Backup file " + backupfile + " does not exist.");

   servicePaths servicepaths(servicename);
   backupPathManager backuppaths(servicename);
   std::string password = utils::getenv("PASS");

   if (utils::fileexists(servicepaths.getPathdService()))
      fatal("Can't restore to " + servicename + " - it already exists. Obliterate first or choose another service name.");
   if (utils::fileexists(servicepaths.getPathHostVolume()))
      fatal("Can't restore to " + servicename + " - data already exists. Obliterate first or choose another service name.");

   logmsg(kLDEBUG, "Restoring from " + bf.toString());

   // -----------------------------------------
   // decompress main backup. copy to ensure on same physical volume (or decompress can fail).
   Poco::File bff(bf);
   bff.copyTo(backuppaths.getPathArchiveFile().toString());
   compress::decompress_folder(password, backuppaths.getPathSubArchives(), backuppaths.getPathArchiveFile());

   if (!utils::fileexists(backuppaths.getPathSubArchives()))
      logmsg(kLERROR, "Backup corrupt - missing " + backuppaths.getPathSubArchives().toString());

   // backup seems okay - lets go!
   service_manage::_create_common(servicename);

   // -----------------------------------------
   // restore host vol (local storage)
   logmsg(kLDEBUG, "Restoring host volume.");
   compress::decompress_folder("", servicepaths.getPathHostVolume(), backuppaths.getPathHostVolArchiveFile());

   logmsg(kLINFO, "Time for host volume restore:  " + tstep.getelpased());
   tstep.restart();

   // ------------------------------------------
   // restore dservice definition files

   logmsg(kLDEBUG, "Restoring dService definition files.");
   compress::decompress_folder("", servicepaths.getPathdService(), backuppaths.getPathdServiceDefArchiveFile());

   logmsg(kLINFO, "Time for dService def restore: " + tstep.getelpased());
   tstep.restart();


   // -----------------------------------------
   // restore whatever the dService tells us to
   serviceVars sv(servicename);
   sv.setTempBackupFolder(backuppaths.getPathSubArchives().toString());
   servicelua::luafile lf(sv, CommandLine("restore"));
   if (!lf.getResult().success())
      fatal("Failed to run restore command in the dService's service.lua.");

   logmsg(kLINFO, "Time for volume restore:      " + tstep.getelpased());
   tstep.restart();

   // -----------------------------------------
   // Output results.
   logmsg(kLINFO, "The backup " + bf.toString() + " has been restored to service " + servicename + ". Try it!");
   logmsg(kLINFO, "Total time taken:                 " + ttotal.getelpased());
   return kRSuccess;
}

