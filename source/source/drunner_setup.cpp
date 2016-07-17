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

   static bool s_setup_fully_checked = false;

   cResult check_setup(bool forceUpdate)
   {
      if (s_setup_fully_checked)
         return kRNoChange;

      const params & p(*GlobalContext::getParams().get());

      // -----------------------------------------------------------------------------
      // create rootpath if it doesn't exist.
      if (utils::fileexists(drunnerPaths::getPath_Root()) && !forceUpdate)
#ifndef _DEBUG
         return kRNoChange;
#else 
         logmsg(kLINFO, "Debug build - checking drunner setup is up to date.");
#endif
      else
         logmsg(kLINFO, forceUpdate ? "Ensuring drunner setup is up to date." :
            "Settup up drunner for the first time.");

      s_setup_fully_checked = true;

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
      utils::makedirectory(drunnerPaths::getPath_empty(), S_755);

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
      return kRSuccess;
   }

   cResult setup()
   {
      cResult rval = check_setup(true);
      logmsg(kLINFO, "Setup complete.");
      return rval;
   }

}
