#include <sstream>

#include "drunner_setup.h"
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

namespace drunner_setup
{

   cResult check_setup(bool forceUpdate)
   {
      const params & p(*GlobalContext::getParams().get());

      // -----------------------------------------------------------------------------
      // create rootpath if it doesn't exist.
      if (!utils::fileexists(drunnerSettings::getPath_Root()))
         utils::makedirectory(drunnerSettings::getPath_Root(), S_755);
      else if (!forceUpdate)
         return kRNoChange; // nothing needed.

      // -----------------------------------------------------------------------------
      // create bin directory
      utils::makedirectory(drunnerSettings::getPath_Bin(), S_700);
      
      // -----------------------------------------------------------------------------
      // generate plugin scripts
      GlobalContext::getPlugins()->generate_plugin_scripts();

      // -----------------------------------------------------------------------------
      // get latest root util image.
      utils_docker::pullImage(drunnerSettings::getdrunnerUtilsImage());

      // -----------------------------------------------------------------------------
      // create services, support and temp directories
      utils::makedirectory(drunnerSettings::getPath_dServices(), S_755);
      utils::makedirectory(drunnerSettings::getPath_Support(), S_755);
      utils::makedirectory(drunnerSettings::getPath_Temp(), S_755);
      utils::makedirectory(drunnerSettings::getPath_HostVolumes(), S_755);

      // create the validator script that is run inside containers
      generate_validator_image(drunnerSettings::getPath_Support());

      // write settings.
      GlobalContext::getSettings()->writeSettings();

      // -----------------------------------------------------------------------------
      // Finished!
      if (forceUpdate)
         logmsg(kLINFO, "Update of drunner to " + p.getVersion() + " completed succesfully.");
      else
         logmsg(kLINFO, "Initial setup of drunner " + p.getVersion() + " completed succesfully.");

      return kRSuccess;
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
