#include <cereal/archives/json.hpp>
#include <fstream>
#include <Poco/Environment.h>

#include "service_vars.h"
#include "utils.h"
#include "service_lua.h"
#include "globallogger.h"

serviceVars::serviceVars(std::string serviceName) :
   mServicePaths(serviceName)
{
   setVariable("SERVICENAME", serviceName);
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

   // update serviceName if needed.
   if (mVariables.getVal("SERVICENAME") != mServicePaths.getName())
   {
      logmsg(kLWARN, "SERVICENAME in " + mServicePaths.getPathServiceVars().toString() + " is incorrect.");
      logmsg(kLWARN, "(it is " + mVariables.getVal("SERVICENAME") + " but was expecting " + mServicePaths.getName() + ")");
      setVariable("SERVICENAME", mServicePaths.getName());
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

void serviceVars::setVariable(std::string key, std::string val)
{
   mVariables.setVal(key, val);
   logmsg(kLDEBUG, "Set key " + key + " to value " + val);
}