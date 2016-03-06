#ifndef __SH_SERVICECFG_H
#define __SH_SERVICECFG_H


   class sh_servicecfg : public settingsbash
   {
   public:
      // read ctor
      sh_servicecfg(
         const params & p,
         std::string path
         )
         :  settingsbash(p,path+"/servicecfg.sh")
      {
         std::vector<std::string> nothing;
         setSetting(sbelement("VOLUMES",nothing));
         setSetting(sbelement("EXTRACONTAINERS",nothing));

         bool readok = readSettings();
         if (!readok)
            logmsg(kLERROR,"Not a valid dService. Couldn't read "+path+"/servicecfg.sh",p);
      } // ctor

      void getVolumes(std::vector<std::string> & volumes) const
         {getVec("VOLUMES",volumes);}
      void getExtraContainers(std::vector<std::string> & extracontainers) const
         {getVec("EXTRACONTAINERS",extracontainers);}
   }; //class



#endif
