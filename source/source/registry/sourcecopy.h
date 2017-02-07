#ifndef __SOURCE_PLUGINS_H
#define __SOURCE_PLUGINS_H

#include "utils.h"
#include "service_paths.h"

namespace sourcecopy
{
   cResult install(std::string imagename, const servicePaths & sp);
   cResult normaliseNames(std::string & imagename, std::string & servicename);
   cResult registrycommand();

   cResult gitcopy(std::string repoURL, std::string tag, 
      std::vector<std::string> subfoldersearch, Poco::path dest);

} // namespace

#endif