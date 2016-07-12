#ifndef __SERVICE_CONFIG_H
#define __SERVICE_CONFIG_H

#include "service_paths.h"
#include "cresult.h"
#include "service_yml.h"
#include "service_variables.h"

class serviceConfig : public variables {
public:
   serviceConfig(const servicePaths & svp);
   cResult create(const serviceyml::file & y);
   cResult loadconfig();
   cResult saveconfig();

   variables & getVariables() { return mVariables; }

private:
   variables mVariables;
   Poco::Path mServicePath;
};
CEREAL_CLASS_VERSION(serviceConfig, 1);


#endif