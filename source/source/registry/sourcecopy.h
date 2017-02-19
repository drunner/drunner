#ifndef __SOURCE_PLUGINS_H
#define __SOURCE_PLUGINS_H

#include "utils.h"
#include "service_paths.h"

namespace sourcecopy
{
   cResult install(std::string imagename, const servicePaths & sp);
   cResult normaliseNames(std::string & imagename, std::string & servicename);
   cResult registrycommand();

   // modifies the given path to the folder containing the best 
   // service.lua to use (searching within the given path).
   // Current search order is:
   //  1) drunnerNICEVER (e.g. drunner10) subfolder of the given folder
   //  2) drunner subfolder of the given folder
   //  3) the given folder
   cResult getServiceLuaParent(Poco::Path & p); 

} // namespace

#endif