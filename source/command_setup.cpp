#include <boost/filesystem.hpp>
#include <sstream>

#include "command_setup.h"
#include "utils.h"
#include "utils_docker.h"
#include "sh_drunnercfg.h"
#include "logmsg.h"
#include "generate_validator_image.h"
#include "drunnercompose.h"

namespace command_setup
{

   int setup(const params & p)
   {
      if (p.getArgs().size() < 1)
         logmsg(kLERROR, "Usage:\n   drunner setup ROOTPATH", p);

      // -----------------------------------------------------------------------------
      // determine rootpath.
      std::string rootpath = utils::getabsolutepath(p.getArgs()[0]);
      if (rootpath.length() == 0)
         logmsg(kLERROR, "Couldn't determine path for " + p.getArgs()[0], p);

      // -----------------------------------------------------------------------------
      // create rootpath if it doesn't exist.
      utils::makedirectory(rootpath, p, S_755);

      // -----------------------------------------------------------------------------
      // now that rootpath is created we can get concrete path to it.
      rootpath = utils::getcanonicalpath(rootpath);

      // -----------------------------------------------------------------------------
      // create the settings and write to config.sh
      sh_drunnercfg settings(rootpath);
      if (!settings.write())
         logmsg(kLERROR, "Couldn't write settings file!", p);

      // -----------------------------------------------------------------------------
      // move this executable to the directory.
      //int result = rename( utils::get_exefullpath().c_str(), (rootpath+"/drunner").c_str());
      if (!utils::copyfile(utils::get_exefullpath(), rootpath + "/drunner"))
         logmsg(kLERROR, "Couldn't copy drunner executable from " + utils::get_exefullpath() + " to " + rootpath + ".", p);

      // -----------------------------------------------------------------------------
      // create bin directory
      std::string bindir = utils::get_usersbindir();
      utils::makedirectory(bindir, p, S_700);

      // -----------------------------------------------------------------------------
      // create symlink
      utils::makesymlink(rootpath + "/drunner", bindir + "/drunner", p);

      // sort out docker-compose - now expected to be present.
      //InstallDockerCompose(p);

      // get latest root util image.
      //std::cerr << "ROOTUITILIMAGE = " << settings.getRootUtilImage() << std::endl;
      utils_docker::pullImage(p, settings, settings.getRootUtilImage());

      // -----------------------------------------------------------------------------
      // create services, support and temp directories
      utils::makedirectory(settings.getPath_dServices(), p, S_755);
      utils::makedirectory(settings.getPath_Support(), p, S_755);
      utils::makedirectory(settings.getPath_Temp(), p, S_755);
      utils::makedirectory(settings.getPath_HostVolumes(), p, S_755);

      // create the validator script that is run inside containers
      generate_validator_image(settings.getPath_Support(), p);

      // -----------------------------------------------------------------------------
      // Finished!
      if (settings.readOkay())
         logmsg(kLINFO, "Update of drunner to " + p.getVersion() + " completed succesfully.", p);
      else
         logmsg(kLINFO, "Setup of drunner " + p.getVersion() + " completed succesfully.", p);

      return 0;
   }

   int update(const params & p, const sh_drunnercfg & s)
   {
      logmsg(kLDEBUG, "Updating dRunner in " + s.getPath_Root(), p);

      std::string url(s.getdrunnerInstallURL()), trgt(s.getPath_Root() + "/drunner-install");
      utils::downloadexe(url, trgt, p);

      logmsg(kLINFO, "Updating...", p);

      tVecStr args;
      args.push_back("drunner-install");
      for (auto opt : p.getOptions())
         args.push_back(opt);
      args.push_back("setup");
      args.push_back(s.getPath_Root());
      
      std::ostringstream oss;
      for (auto arg : args)
         oss << arg << " ";
      logmsg(kLDEBUG, utils::trim_copy(oss.str()), p);

      utils::execv(trgt, args);

      logmsg(kLERROR, "Exec failed.", p);
      return 1;
   }

}
