#ifndef __REGISTRIES_H
#define __REGISTRIES_H

#include <string>
#include <cereal/access.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/map.hpp>

#include "service_paths.h"
#include "sourcecopy.h"
#include "variables.h"
#include "registry.h"

class registrydefinitions
{
public:

   void setVal(const registrydefinition & val);
   bool exists(std::string nicename) const;
   cResult getVal(std::string nicename, registrydefinition & val) const;
   void delVal(std::string nicename);

   const std::vector<registrydefinition> & getAll() const;

private:
   // --- serialisation --
   friend class cereal::access;
   template <class Archive> void save(Archive &ar, std::uint32_t const version) const { ar(mItems); }
   template <class Archive> void load(Archive &ar, std::uint32_t const version) { mItems.clear(); ar(mItems); }
   // --- serialisation --

   std::vector<registrydefinition> mItems;
};
CEREAL_CLASS_VERSION(registrydefinitions, 1);


class registries
{
public:
   registries();

   cResult addregistry(std::string nicename, std::string protocol, std::string url);
   cResult delregistry(std::string nicename);
   cResult showregistries();

   cResult showAllRegistereddServices();

   registrydefinition get(std::string & registry, std::string & dService) const;

   static cResult splitImageName(std::string imagename, std::string & registry, std::string & dService, std::string & tag);

private:
   cResult load();
   cResult save();

   registrydefinitions mData;
   Poco::Path mPath;
};

#endif