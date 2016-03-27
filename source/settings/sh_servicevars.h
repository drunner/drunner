#ifndef __SH_SERVICEVARS_H
#define __SH_SERVICEVARS_H

#include <string>

#include "settingsbash.h"

class sh_servicevars : public settingsbash_reader
{
public:

   // reading ctor
   sh_servicevars(std::string servicepath) // sets defaults and reads the file if present.
      : settingsbash_reader(servicepath + "/servicevars.sh")
   {
      setDefaults();
      read();
   }

   // creates variables.sh from servicecfg.sh
   bool create(std::string imagename)
   {
      setString("IMAGENAME", imagename);
      return true;
   }

   bool write() const
   {
      return writeSettings(getPath());
   }

   const std::string & getImageName() const { return getString("IMAGENAME"); }

protected:
   void setDefaults()
   {
      setString("IMAGENAME", "not set");
   }

}; // sh_servicevars

#endif
