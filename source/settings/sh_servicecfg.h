#ifndef __SH_SERVICECFG_H
#define __SH_SERVICECFG_H


   class sh_servicecfg : public settingsbash
   {
   public:
      // read ctor
      sh_servicecfg(
         const params & p,
         const sh_drunnercfg & settings,
         std::string path
         )
         :  settingsbash(p,path+"/servicecfg.sh")
      {
         std::vector<std::string> nothing;
         setSettingv("VOLUMES",nothing);
         setSettingv("EXTRACONTAINERS",nothing);

         bool readok = readSettings();
         if (!readok)
            logmsg(kLERROR,"Not a valid dService. Couldn't read "+path+"/servicecfg.sh",p);
      } // ctor

      void getVolumes(std::vector<std::string> & volumes) const
         {getSettingv("VOLUMES",volumes);}
      void getExtraContainers(std::vector<std::string> & extracontainers) const
         {getSettingv("EXTRACONTAINERS",extracontainers);}
   }; //class



#endif
