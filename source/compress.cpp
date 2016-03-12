#include <iostream>

#include "compress.h"
#include "utils.h"
#include "logmsg.h"
#include "params.h"

namespace compress
{


   bool compress(std::string password, std::string volumename,
      std::string archivefolder, std::string archivename, const params & p)

   {
      if (!utils::fileexists(archivefolder))
         fatal("Can't archive to non-existant folder " + archivefolder);

      if (utils::fileexists(archivename))
         fatal("Can't compress to already existing " + archivename);


      std::string cmd;
      std::vector<std::string> args;

      cmd = "docker";
      args.push_back(cmd);
      args.push_back("run");
      args.push_back("-i");
      args.push_back("--name=\"dr_compress\"");
      if (password.length() > 0)
      {
         logmsg(kLDEBUG, "Using password supplied.", p);
         args.push_back("-e");
         args.push_back("PASS=\"" + password + "\"");
      }
      args.push_back("-v");
      args.push_back(volumename + ":/src");
      args.push_back("-v");
      args.push_back(archivefolder + ":/dst");
      args.push_back("drunner/install-rootutils");
      args.push_back("bash");
      args.push_back("-c");
      args.push_back("/usr/local/bin/dr_compress " + archivename + " && chmod 0666 /dst/" + archivename);

      utils::dockerrun dr(cmd, args,"dr_compress",p);

      return true;
   }


   bool compress_volume(std::string password, std::string volumename,
      std::string archivefolder, std::string archivename, const params & p)
   {
      if (!utils::dockerVolExists(volumename))
         fatal("Can't compress non-existant volume " + volumename);

      return compress(password, volumename, archivefolder+"/", archivename,p);
   }
         
         


   bool compress_folder(std::string password, std::string foldername,
      std::string archivefolder, std::string archivename, const params & p)
   {
      if (!utils::fileexists(foldername))
         fatal("Can't archive non-existant folder " + foldername);
      std::string ap = utils::getcanonicalpath(foldername);

      return compress(password, ap+"/", archivefolder+"/", archivename,p);
   }



} // namespace