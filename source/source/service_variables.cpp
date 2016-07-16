#include <Poco/String.h>

#include "service_variables.h"
#include "utils.h"
#include "globallogger.h"


variables::variables(const variables & other)
{
   const std::vector<keyval> & otherkvs(other.getAll());
   mVariables.insert(mVariables.end(), otherkvs.begin(), otherkvs.end());
}
variables::variables(const variables & other1, const variables & other2)
{
   const std::vector<keyval> & otherkvs1(other1.getAll());
   const std::vector<keyval> & otherkvs2(other2.getAll());
   mVariables.insert(mVariables.end(), otherkvs1.begin(), otherkvs1.end());
   mVariables.insert(mVariables.end(), otherkvs2.begin(), otherkvs2.end());
}

bool variables::hasKey(std::string key) const
{
   for (auto x : mVariables)
      if (utils::stringisame(key, x.key))
         return true;
   return false;
}

std::string variables::getVal(std::string key) const
{
   for (auto x : mVariables)
      if (utils::stringisame(key, x.key))
         return x.value;
   return "";
}

void variables::setVal(keyval kv)
{
   for (unsigned int i=0;i<mVariables.size();++i)
      if (utils::stringisame(kv.key, mVariables[i].key))
      {
         mVariables.erase(mVariables.begin() + i);
         --i;
      }

   mVariables.push_back(kv);
}

std::string variables::substitute(std::string s) const
{
   std::string os(s);
   for (auto it = mVariables.begin(); it != mVariables.end(); ++it)
   {
      Poco::replaceInPlace(os, "$" + it->key, it->value);
      Poco::replaceInPlace(os, "${" + it->key+"}", it->value);
   }
   return os;
}
