#ifndef __SERVICE_CONFIG_H
#define __SERVICE_CONFIG_H

#include "service_paths.h"
#include "cresult.h"
#include "service_yml.h"
#include "service_variables.h"

class serviceConfig {
public:
   serviceConfig(Poco::Path path);
   cResult create(const serviceyml::simplefile & y);
   cResult loadconfig();
   cResult saveconfig() const;

   std::string getImageName() const;
   void setImageName(std::string iname);

   std::string getServiceName() const;
   void setServiceName(std::string sname);

   const variables & getVariables() const { return mVariables; }

private:
   variables mVariables;
   Poco::Path mServicePath;
};
CEREAL_CLASS_VERSION(serviceConfig, 1);


#endif