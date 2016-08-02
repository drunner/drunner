#include <Poco/UnicodeConverter.h>
#include <Poco/File.h>

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
   Poco::Path drunnerdir = Poco::Path::home();
   drunnerdir.makeDirectory();
   poco_assert(drunnerdir.isDirectory());
   drunnerdir.pushDirectory(".drunner");
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

Poco::Path drunnerPaths::getPath_Temp()
{
   return getPath_Root().pushDirectory("temp");
}

Poco::Path drunnerPaths::getPath_HostVolumes()
{
   return getPath_Root().pushDirectory("hostVolumes");
}

Poco::Path drunnerPaths::getPath_drunnerSettings_json()
{
   return getPath_Root().setFileName("drunnerSettings.json");
}

std::string drunnerPaths::getdrunnerUtilsImage()
{
   return  "drunner/drunner_utils";
}