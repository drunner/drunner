#ifndef __SH_SERVICECFG_H
#define __SH_SERVICECFG_H

#include "service.h"

   class sh_servicecfg : public settingsbash_reader
   {
   public:
      // read ctor
      sh_servicecfg(std::string fullpath)
         :  settingsbash_reader(fullpath)
      {
         setDefaults();
         read();
      } // ctor

      void setDefaults()
      {
         std::vector<std::string> nothing;
         setVec("VOLUMES", nothing);
         setVec("EXTRACONTAINERS", nothing);
      }

      const std::vector<std::string> & getVolumes() const
         {return getVec("VOLUMES");}
      const std::vector<std::string> & getExtraContainers() const
         {return getVec("EXTRACONTAINERS");}

   }; //class



#endif
