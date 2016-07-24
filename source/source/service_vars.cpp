#include <cereal/archives/json.hpp>
#include <fstream>
#include <Poco/Environment.h>

#include "service_vars.h"
#include "utils.h"
#include "service_lua.h"
#include "globallogger.h"

serviceVars::serviceVars(const servicePaths & p) :
   mServicePaths(p)
{
}

cResult serviceVars::loadconfig()
{
   if (!utils::fileexists(mServicePaths.getPathServiceVars()))
      return kRError;
   std::ifstream is(mServicePaths.getPathServiceVars().toString());
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

cResult serviceVars::saveconfig() const
{
   std::ofstream os(mServicePaths.getPathServiceVars().toString());
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

std::string serviceVars::getImageName() const
{
   std::string iname = mVariables.getVal("IMAGENAME");
   poco_assert(iname.length() > 0);
   return iname;
}

void serviceVars::setImageName(std::string iname)
{
   mVariables.setVal("IMAGENAME", iname);
}

std::string serviceVars::getServiceName() const
{
   return mVariables.getVal("SERVICENAME");
}

void serviceVars::setServiceName(std::string sname)
{
   mVariables.setVal("SERVICENAME", sname);
}

//cResult serviceVars::create(const servicelua::file & y)
//{
//   const std::vector<servicelua::Configuration> & ci(y.getConfigItems());
//
//   for (const auto & i : ci)
//      mVariables.setVal(i.name, i.defaultval);
//
//   return kRSuccess;
//}

cResult serviceVars::setSaveVariable(std::string key, std::string val)
{
   setVariable(key, val);
   return saveconfig();
}

cResult serviceVars::setVariable(std::string key, std::string val)
{
   mVariables.setVal(key, val);
   logmsg(kLDEBUG, "Set key " + key + " to value " + val);
}