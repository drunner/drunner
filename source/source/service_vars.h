#ifndef __SERVICE_CONFIG_H
#define __SERVICE_CONFIG_H

#include "cresult.h"
#include "variables.h"
#include "service_paths.h"

class serviceVars {
public:
   serviceVars(std::string serviceName);
   cResult loadconfig();
   cResult saveconfig() const;

   const variables & getVariables() const { return mVariables; }
   void setVariable(std::string key, std::string val);

private:
   variables mVariables;   
   const servicePaths mServicePaths;
};
CEREAL_CLASS_VERSION(serviceVars, 1);


#endif