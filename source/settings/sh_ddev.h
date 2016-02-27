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
         setSetting("BUILDNAME","undefined");
         setSettingb("DSERVICE",false);
         setSetting("DEVSERVICENAME","undefined");

         isdService=readSettings();
         buildname=getSetting("BUILDNAME");
         devservicename=getSetting("DEVSERVICENAME");
         if (isdService)
         {
            logmsg(kLDEBUG, "DIRECTORY:        "+pwd,p);
            logmsg(kLDEBUG, "DDEV COMPATIBLE:  yes",p);
            logmsg(kLDEBUG, "BUILDNAME:        "+buildname,p);
            logmsg(kLDEBUG, "DEVSERVICENAME:   "+devservicename,p);
         }
      } // ctor
   }; //class

#endif
