#ifndef __REGISTRY_H
#define __REGISTRY_H

#include <string>

#include "cresult.h"
#include "captaincopy.h"

namespace sourceplugins
{

   class registryitem
   {
   public:
      std::string nicename;
      eProtocol protocol;
      std::string url;
      std::string description;
   };

   class registry
   {
   public:
      registry(std::string fullurl);
      
      cResult get(
         const std::string nicename,
         registryitem & item
      );

      const std::vector<registryitem> & getAll() { return mRegistryItems; }

   private:
      cResult loadline(const std::string line, registryitem & ri);
      std::vector<registryitem> mRegistryItems;
   };
}


#endif