#ifndef __SERVICE_PATHS_H
#define __SERVICE_PATHS_H

#include <string>
#include <Poco/Path.h>

class servicePaths
{
public:
   servicePaths(const std::string & servicename);

   Poco::Path getPathdService() const;
   Poco::Path getPathHostVolume() const;
   Poco::Path getPathLaunchScript() const;
   Poco::Path getPathServiceVars() const;
   std::string getName() const;

   // provided by the dService.
   Poco::Path getPathServiceLua() const;
   Poco::Path getPathVariablesLua() const;


protected:
   const std::string mName;
};


#endif

