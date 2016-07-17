#include <Poco/String.h>

#include "service_variables.h"
#include "utils.h"
#include "globallogger.h"


variables::variables(const variables & other)
{
   const tKeyVals & otherkvs(other.getAll());
   mVariables.insert(otherkvs.begin(), otherkvs.end());
}
variables::variables(const variables & other1, const variables & other2)
{
   const tKeyVals & otherkvs1(other1.getAll());
   const tKeyVals & otherkvs2(other2.getAll());
   mVariables.insert(otherkvs1.begin(), otherkvs1.end());
   mVariables.insert(otherkvs2.begin(), otherkvs2.end());
}

bool variables::hasKey(std::string key) const
{
   for (const auto & x : mVariables)
      if (utils::stringisame(key, x.first))
         return true;
   return false;
}

std::string variables::getVal(std::string key) const
{
   for (const auto & x : mVariables)
      if (utils::stringisame(key, x.first))
         return x.second;
   return "";
}

void variables::setVal(std::string key, std::string val) 
{
   mVariables[key] = val;
}

std::string variables::substitute(std::string s) const
{
   std::string os(s);
   for (auto it = mVariables.begin(); it != mVariables.end(); ++it)
   {
      Poco::replaceInPlace(os, "$" + it->first, it->second);
      Poco::replaceInPlace(os, "${" + it->first+"}", it->second);
   }
   return os;
}

const Poco::Process::Env & variables::getEnv() const
{
   return mVariables;
}
