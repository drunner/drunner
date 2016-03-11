#ifndef __SH_DDEV__
#define __SH_DDEV__

#include "settingsbash.h"

// in the service directory
   class sh_ddev : public settingsbash_reader
   {
   public:
      std::string buildname, devservicename;
      bool isdService;

      sh_ddev(const params & p, std::string pwd) : settingsbash_reader(pwd+"/ddev.sh")
      {
         setDefaults();
         read();

         buildname=getString("BUILDNAME");
         devservicename=getString("DEVSERVICENAME");
         isdService = readOkay();
         
         if (isdService)
         {
            logmsg(kLDEBUG, "DIRECTORY:        "+pwd,p);
            logmsg(kLDEBUG, "DDEV COMPATIBLE:  yes",p);
            logmsg(kLDEBUG, "BUILDNAME:        "+buildname, p);
            logmsg(kLDEBUG, "DEVSERVICENAME:   "+devservicename, p);
         }
      } // ctor

   protected:
      void setDefaults()
      {
         setString("BUILDNAME", "undefined");
         setBool("DSERVICE", false);
         setString("DEVSERVICENAME", "undefined");
      }
   }; //class

#endif
