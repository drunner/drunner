#include <sstream>

#include "command_setup.h"
#include "utils.h"
#include "utils_docker.h"
#include "drunnerSettings.h"
#include "globallogger.h"
#include "globalcontext.h"
#include "generate_validator_image.h"
#include "drunnercompose.h"

#include <Poco/Process.h>
#include <Poco/Path.h>
#include <Poco/File.h>

namespace command_setup
{

   int setup()
   {
      const params & p(*GlobalContext::getParams().get());
      const drunnerSettings & settings(*GlobalContext::getSettings().get());

      // -----------------------------------------------------------------------------
      // create rootpath if it doesn't exist.
      if (!utils::fileexists(settings.getPath_Root()))
         logmsg(kLDEBUG,"Root path already exists: " + settings.getPath_Root().toString());
      else
         utils::makedirectory(settings.getPath_Root(), S_755);

      // -----------------------------------------------------------------------------
      // Update settings on disk.
      logmsg(kLDEBUG, "Writing drunner master settings to "+settings.getPath_drunnercfg_sh().toString());
      if (!settings.writeSettings())
         logmsg(kLERROR, "Couldn't write settings file!");

      // -----------------------------------------------------------------------------
      // move this executable to the directory.
      //int result = rename( utils::get_exefullpath().c_str(), (rootpath+"/drunner").c_str());
      std::string drunnerexe = settings.getPath_Root().setFileName("drunner").toString();
      logmsg(kLDEBUG, "Copying the drunner executable to " + drunnerexe);
      Poco::File f(drunnerSettings::getPath_Exe());
      f.copyTo(drunnerexe);

      // -----------------------------------------------------------------------------
      // create bin directory
      Poco::Path bindir = utils::get_usersbindir();
      utils::makedirectory(bindir, S_700);

      // -----------------------------------------------------------------------------
      // create symlink
      Poco::Path symsource = settings.getPath_Root().setFileName("drunner");
      Poco::Path symlink = bindir;
      symlink.setFileName("drunner");
      utils::makesymlink(symsource, symlink);
      
      // -----------------------------------------------------------------------------
      // generate plugin scripts
      GlobalContext::getPlugins()->generate_plugin_scripts();

      // -----------------------------------------------------------------------------
      // get latest root util image.
      //std::cerr << "ROOTUITILIMAGE = " << settings.getRootUtilImage() << std::endl;
      utils_docker::pullImage(settings.getRootUtilImage());

      // -----------------------------------------------------------------------------
      // create services, support and temp directories
      utils::makedirectory(settings.getPath_dServices(), S_755);
      utils::makedirectory(settings.getPath_Support(), S_755);
      utils::makedirectory(settings.getPath_Temp(), S_755);
      utils::makedirectory(settings.getPath_HostVolumes(), S_755);

      // create the validator script that is run inside containers
      generate_validator_image(settings.getPath_Support());

      // -----------------------------------------------------------------------------
      // Finished!
      if (settings.mReadOkay)
         logmsg(kLINFO, "Update of drunner to " + p.getVersion() + " completed succesfully.");
      else
         logmsg(kLINFO, "Setup of drunner " + p.getVersion() + " completed succesfully.");

      return 0;
   }

   int update()
   {
      const params & p(*GlobalContext::getParams().get());
      const drunnerSettings & s(*GlobalContext::getSettings().get());

      logmsg(kLDEBUG, "Updating dRunner in " + s.getPath_Root().toString());

      std::string url(s.getdrunnerInstallURL());
      Poco::Path trgt(s.getPath_Root());
      trgt.setFileName("drunner-install");

      utils::downloadexe(url, trgt);

      logmsg(kLINFO, "Updating...");

      tVecStr args;
      args.push_back("drunner-install");
      for (auto opt : p.getOptions())
         args.push_back(opt);
      args.push_back("setup");
      args.push_back(s.getPath_Root().toString());
      
      std::ostringstream oss;
      for (auto arg : args)
         oss << arg << " ";
      logmsg(kLDEBUG, utils::trim_copy(oss.str()));

      Poco::ProcessHandle ph = Poco::Process::launch(trgt.toString(), args);
      int result = ph.wait();
      return result;
   }

}
