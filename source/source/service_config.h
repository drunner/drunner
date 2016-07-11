#ifndef __SERVICE_CONFIG_H
#define __SERVICE_CONFIG_H

#include <cereal/access.hpp>

#include "service_paths.h"
#include "cresult.h"
#include "service_yml.h"

class keyval {
public:
   keyval() {}

   std::string key;
   std::string value;

// --- serialisation --
private:
   friend class cereal::access;
   template <class Archive>
   void save(Archive &ar, std::uint32_t const version) const;
   template <class Archive>
   void load(Archive &ar, std::uint32_t const version) const;
// --- serialisation --
};
CEREAL_CLASS_VERSION(keyval, 1);

class serviceConfig {
public:
   serviceConfig(const servicePaths & svp);
   cResult create(const serviceyml & y);
   cResult load();
   cResult save();

   std::string getVal(std::string key) const;
   void setVal(std::string key, std::string val);
   const std::vector<const keyval> & getData() const;

private:
   std::vector<keyval> mData;
   Poco::Path mServicePath;

// --- serialisation --
private:
   friend class cereal::access;
   template <class Archive>
   void save(Archive &ar, std::uint32_t const version) const;
   template <class Archive>
   void load(Archive &ar, std::uint32_t const version) const;
// --- serialisation --
};
CEREAL_CLASS_VERSION(serviceConfig, 1);

#endif