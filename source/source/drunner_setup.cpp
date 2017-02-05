#include <sstream>
#include <string>
#include <fstream>

#include "drunner_setup.h"
#include "utils.h"
#include "utils_docker.h"
#include "drunner_settings.h"
#include "globallogger.h"
#include "globalcontext.h"
#include "drunner_paths.h"
#include "dassert.h"

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

   bool _file_has_line(std::string file, std::string line)
   {
      bool hasLine = false;
      std::ifstream mf(file);
      std::string l;

      if (!mf.is_open())
         return false;

      while (!mf.eof())
      {
         getline(mf, l);
         Poco::trimInPlace(l);
         if (l.compare(line) == 0)
         {
            mf.close();
            hasLine = true;
            break;
         }
      }
      mf.close();
      return hasLine;
   }

   void _ensure_line(std::string file, std::string line)
   {
      if (_file_has_line(file, line))
      {
         logdbg(file + " is configured.");
         return;
      }
      logdbg("Appending to " + file + " :\n " + line);
      std::ofstream f(file, std::ios_base::app | std::ios_base::out);
      f << line << "\n";
   }

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
      _check_prereqs_xplatform();

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

      // check ~/.drunner/bin is in ~/.profile.
      _ensure_line(Poco::Path::home() + ".profile", "PATH=\"$HOME/.drunner/bin:$PATH\"");
   }
#endif

   void _copyexe()
   {
      Poco::Path currentexe = drunnerPaths::getPath_Exe();

      try 
      {
         if (currentexe.parent().toString().compare(drunnerPaths::getPath_Bin().toString()) != 0)
            Poco::File(currentexe).copyTo(drunnerPaths::getPath_Bin().toString());
         logdbg("Copied drunner to "+ drunnerPaths::getPath_Bin().toString());
         drunner_assert(utils::fileexists(drunnerPaths::getPath_Exe_Target()), "Failed to install drunner.exe");
      }
      catch (const Poco::FileException & e)
      {
         logmsg(kLWARN, std::string("Couldn't copy exe to bin directory: ") + e.what());
      }
   }

   void _makedirectory(Poco::Path d, mode_t mode)
   {
      cResult r = utils::makedirectory(d, mode);
      if (!r.success())
         fatal("Couldn't create directory " + d.toString() + "\nError: " + r.what());
   }

   cResult check_setup()
   {
      const params & p(*GlobalContext::getParams().get());

      // check prereqs (e.g. docker installed).
      _check_prerequisits();

      // -----------------------------------------------------------------------------
      // create directory structure.
      _makedirectory(drunnerPaths::getPath_Root(), S_755);
      _makedirectory(drunnerPaths::getPath_Bin(), S_700);
      _makedirectory(drunnerPaths::getPath_dServices(), S_755);
      _makedirectory(drunnerPaths::getPath_Temp(), S_755);
      _makedirectory(drunnerPaths::getPath_HostVolumes(), S_755);
      _makedirectory(drunnerPaths::getPath_Settings(), S_755);
      _makedirectory(drunnerPaths::getPath_Logs(), S_755);


      // On windows, we copy the executable to the .drunner/bin folder and make the .drunner folder hidden.
      // On Linux drunner is manually copied to /usr/local/bin (or wherever) by the user as part of the install process.
#ifdef _WIN32
      _copyexe();
      // make root hidden on windows.
      drunner_assert(0 != SetFileAttributesA(drunnerPaths::getPath_Root().toString().c_str(), FILE_ATTRIBUTE_HIDDEN), "Couldn't change attributes on the .drunner directory");
#endif

      // -----------------------------------------------------------------------------
      // generate plugin scripts
      GlobalContext::getPlugins()->generate_plugin_scripts();

      // -----------------------------------------------------------------------------
      // get latest root util image.
      utils_docker::pullImage(drunnerPaths::getdrunnerUtilsImage());

      // write settings.
      GlobalContext::getSettings()->savevariables();

      // -----------------------------------------------------------------------------
      // Finished!
      return kRSuccess;
   }


   cResult update_drunner()
   {
      // download and install the latest drunner.

#ifdef _WIN32
      fatal("Update not yet supported on Windows.");
#else

      std::string targetdir = "/tmp/drunner";
      std::string target = targetdir + "/drunner-install";

      utils::makedirectory(targetdir, S_700);
      std::string op;
      CommandLine cl("docker", { "run","--rm","-v",targetdir+":/dtemp","drunner/drunner_utils","download_drunner_install" });
      int r = utils::runcommand_stream(cl, kORaw, "", {}, &op);
      if (r != 0)
         fatal("Update script failed:\n "+op);
   
      if (!utils::fileexists(target))
         fatal("Couldn't download drunner-install.");

      // exec to switch to that process.
      const char    *my_argv[64] = { "/tmp/drunner/drunner-install",NULL};
      execve(my_argv[0], (char **)my_argv, NULL);

      fatal("Execution failed");
#endif

      return kRSuccess;
   }
}
