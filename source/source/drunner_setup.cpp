#include <sstream>
#include <string>

#include "drunner_setup.h"
#include "utils.h"
#include "utils_docker.h"
#include "drunner_settings.h"
#include "globallogger.h"
#include "globalcontext.h"
#include "drunner_paths.h"
#include "validateimage.h"

#include <Poco/Process.h>
#include <Poco/Path.h>
#include <Poco/File.h>
#include <Poco/String.h>

namespace drunnerSetup
{

   void _check_prereqs_xplatform()
   {
      CommandLine cl("docker", { "version","--format","{{.Server.Version}}"});
      std::string op;
      if (utils::runcommand(cl, op) != 0)
         fatal("Running \"docker --version\" failed!\nIs docker correctly installed on this machine?\n" + op);
      std::string major, minor;
      unsigned int i;
      for (i=0; i < op.length() && op[i] != '.'; ++i)
         major += op[i];
      for (++i; i < op.length() && op[i] != '.'; ++i)
         minor += op[i];
      logdbg("Docker major version = " + major + " and minor = " + minor);
      if (std::strtol(major.c_str(),0,10) == 1 && std::stoi(minor.c_str(),0,10) < 12)
         fatal("dRunner requires docker version 1.12 or newer. Please update!");
   }

   // ---------------------------------------------------------------------------------------------------------------------------------------------
   // ---------------------------------------------------------------------------------------------------------------------------------------------
   // ---------------------------------------------------------------------------------------------------------------------------------------------


#ifdef _WIN32
   void _check_prerequisits() 
   { 
      _check_prereqs_xplatform(); 
   }
#else   // Linux
#include <unistd.h>

   int bashcommand(std::string bashline, std::string & op, bool trim)
   {
      CommandLine cl("/bin/bash", { "-c", bashline });
      int r=utils::runcommand(cl, op);
      if (trim)
         Poco::trimInPlace(op);
      return r;
   }
   int bashcommand(std::string bashline)
   {
      std::string op;
      return bashcommand(bashline, op, false);
   }
   bool commandexists(std::string command)
   {
      return (0 == bashcommand("command -v " + command));
   }
   std::string getUSER()
   {
      char s[200];
      getlogin_r(s, 199);
      return s;
   }

   void _check_prerequisits()
   {
      uid_t euid = geteuid();
      if (euid == 0)
         fatal("Please run as a standard user, not as root.");

      std::string user = getUSER();
      if (0 != bashcommand("groups $USER | grep docker"))
         fatal("Please add the current user to the docker group. As root: " + utils::kCODE_S + "adduser " + user + " docker" + utils::kCODE_E);

      if (0 != bashcommand("groups | grep docker"))
         fatal(user + " hasn't picked up group docker yet. Log out then in again, or run " + utils::kCODE_S + "exec su -l " + user + utils::kCODE_E);

      if (!commandexists("docker"))
         fatal("Please install Docker before using dRunner.\n(e.g. use  https://raw.githubusercontent.com/j842/scripts/master/install_docker.sh )");

      if (!commandexists("curl"))
         fatal("Please install curl before using dRunner.");

      _check_prereqs_xplatform();
   }
#endif

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

   void _makedirectory(Poco::Path d, mode_t mode)
   {
      cResult r = utils::makedirectory(d, mode);
      if (!r.success())
         fatal("Couldn't create directory " + d.toString() + "\nError: " + r.what());
   }

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
      _check_prerequisits();

      // -----------------------------------------------------------------------------
      // create directory structure.
      utils::makedirectory(drunnerPaths::getPath_Root(), S_755);
      // make root hidden on windows.
#ifdef _WIN32
      if (!Poco::File(drunnerPaths::getPath_Root()).isHidden())
         SetFileAttributes(drunnerPaths::getPath_Root().toString().c_str(), FILE_ATTRIBUTE_HIDDEN);
#endif

      _makedirectory(drunnerPaths::getPath_Bin(), S_700);
      _copyexe();

      _makedirectory(drunnerPaths::getPath_dServices(), S_755);
      _makedirectory(drunnerPaths::getPath_Support(), S_755);
      _makedirectory(drunnerPaths::getPath_Temp(), S_755);
      _makedirectory(drunnerPaths::getPath_HostVolumes(), S_755);
      _makedirectory(drunnerPaths::getPath_empty(), S_755);

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
}
