#ifndef __SH_SERVICECFG_H
#define __SH_SERVICECFG_H

#include "service.h"

   class sh_servicecfg : public settingsbash
   {
   public:
      // read ctor
      sh_servicecfg(const service & svc)
         :  settingsbash(svc.getParams(),svc.getPathServiceCfg()), mSvc(svc)
      {
         std::vector<std::string> nothing;
         setVec("VOLUMES",nothing);
         setVec("EXTRACONTAINERS",nothing);

         bool readok = readSettings();
         if (!readok)
            logmsg(kLERROR,"Not a valid dService. Couldn't read "+getPath());
      } // ctor

      const std::vector<std::string> & getVolumes() const
         {return getVec("VOLUMES");}
      const std::vector<std::string> & getExtraContainers() const
         {return getVec("EXTRACONTAINERS");}
      const service & getService() const
         { return mSvc; }

   private:
      const service & mSvc;
   }; //class



#endif
