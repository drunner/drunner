#ifndef __SH_SERVICEVARS_H
#define __SH_SERVICEVARS_H

#include <string>

#include "settingsbash.h"

class sh_servicevars : public settingsbash
{
public:

   // reading ctor
   sh_servicevars() // sets defaults and reads the file if present.
      : settingsbash(false)
   {
      setString("IMAGENAME", "not set");
   }

   bool create(std::string imagename)
   {
      setString("IMAGENAME", imagename);
      return true;
   }

   std::string getImageName() const { return getString("IMAGENAME"); }
   std::string getPath(std::string servicepath) { return servicepath + "/servicevars.sh"; }

}; // sh_servicevars

#endif
