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
      virtual cResult install(std::string & imagename, const servicePaths & sp) = 0;
   };



   cResult install(std::string & imagename, const servicePaths & sp);

} // namespace

#endif