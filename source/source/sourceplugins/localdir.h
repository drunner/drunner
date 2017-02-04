#ifndef __LOCALDIR_H
#define __LOCALDIR_H

#include "sourcecopy.h"

namespace sourcecopy
{

   class localdir : public sourceplugin
   {
   public:
      localdir() {}
      cResult install(std::string imagename, const servicePaths & sp);
      bool pluginmatch(std::string imagename);
      cResult normaliseNames(std::string & imagename, std::string & servicename);

      static cResult copydServiceFiles(Poco::Path pfrom, Poco::Path pto);
   };

}


#endif