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

std::string serviceConfig::getImageName() const
{
   std::string iname = mVariables.getVal("IMAGENAME");
   poco_assert(iname.length() > 0);
   return iname;
}

void serviceConfig::setImageName(std::string iname)
{
   mVariables.setVal(keyval("IMAGENAME", iname));
}

std::string serviceConfig::getServiceName() const
{
   return mVariables.getVal("SERVICENAME");
}

void serviceConfig::setServiceName(std::string sname)
{
   mVariables.setVal(keyval("SERVICENAME", sname));
}

cResult serviceConfig::create(const serviceyml::simplefile & y)
{
   const std::vector<serviceyml::Configuration> & ci(y.getConfigItems());

   for (const auto & i : ci)
      mVariables.setVal(keyval(i.name, i.defaultval));

   return kRSuccess;
}
