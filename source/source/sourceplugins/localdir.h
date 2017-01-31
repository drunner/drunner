#ifndef __LOCALDIR_H
#define __LOCALDIR_H

#include "sourceplugins.h"

namespace sourceplugins
{

   class localdir : public sourceplugin
   {
   public:
      localdir() {}
      cResult install(std::string & imagename, const servicePaths & sp);
      static cResult copydServiceFiles(Poco::Path pfrom, Poco::Path pto);
   };

}


#endif