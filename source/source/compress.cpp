#include <iostream>

#include <Poco/Path.h>
#include <Poco/File.h>

#include "compress.h"
#include "utils.h"
#include "globallogger.h"
#include "params.h"
#include "utils_docker.h"
#include "drunner_paths.h"
#include "globalcontext.h"

namespace compress
{
   void _rundocker(std::string src, std::string dst, std::string passwd, std::string ctrcmd)
   {
      CommandLine cl;
      cl.command = "docker";

      if (passwd.length()==0)
         cl.args = { "run","--rm","-v",src + ":/src","-v",dst + ":/dst", 
            drunnerPaths::getdrunnerUtilsImage(), "bash", "-c", ctrcmd};
      else
         cl.args = { "run","--rm","-v",src + ":/src","-v",dst + ":/dst",
            "-e","PASS=\"" + passwd + "\"",
            drunnerPaths::getdrunnerUtilsImage(), "bash", "-c", ctrcmd };

      utils::runcommand_stream(cl, GlobalContext::getParams()->supportCallMode(), "", {},NULL);
   }

   // --------------------------------------

   bool _compress(std::string password, std::string volumename, Poco::Path archive)
   {
      logdbg("Backing up " + volumename + " to " + archive.toString());
      std::string archivefolder = archive.parent().toString();
      std::string archivename = archive.getFileName();

      if (!utils::fileexists(archivefolder))
         fatal("Can't archive to non-existant folder " + archivefolder);

      if (utils::fileexists(archivefolder+archivename))
         fatal("Can't compress to already existing " + archivename);

      std::string cmd = "/usr/local/bin/dr_compress_fast " + archivename;
#ifdef _WIN32
#else
      std::string uid = std::to_string(getuid());
      cmd += " && chown "+uid+" /dst/" + archivename;
#endif

      _rundocker(volumename, archivefolder, password, cmd);

      if (!utils::fileexists(archivefolder + archivename))
         fatal("Archive doesn't exist: " + archivefolder + archivename);

      return true;
   }

   // --------------------------------------

   bool _decompress(std::string password, std::string volumename, Poco::Path archive)
   {
      std::string archivefolder = archive.parent().toString();
      std::string archivename = archive.getFileName();

      if (!utils::fileexists(archivefolder) || !utils::fileexists(archivefolder + archivename))
         fatal("Can't decompress missing archive " + archivefolder + archivename);

      _rundocker(archivefolder, volumename, password,
         "/usr/local/bin/dr_decompress_fast "+archivename );

      return true;
   }

   // --------------------------------------


   bool compress_volume(std::string password, std::string volumename, Poco::Path archive)
   {
      if (!utils_docker::dockerVolExists(volumename))
         fatal("Can't compress non-existant volume " + volumename);

      return _compress(password, volumename, archive);
   }

   // --------------------------------------

   bool compress_folder(std::string password, Poco::Path foldername, Poco::Path archive)
   {
      Poco::File f(foldername);
      if (!f.exists())
         fatal("Can't archive non-existant folder " + foldername.toString());
      
      return _compress(password, foldername.absolute().toString(), archive);
   }

   // --------------------------------------

   bool decompress_volume(std::string password, std::string targetvolumename, Poco::Path archive)
   {
      if (!utils_docker::dockerVolExists(targetvolumename))
         fatal("Can't restore into volume " + targetvolumename + " because it doesn't exist.");

      return _decompress(password, targetvolumename, archive);
   }


   // --------------------------------------

   bool decompress_folder(std::string password, Poco::Path targetfoldername, Poco::Path archive)
   {
      Poco::File tf(targetfoldername);
      if (!tf.exists())
         fatal("Can't archive to non-existant folder " + targetfoldername.toString());

      return _decompress(password, targetfoldername.absolute().toString(), archive);
   }


} // namespace