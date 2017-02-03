#ifndef __REGISTRIES_H
#define __REGISTRIES_H

#include <string>
#include <cereal/access.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/map.hpp>

#include "service_paths.h"
#include "sourceplugins.h"
#include "variables.h"
#include "captaincopy.h"

class registrydataitem
{
public:
   registrydataitem(std::string n, eProtocol p, std::string u) : mNiceName(n), mProtocol(p), mURL(u) {}
   registrydataitem(std::string n, std::string p, std::string u) : mNiceName(n), mProtocol(ProtoParse(p)), mURL(u) {}
   std::string mNiceName;
   eProtocol mProtocol;
   std::string mURL;

   std::string protostr() const { return unParseProto(mProtocol); }
private:
   // --- serialisation --
   friend class ::cereal::access;
   template <class Archive> void save(Archive &ar, std::uint32_t const version) const { ar(mNiceName,mProtocol,mURL); }
   template <class Archive> void load(Archive &ar, std::uint32_t const version) { ar(mNiceName, mProtocol, mURL); }
   // --- serialisation --
};
CEREAL_CLASS_VERSION(registrydataitem, 1);

class registrydata
{
public:

   void setVal(const registrydataitem & val);
   bool exists(std::string nicename) const;
   const registrydataitem & getVal(std::string nicename) const;
   void delVal(std::string nicename);

   const std::vector<registrydataitem> & getAll() const;

private:
   // --- serialisation --
   friend class cereal::access;
   template <class Archive> void save(Archive &ar, std::uint32_t const version) const { ar(mItems); }
   template <class Archive> void load(Archive &ar, std::uint32_t const version) { mItems.clear(); ar(mItems); }
   // --- serialisation --

   std::vector<registrydataitem> mItems;
};
CEREAL_CLASS_VERSION(registrydata, 1);


class registries
{
public:
   registries();

   cResult addregistry(std::string nicename, std::string protocol, std::string url);
   cResult delregistry(std::string nicename);
   cResult showregistries();

   registrydataitem get(const std::string imagename) const;

private:
   cResult load();
   cResult save();

   registrydata mData;
   Poco::Path mPath;
};

#endif