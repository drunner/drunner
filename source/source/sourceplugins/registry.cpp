#include "Poco/String.h"

#include "registry.h"

cResult sourceplugins::registry::get(const std::string nicename, std::string & protocolURL, std::string & description)
{
   std::vector<std::string> nicenames, protocolURLs, descriptions;

   cResult r = getAll(nicenames, protocolURLs, descriptions);
   if (!r.success())
      return r;

   for (unsigned int i=0; i<nicenames.size(); ++i)
      if (Poco::icompare(nicenames[i], nicename) == 0)
      {
         protocolURL = protocolURLs[i];
         description = descriptions[i];
         return kRSuccess;
      }

   return cError(nicename + " does not exist in registry.");
}

cResult sourceplugins::registry::getAll(std::vector<std::string>& nicenames, std::vector<std::string>& protocolURLs, std::vector<std::string>& descriptions)
{
   // download mFullURL, parse and return contents.

   return cResult();
}
