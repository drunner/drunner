#ifndef __service_manage
#define __service_manage

#include <string>
#include "cresult.h"
#include "service_paths.h"
#include "service_lua.h"

namespace service_manage
{
   cResult obliterate(std::string servicename);
   cResult uninstall(std::string servicename);
   cResult update(std::string servicename);
   cResult install(std::string & servicename, bool devMode);
   cResult service_restore(const std::string & backupfile, std::string servicename);

   // helper routines.
   cResult _createLaunchScript(std::string servicename);
   cResult _removeLaunchScript(std::string servicename);

   void _ensureDirectoriesExist(std::string servicename);
   cResult _recreate(std::string servicename,bool devMode);
}


#endif

