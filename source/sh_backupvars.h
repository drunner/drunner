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
      //tVecStr dockervols;
      //drc.getDockerVolumeNamesBackup(dockervols);

      //// all docker volumes that dRunner has to manage.
      //setVec("DOCKERVOLS", dockervols);

      //tVecStr test;
      //getVec("DOCKERVOLS", test);
      //if (test.size() != dockervols.size())
      //   fatal("Mismatch in shb :/  ");
      //for (int i = 0; i < test.size(); ++i)
      //   if (test[i] != dockervols[i])
      //      fatal("not equal: " + test[i] + " " + dockervols[i]);

      // the main image name.
      setString("IMAGENAME", imagename);

      return true;
   }

   //void getDockerVolumeNamesBackup(std::vector<std::string> & s)	const { getVec("DOCKERVOLS",s); }
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
