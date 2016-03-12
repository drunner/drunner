#include "compress.h"
#include "utils.h"
#include "logmsg.h"

namespace compress
{

   bool compress_volume(std::string password, std::string volumename,
      std::string archivefolder, std::string archivename)
   {
      if (!utils::dockerVolExists(volumename))
         fatal("Can't compress non-existant volume " + volumename);

      if (!utils::fileexists(archivefolder))
         fatal("Can't archive to non-existant folder " + archivefolder);

      if (utils::fileexists(archivename))
         fatal("Can't compress to already existing " + archivename);

      std::string op, cmd;

      cmd = "docker run -i --name=dr_compress ";
      if (password.length() > 0)
         cmd += "-e \"PASS=" + password + " ";

      cmd += " -v " + volumename + ":/src -v " + archivefolder + ":/dst drunner/install-rootutils ";

      cmd += "bash -c \"dr_compress " + archivename + " && chmod 0755 /dst/" + archivename;


      if (0 != utils::bashcommand(cmd, op))
         fatal("Failed to archive volume " + volumename);

      return true;
   }


} // namespace