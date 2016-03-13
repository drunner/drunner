#ifndef __SH_SERVICECFG_H
#define __SH_SERVICECFG_H

#include "service.h"
#include "utils.h"

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
         setVec("HOOKS", nothing);
         setString("VERSION", "1");
      }

      const std::vector<std::string> & getVolumes() const
      {
         return getVec("VOLUMES");
      }

      const std::vector<std::string> & getExtraContainers() const
      {
         return getVec("EXTRACONTAINERS");
      }

      bool hasHook(std::string hook)
      {
         for (const auto & entry : getVec("HOOKS"))
            if (utils::stringisame(entry, hook))
               return true;
         return false;
      }

      int getVersion()
      {
         return atoi(getString("VERSION").c_str());
      }


   }; //class



#endif
