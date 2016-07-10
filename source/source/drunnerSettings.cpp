#include "drunnerSettings.h"
#include "utils.h"
#include "globallogger.h"

#include <Poco/Path.h>
#include <Poco/UnicodeConverter.h>

#ifdef _WIN32
#include <ShlObj.h>
#else
#include <linux/limits.h>
#include <unistd.h>
#endif

// can't put this in header because circular
// dependency then with utils::getTime.
drunnerSettings::drunnerSettings() :
   settingsbash(false)
{
   setString("DRUNNERUTILSIMAGE", "drunner/drunner_utils");
   setString("DRUNNERINSTALLURL", R"EOF(https://drunner.s3.amazonaws.com/drunner-install)EOF");
   setString("DRUNNERINSTALLTIME", utils::getTime());
   setBool("PULLIMAGES", true);

   mReadOkay = false;
   readSettings();
}

bool drunnerSettings::readSettings()
{
   mReadOkay = settingsbash::readSettings(getPath_drunnerSettings_sh());
   if (!mReadOkay)
      return false;

   // migrate old settings.
   if (utils::findStringIC(getdrunnerUtilsImage(), "drunner/rootutils"))
   {
      setString("DRUNNERUTILSIMAGE", "drunner/drunner_utils");
      if (!writeSettings())
         fatal("Couldn't migrate old dRunner settings."); // couldn't migrate settings.
   }
   return mReadOkay;
}

bool drunnerSettings::writeSettings() const
{
   return settingsbash::writeSettings(getPath_drunnerSettings_sh());
}

Poco::Path drunnerSettings::getPath_Root() {
#ifdef _WIN32
   wchar_t * ppszPath = NULL;
   if (S_OK != SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_DEFAULT, NULL, &ppszPath))
      fatal("Could not determine the APPDATA folder for the current user.");
   std::wstring wpath(ppszPath);
   CoTaskMemFree(ppszPath);

   std::string path;
   Poco::UnicodeConverter::convert(wpath, path);
   Poco::Path drunnerdir(path);
   drunnerdir.makeDirectory();
   drunnerdir.pushDirectory("drunner");   
#else
   Poco::Path drunnerdir = Poco::Path::home();
   drunnerdir.makeDirectory();
   poco_assert(drunnerdir.isDirectory());
   drunnerdir.pushDirectory(".drunner");
#endif
   //logmsg(kLDEBUG, "drunner directory is " + drunnerdir.toString());

   return drunnerdir;
}

namespace _drunnerSettings
{

   // get the full path the the current executable.
   std::string get_exefullpath()
   {
#ifdef _WIN32
      std::vector<char> pathBuf;
      DWORD copied = 0;
      do {
         pathBuf.resize(pathBuf.size() + MAX_PATH);
         copied = GetModuleFileNameA(0, &pathBuf.at(0), pathBuf.size());
      } while (copied >= pathBuf.size());
      pathBuf.resize(copied);

      std::string path(pathBuf.begin(), pathBuf.end());
      return path;
#else
      char buff[PATH_MAX];
      ssize_t len = ::readlink("/proc/self/exe", buff, sizeof(buff) - 1);
      if (len != -1)
      {
         buff[len] = '\0';
         return std::string(buff);
      }
      logmsg(kLERROR, "Couldn't get path to drunner executable!");
      return "";
#endif
   }

};

Poco::Path drunnerSettings::getPath_Exe()
{
   Poco::Path p(_drunnerSettings::get_exefullpath());
   poco_assert(p.isFile());
   poco_assert(utils::fileexists(p));

   return p;
}


