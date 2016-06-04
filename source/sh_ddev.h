#ifndef __SH_DDEV__
#define __SH_DDEV__

#include "settingsbash.h"

// in the service directory
   class sh_ddev : public settingsbash
   {
   public:
      std::string buildname, devservicename;

      sh_ddev() : settingsbash(false)
      {
         setDefaults();
      }

      bool readSettings(std::string settingspath)
      {
         bool isdService = settingsbash::readSettings(settingspath);

         buildname = getString("BUILDNAME");
         devservicename = getString("DEVSERVICENAME");

         //if (isdService)
         //{
         //   logmsg(kLDEBUG, "DIRECTORY:        " + pwd);
         //   logmsg(kLDEBUG, "DDEV COMPATIBLE:  yes");
         //   logmsg(kLDEBUG, "BUILDNAME:        " + buildname);
         //   logmsg(kLDEBUG, "DEVSERVICENAME:   " + devservicename);
         //}

         return isdService;
      }

      std::string getPathFromParent(std::string parent) 
      { 
         return parent + "/ddev.sh"; 
      }

   protected:
      void setDefaults()
      {
         buildname = "undefined";
         devservicename = "undefined";

         setString("BUILDNAME", "undefined");
         setBool("DSERVICE", false);
         setString("DEVSERVICENAME", "undefined");
      }
   }; //class

#endif
