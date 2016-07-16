#include <sstream>

#include "drunner_setup.h"
#include "utils.h"
#include "utils_docker.h"
#include "drunner_settings.h"
#include "globallogger.h"
#include "globalcontext.h"
#include "drunner_paths.h"
#include "checkprerequisits.h"
#include "validateimage.h"

#include <Poco/Process.h>
#include <Poco/Path.h>
#include <Poco/File.h>

namespace drunnerSetup
{

   void _copyexe()
   {
      Poco::Path drunnerexe = drunnerPaths::getPath_Exe();

      try 
      {
         if (drunnerexe.parent().toString().compare(drunnerPaths::getPath_Bin().toString()) != 0)
            Poco::File(drunnerexe).copyTo(drunnerPaths::getPath_Bin().toString());
      }
      catch (const Poco::FileException & e)
      {
         logmsg(kLWARN, std::string("Couldn't copy exe to bin directory: ") + e.what());
      }
   }

   cResult check_setup(bool forceUpdate)
   {
      const params & p(*GlobalContext::getParams().get());

      // -----------------------------------------------------------------------------
      // create rootpath if it doesn't exist.
#ifndef _DEBUG
      if (utils::fileexists(drunnerPaths::getPath_Root()) && !forceUpdate)
         return kRNoChange;
#endif

      // check prereqs (e.g. docker installed).
      check_prerequisits();

      // -----------------------------------------------------------------------------
      // create directory structure.
      utils::makedirectory(drunnerPaths::getPath_Root(), S_755);
      // make root hidden on windows.
#ifdef _WIN32
      if (!Poco::File(drunnerPaths::getPath_Root()).isHidden())
         SetFileAttributes(drunnerPaths::getPath_Root().toString().c_str(), FILE_ATTRIBUTE_HIDDEN);
#endif

      utils::makedirectory(drunnerPaths::getPath_Bin(), S_700);
      _copyexe();

      utils::makedirectory(drunnerPaths::getPath_dServices(), S_755);
      utils::makedirectory(drunnerPaths::getPath_Support(), S_755);
      utils::makedirectory(drunnerPaths::getPath_Temp(), S_755);
      utils::makedirectory(drunnerPaths::getPath_HostVolumes(), S_755);

      // -----------------------------------------------------------------------------
      // generate plugin scripts
      GlobalContext::getPlugins()->generate_plugin_scripts();

      // -----------------------------------------------------------------------------
      // get latest root util image.
      utils_docker::pullImage(drunnerPaths::getdrunnerUtilsImage());

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
      logmsg(kLERROR, "Not implemented.");
      return kRError;

      //const params & p(*GlobalContext::getParams().get());
      //const drunnerSettings & s(*GlobalContext::getSettings().get());

      //logmsg(kLDEBUG, "Updating dRunner in " + drunnerPaths::getPath_Root().toString());

      //std::string url(s.getdrunnerInstallURL());
      //Poco::Path trgt(drunnerPaths::getPath_Root());
      //trgt.setFileName("drunner-install");

      //utils::downloadexe(url, trgt);

      //logmsg(kLINFO, "Updating...");

      //tVecStr args;
      //args.push_back("drunner-install");
      //for (auto opt : p.getOptions())
      //   args.push_back(opt);
      //args.push_back("setup");
      //args.push_back(drunnerPaths::getPath_Root().toString());
      //
      //std::ostringstream oss;
      //for (auto arg : args)
      //   oss << arg << " ";
      //logmsg(kLDEBUG, utils::trim_copy(oss.str()));

      //Poco::ProcessHandle ph = Poco::Process::launch(trgt.toString(), args);
      //int result = ph.wait();
      //return result;
   }

}
