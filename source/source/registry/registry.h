#ifndef __REGISTRY_H
#define __REGISTRY_H

#include <string>
#include <cereal/access.hpp>
#include <cereal/types/string.hpp>

#include "cresult.h"


class registrydefinition
{
public:
   registrydefinition() : mSet(false) {}
   registrydefinition(std::string n, std::string u) : mNiceName(n), mURL(u), mSet(true) {}
   registrydefinition(std::string n, std::string p, std::string u) : mNiceName(n), mURL(u), mSet(true) {}

   std::string mNiceName;
   std::string mURL;
   bool mSet;

private:
   // --- serialisation --
   friend class ::cereal::access;
   template <class Archive> void save(Archive &ar, std::uint32_t const version) const { ar(mNiceName, mURL); }
   template <class Archive> void load(Archive &ar, std::uint32_t const version) { ar(mNiceName, mURL); }
   // --- serialisation --
};
CEREAL_CLASS_VERSION(registrydefinition, 1);


namespace sourcecopy
{

   class registryitem
   {
   public:
      std::string nicename;
      std::string url;
      std::string description;
   };

   class registry
   {
   public:
      registry(registrydefinition r);
      
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