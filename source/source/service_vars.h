#ifndef __SERVICE_VARS_H
#define __SERVICE_VARS_H

#include "variables.h"

class serviceVars : public persistvariables
{
public:
   serviceVars(std::string servicename, const std::vector<Configuration> & config);
   serviceVars(std::string servicename, std::string imagename, const std::vector<Configuration> & config);

   //serviceVars(std::string servicename);
   //initialise(const std::vector<Configuration> & config);

   std::string getImageName() const;
   std::string getServiceName() const;

private:
   void _setconfig();
};


#endif