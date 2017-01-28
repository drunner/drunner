#ifndef __SERVICE_VARS_H
#define __SERVICE_VARS_H

#include "variables.h"

class serviceVars : public persistvariables
{
public:
   // the configuration passed in is as defined in the Lua file by the service.
   // we mix in our own variables here too, namely whether it's in dev mode, 
   // the service name and the image name.

   serviceVars(std::string servicename);

   std::string getImageName() const;
   std::string getServiceName() const;
   bool getIsDevMode() const;

   void setImageName(std::string imagename);
   void setDevMode(bool isDevMode);

private:
   void _extendconfig();
};


#endif