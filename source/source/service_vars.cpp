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
      return cError("Service vars file doesn't exist.");
   std::ifstream is(mServicePaths.getPathServiceVars().toString());
   if (is.bad())
      return cError("Service vars file couldn't be opened for reading.");

   try
   {
      cereal::JSONInputArchive archive(is);
      archive(mVariables);
   }
   catch (const cereal::Exception &e)
   {
      return cError("Failed to read service vars file: "+std::string(e.what()));
   }

   // update serviceName if needed.
   if (mVariables.getVal("SERVICENAME") != mServicePaths.getName())
   {
      logmsg(kLDEBUG, "SERVICENAME in " + mServicePaths.getPathServiceVars().toString() + " is incorrect.");
      logmsg(kLDEBUG, "(it is " + mVariables.getVal("SERVICENAME") + " but was expecting " + mServicePaths.getName() + ")");
      logmsg(kLDEBUG, "This is okay if we're restoring from a backup.");
      setVariable("SERVICENAME", mServicePaths.getName());
   }

   return kRSuccess;
}

cResult serviceVars::saveconfig() const
{
   std::ofstream os(mServicePaths.getPathServiceVars().toString());
   if (os.bad() || !os.is_open())
      return cError("Service vars file couldn't be opened for writing.");

   try
   {
      cereal::JSONOutputArchive archive(os);
      archive(mVariables);
   }
   catch (const cereal::Exception & e)
   {
      return cError("Failed to write service vars file: " + std::string(e.what()));
   }
   return kRSuccess;
}

void serviceVars::setVariable(std::string key, std::string val)
{
   mVariables.setVal(key, val);
   logmsg(kLDEBUG, "Set key " + key + " to value " + val);
}