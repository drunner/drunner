#include <cereal/archives/json.hpp>

#include <fstream>
#include "service_config.h"
#include "utils.h"
#include "service_yml.h"

serviceConfig::serviceConfig(Poco::Path path) :
   mServicePath(path)
{
}

cResult create(const serviceyml::file & y)
{
   return kRNotImplemented;
}

cResult serviceConfig::loadconfig()
{
   if (!utils::fileexists(mServicePath))
      return kRError;
   std::ifstream is(mServicePath.toString());
   if (is.bad())
      return kRError;
   cereal::JSONInputArchive archive(is);
   archive(mVariables);
   return kRSuccess;
}

cResult serviceConfig::saveconfig() const
{
   std::ofstream os(mServicePath.toString());
   if (os.bad() || !os.is_open())
      return kRError;
   cereal::JSONOutputArchive archive(os);
   archive(mVariables);
   return kRSuccess;
}

cResult serviceConfig::create(const serviceyml::simplefile & y)
{
   const std::vector<serviceyml::configitem> & ci(y.getConfigItems());

   for (const auto & i : ci)
      mVariables.setVal(keyval(i.name(), i.defaultvalue()));

   return kRSuccess;
}
