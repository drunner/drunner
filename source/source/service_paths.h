#ifndef __SERVICE_PATHS_H
#define __SERVICE_PATHS_H

#include <string>
#include <Poco/Path.h>

class servicePaths
{
public:
   servicePaths(const std::string & servicename);

   Poco::Path getPath() const;
   Poco::Path getPathdRunner() const;
   Poco::Path getPathHostVolume() const;
   //Poco::Path getPathHostVolume_environment() const;
   Poco::Path getPathLaunchScript() const;
   Poco::Path getPathservicelua() const;
   Poco::Path getPathServiceConfig() const;

   std::string getName() const;

protected:
   const std::string mName;
};


#endif

