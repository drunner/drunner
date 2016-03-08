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
         setSetting(sb_vec("VOLUMES"));
         setSetting(sb_vec("EXTRACONTAINERS"));

         bool readok = readSettings();
         if (!readok)
            logmsg(kLERROR,"Not a valid dService. Couldn't read "+path+"/servicecfg.sh",p);
      } // ctor

      const std::vector<std::string> & getVolumes() const
         {return getVec("VOLUMES");}
      const std::vector<std::string> & getExtraContainers() const
         {return getVec("EXTRACONTAINERS");}
   }; //class



#endif
