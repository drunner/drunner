#ifndef __SERVICE_DSERVICEDEFINSTALL_H
#define __SERVICE_DSERVICEDEFINSTALL_H

#include <string>
#include "service_paths.h"

namespace sddi
{
   class registries
   {
   public:
      registries();
      cResult addregistry(std::string nicename, std::string giturl);
      cResult delregistry(std::string nicename);
      cResult showregistries();
      cResult copy(std::string nicename, std::string repo, std::string tag, Poco::Path dest) const;
   };


   cResult copy_from_container(std::string imagename, const servicePaths & sp);
   cResult copy_from_github(std::string & imagename, const servicePaths & sp);
}


#endif