#include <fstream>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/archives/json.hpp>

#include "service_config.h"


template <class Archive>
void keyval::save(Archive &ar, std::uint32_t const version) const
{
   ar(key, value);
}

template <class Archive>
void keyval::load(Archive &ar, std::uint32_t const version) const
{
   ar(key, value);
}


serviceConfig::serviceConfig(const servicePaths & svp) :
   mServicePath(svp.getPathServiceConfig())
{
}

cResult serviceConfig::create(const serviceyml & y)
{
   // hairdressers
   // squash courts
}

cResult serviceConfig::load()
{
   std::ifstream is(mServicePath.toString());
   if (is.bad())
      return kRError;
   cereal::JSONInputArchive archive(is);
   archive(*this);
   return kRSuccess;
}

cResult serviceConfig::save()
{
   std::ofstream os(mServicePath.toString());
   if (os.bad())
      return kRError;
   cereal::JSONOutputArchive archive(os);
   archive(*this);
   return kRSuccess;
}

std::string serviceConfig::getVal(std::string key) const
{

}

void serviceConfig::setVal(std::string key, std::string val)
{

}

const std::vector<const keyval> & serviceConfig::getData() const
{

}

template <class Archive>
void serviceConfig::save(Archive &ar, std::uint32_t const version) const
{
   ar(mData);
}
template <class Archive>
void serviceConfig::load(Archive &ar, std::uint32_t const version) const
{
   ar(mData);
}