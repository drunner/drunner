#include <Poco/UnicodeConverter.h>

#include "drunner_paths.h"
#include "utils.h"
#include "globallogger.h"

#ifdef _WIN32
#include <ShlObj.h>
#else
#include <linux/limits.h>
#include <unistd.h>
#endif

Poco::Path drunnerPaths::getPath_Root() {
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

Poco::Path drunnerPaths::getPath_Exe()
{
   Poco::Path p;
#ifdef _WIN32
   std::vector<char> pathBuf;
   DWORD copied = 0;
   do {
      pathBuf.resize(pathBuf.size() + MAX_PATH);
      copied = GetModuleFileNameA(0, &pathBuf.at(0), pathBuf.size());
   } while (copied >= pathBuf.size());
   pathBuf.resize(copied);

   std::string path(pathBuf.begin(), pathBuf.end());
   p=Poco::Path(path);
#else
   char buff[PATH_MAX];
   ssize_t len = ::readlink("/proc/self/exe", buff, sizeof(buff) - 1);
   if (len==-1)
      logmsg(kLERROR, "Couldn't get path to drunner executable!");

   buff[len] = '\0';
   p=Poco::Path(buff);
#endif
   poco_assert(p.isFile());
   poco_assert(utils::fileexists(p));

   return p;
}

Poco::Path drunnerPaths::getPath_Bin()
{
   return getPath_Root().pushDirectory("bin");
} // bin subfolder 

Poco::Path drunnerPaths::getPath_dServices()
{
   return getPath_Root().pushDirectory("dServices");
}

Poco::Path drunnerPaths::getPath_Support()
{
   return getPath_Root().pushDirectory("support");
}

Poco::Path drunnerPaths::getPath_Temp()
{
   return getPath_Root().pushDirectory("temp");
}

Poco::Path drunnerPaths::getPath_HostVolumes()
{
   return getPath_Root().pushDirectory("hostVolumes");
}

Poco::Path drunnerPaths::getPath_drunnerSettings_sh()
{
   return getPath_Root().setFileName("drunnerSettings.sh");
}

Poco::Path drunnerPaths::getPath_drunnerbackups_cfg()
{
   return getPath_Root().setFileName("drunnerBackups.cfg");
}

std::string drunnerPaths::getdrunnerUtilsImage()
{
   return  "drunner/drunner_utils";
}