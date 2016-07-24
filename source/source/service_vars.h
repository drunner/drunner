#ifndef __SERVICE_CONFIG_H
#define __SERVICE_CONFIG_H

#include "service_paths.h"
#include "cresult.h"
#include "variables.h"


class serviceVars {
public:
   serviceVars(const servicePaths & p);
   cResult loadconfig();
   cResult saveconfig() const;

   const variables & getVariables() const { return mVariables; }
   void setVariable(std::string key, std::string val);

private:
   variables mVariables;   
   const servicePaths & mServicePaths;
};
CEREAL_CLASS_VERSION(serviceVars, 1);


#endif