#include <cereal/archives/json.hpp>

#include <fstream>
#include "service_config.h"
#include "utils.h"


serviceConfig::serviceConfig(const servicePaths & svp) :
   mServicePath(svp.getPathServiceConfig())
{
}

cResult create(const serviceyml::file & y)
{
   return kRNotImplemented;
}

cResult serviceConfig::loadconfig()
{
   std::ifstream is(mServicePath.toString());
   if (is.bad())
      return kRError;
   cereal::JSONInputArchive archive(is);
   archive(mVariables);
   return kRSuccess;
}

cResult serviceConfig::saveconfig()
{
   std::ofstream os(mServicePath.toString());
   if (os.bad())
      return kRError;
   cereal::JSONOutputArchive archive(os);
   archive(mVariables);
   return kRSuccess;
}
