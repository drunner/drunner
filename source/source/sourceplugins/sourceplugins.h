#ifndef __SOURCE_PLUGINS_H
#define __SOURCE_PLUGINS_H

#include "utils.h"
#include "service_paths.h"

namespace sourceplugins
{
   class sourceplugin
   {
   public:
      // returns kRNoChange if nothing installed (e.g. imagename not of the correct type).
      virtual bool pluginmatch(std::string imagename) = 0;

      virtual cResult install(std::string imagename, const servicePaths & sp) = 0;
      virtual cResult normaliseNames(std::string & imagename, std::string & servicename) = 0;
   };

   cResult install(std::string imagename, const servicePaths & sp);
   cResult normaliseNames(std::string & imagename, std::string & servicename);
   cResult registrycommand();
} // namespace

#endif