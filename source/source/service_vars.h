#ifndef __SERVICE_VARS_H
#define __SERVICE_VARS_H

#include "variables.h"

class serviceVars : public persistvariables
{
public:
   serviceVars(std::string servicename, const std::vector<Configuration> & config);

   std::string getServiceName() const;
   bool getIsDevMode() const;

   void setDevMode(bool isDevMode);

private:
   void _setconfig();
};


#endif