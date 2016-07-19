#include <iostream>

#include <Poco/Path.h>
#include <Poco/File.h>

#include "compress.h"
#include "utils.h"
#include "globallogger.h"
#include "params.h"
#include "utils_docker.h"
#include "drunner_paths.h"
#include "service_variables.h"
#include "globalcontext.h"

namespace compress
{
   void _rundocker(std::string src, std::string dst, std::string passwd, std::string ctrcmd)
   {
      std::string cmd("docker");
      std::vector<std::string> args = { "run","--rm","-v",src + ":/src","-v",dst + ":/dst" };

      if (passwd.length() > 0)
      {
         logmsg(kLDEBUG, "Using password supplied.");
         args.push_back("-e");
         args.push_back("PASS=\"" + passwd + "\"");
      }
      args.push_back(drunnerPaths::getdrunnerUtilsImage());
      args.push_back("bash");
      args.push_back("-c");
      args.push_back(ctrcmd);

      utils::runcommand_stream(cmd, args, GlobalContext::getParams()->getServiceOutput_supportcalls() );
   }

   // --------------------------------------

   bool _compress(std::string password, std::string volumename, Poco::Path archive)
   {
      std::string archivefolder = archive.parent().toString();
      std::string archivename = archive.getFileName();

      if (!utils::fileexists(archivefolder))
         fatal("Can't archive to non-existant folder " + archivefolder);

      if (utils::fileexists(archivefolder+archivename))
         fatal("Can't compress to already existing " + archivename);

      _rundocker(volumename, archivefolder, password,
         "/usr/local/bin/dr_compress_fast " + archivename + " && chmod 0666 /dst/" + archivename );

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