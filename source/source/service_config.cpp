#include <cereal/archives/json.hpp>
#include <fstream>
#include <Poco/Environment.h>

#include "service_config.h"
#include "utils.h"
#include "service_yml.h"
#include "globallogger.h"

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

   try
   {
      cereal::JSONInputArchive archive(is);
      archive(mVariables);
   }
   catch (const cereal::Exception &)
   {
      return kRError;
   }
   return kRSuccess;
}

cResult serviceConfig::saveconfig() const
{
   std::ofstream os(mServicePath.toString());
   if (os.bad() || !os.is_open())
      return kRError;

   try
   {
      cereal::JSONOutputArchive archive(os);
      archive(mVariables);
   }
   catch (const cereal::Exception &)
   {
      return kRError;
   }
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
   mVariables.setVal("IMAGENAME", iname);
}

std::string serviceConfig::getServiceName() const
{
   return mVariables.getVal("SERVICENAME");
}

void serviceConfig::setServiceName(std::string sname)
{
   mVariables.setVal("SERVICENAME", sname);
}

cResult serviceConfig::create(const serviceyml::simplefile & y)
{
   const std::vector<serviceyml::Configuration> & ci(y.getConfigItems());

   for (const auto & i : ci)
      mVariables.setVal(i.name, i.defaultval);

   return kRSuccess;
}

void serviceConfig::setSaveVariable(std::string keyval)
{
   std::string key, val;
   size_t epos = keyval.find('=');
   if (epos == std::string::npos)
   {
      key = keyval;
      try {
         val = Poco::Environment::get(key);
      }
      catch (const Poco::Exception & e)
      {
         logmsg(kLDEBUG, e.what());
         logmsg(kLERROR, "Configuration variables must be given in the form key=value, or the value set as an environment variable");
      }
   }
   else 
   {
      if (epos == 0)
         logmsg(kLERROR, "Missing key.");
      if (epos == keyval.length() - 1)
         logmsg(kLERROR, "Missing value.");

      key = keyval.substr(0, epos);
      val = keyval.substr(epos + 1);
   }
   mVariables.setVal(key, val);
   logmsg(kLDEBUG, "Set key " + key + " to value " + val);
   saveconfig();
}
