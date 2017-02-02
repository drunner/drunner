#ifndef __REGISTRY_H
#define __REGISTRY_H

#include <string>

#include "cresult.h"

namespace sourceplugins
{
   class registry
   {
   public:
      registry(std::string fullurl) : mFullURL(fullurl) {}

      cResult get(
         const std::string nicename,
         std::string & protocolURL,
         std::string & description
      );

      cResult getAll(
         std::vector<std::string> & nicenames,
         std::vector<std::string> & protocolURLs,
         std::vector<std::string> & descriptions
      );

   private:
      std::string mFullURL;
   };
}


#endif