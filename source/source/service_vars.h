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

   std::string getImageName() const;
   void setImageName(std::string iname);

   std::string getServiceName() const;
   void setServiceName(std::string sname);

   const variables & getVariables() const { return mVariables; }
   cResult setSaveVariable(std::string key, std::string val);

   cResult setVariable(std::string key, std::string val);

private:
   variables mVariables;
   
   const servicePaths & mServicePaths;
};
CEREAL_CLASS_VERSION(serviceVars, 1);


#endif