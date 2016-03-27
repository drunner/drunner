#ifndef __SH_BACKUPVARIABLES_H
#define __SH_BACKUPVARIABLES_H

#include <string>
#include <vector>

#include "utils.h"
#include "service.h"
#include "settingsbash.h"
#include "drunnercompose.h"

class sh_backupvars : public settingsbash_reader
{
public:

   // reading ctor
   sh_backupvars(std::string parentpath) // sets defaults and reads the file if present.
      : settingsbash_reader(parentpath+"/backupvars.sh")
   {
      setDefaults();
      read();
   }

   // creates from drunnerCompose.
   bool createFromdrunnerCompose(const drunnerCompose & drc)
   {
      return populate(drc);
   }

   bool write() const
   {
      return writeSettings(getPath());
   }

   const std::vector<std::string> & getDockerVols()	const { return getVec("DOCKERVOLS"); }
   const std::string & getImageName() const { return getString("IMAGENAME"); }

protected:
   void setDefaults()
   {
      std::vector<std::string> nothing;
      setString("IMAGENAME", "not set");
      setVec("DOCKERVOLS", nothing);
   }

private:

   bool populate(const drunnerCompose & drc)
   {
      tVecStr dockervols;
      drc.getDockerVols(dockervols);

      // all docker volumes that dRunner has to manage.
      setVec("DOCKERVOLS", dockervols);

      // the main image name.
      setString("IMAGENAME", drc.getImageName());

      return true;
   }


};

#endif
