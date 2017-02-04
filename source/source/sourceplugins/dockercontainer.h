#ifndef __DOCKERCONTAINER_H
#define __DOCKERCONTAINER_H

#include "sourcecopy.h"

namespace sourcecopy
{

   class dockercontainer : public sourceplugin
   {
   public:
      dockercontainer() {}

      cResult install(std::string imagename, const servicePaths & sp);
      bool pluginmatch(std::string imagename);
      cResult normaliseNames(std::string & imagename, std::string & servicename);
   };

}


#endif