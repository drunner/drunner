#ifndef __SH_DDEV__
#define __SH_DDEV__

#include "settingsbash.h"

// in the service directory
   class sh_ddev : public settingsbash
   {
   public:
      std::string buildname, devservicename;
      bool isdService;

      sh_ddev(const params & p, std::string pwd) : settingsbash(p,pwd+"/ddev.sh")
      {
         setString("BUILDNAME","undefined");
         setBool("DSERVICE",false);
         setString("DEVSERVICENAME","undefined");

         isdService=readSettings();
         buildname=getString("BUILDNAME");
         devservicename=getString("DEVSERVICENAME");
         if (isdService)
         {
            logmsg(kLDEBUG, "DIRECTORY:        "+pwd);
            logmsg(kLDEBUG, "DDEV COMPATIBLE:  yes");
            logmsg(kLDEBUG, "BUILDNAME:        "+buildname);
            logmsg(kLDEBUG, "DEVSERVICENAME:   "+devservicename);
         }
      } // ctor
   }; //class

#endif
