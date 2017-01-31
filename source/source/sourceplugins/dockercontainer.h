#ifndef __DOCKERCONTAINER_H
#define __DOCKERCONTAINER_H

#include "sourceplugins.h"

namespace sourceplugins
{

   class dockercontainer : public sourceplugin
   {
   public:
      dockercontainer() {}
      cResult install(std::string & imagename, const servicePaths & sp);
   };

}


#endif