#ifndef __SH_BACKUPVARIABLES_H
#define __SH_BACKUPVARIABLES_H

#include <string>
#include <vector>

#include "utils.h"
#include "service.h"
#include "settingsbash.h"
#include "drunner_compose.h"

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
   bool createFromdrunnerCompose(const drunnerCompose & drc)
   {
      tVecStr dockervols;
      drc.getDockerVolumeNames(dockervols);

      // all docker volumes that dRunner has to manage.
      setVec("DOCKERVOLS", dockervols);

      // the main image name.
      setString("IMAGENAME", drc.getImageName());

      return true;
   }

   void getDockerVolumeNames(std::vector<std::string> & s)	const { getVec("DOCKERVOLS",s); }
   std::string getImageName() const { return getString("IMAGENAME"); }

   Poco::Path getPathFromParent(Poco::Path parentpath) 
   {
      poco_assert(parentpath.isDirectory());
      return parentpath.pushDirectory("/backupvars.sh"); 
   }

protected:
   void setDefaults()
   {
      std::vector<std::string> nothing;
      setString("IMAGENAME", "not set");
      setVec("DOCKERVOLS", nothing);
   }
};

#endif
