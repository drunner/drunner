#ifndef __SH_BACKUPVARIABLES_H
#define __SH_BACKUPVARIABLES_H

#include <string>
#include <vector>

#include "utils.h"
#include "service.h"
#include "settingsbash.h"
#include "drunnercompose.h"
#include "globallogger.h"

class sh_backupvars : public settingsbash
{
public:

   // reading ctor
   sh_backupvars() // sets defaults
      : settingsbash(false)
   {
      setDefaults();
   }

   // creates from drunnerCompose.
   bool create(std::string imagename)
   {
      // the main image name.
      setString("IMAGENAME", imagename);

      return true;
   }

   std::string getImageName() const { return getString("IMAGENAME"); }
   std::string getPathFromParent(std::string parentpath) { return parentpath + "/backupvars.sh"; }

protected:
   void setDefaults()
   {
      std::vector<std::string> nothing;
      setString("IMAGENAME", "not set");
   }
};

#endif
