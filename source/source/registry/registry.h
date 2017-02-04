#ifndef __REGISTRY_H
#define __REGISTRY_H

#include <string>
#include <cereal/access.hpp>
#include <cereal/types/string.hpp>

#include "cresult.h"
#include "captaincopy.h"


class registrydefinition
{
public:
   registrydefinition() : mSet(false) {}
   registrydefinition(std::string n, eProtocol p, std::string u) : mNiceName(n), mProtocol(p), mURL(u), mSet(true) {}
   registrydefinition(std::string n, std::string p, std::string u) : mNiceName(n), mProtocol(ProtoParse(p)), mURL(u), mSet(true) {}

   std::string mNiceName;
   eProtocol mProtocol;
   std::string mURL;
   bool mSet;

   std::string protostr() const { return unParseProto(mProtocol); }
private:
   // --- serialisation --
   friend class ::cereal::access;
   template <class Archive> void save(Archive &ar, std::uint32_t const version) const { ar(mNiceName, mProtocol, mURL); }
   template <class Archive> void load(Archive &ar, std::uint32_t const version) { ar(mNiceName, mProtocol, mURL); }
   // --- serialisation --
};
CEREAL_CLASS_VERSION(registrydefinition, 1);


namespace sourcecopy
{

   class registryitem
   {
   public:
      std::string nicename;
      eProtocol protocol;
      std::string url;
      std::string description;

      SourceInfo getSourceInfo(std::string tag) const;
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