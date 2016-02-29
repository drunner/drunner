#ifndef __SH_VARIABLES_H
#define __SH_VARIABLES_H



   std::string alphanumericfilter(std::string s)
   {
      std::string validchars="0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
      size_t pos;
      while((pos = s.find_first_not_of(validchars)) != std::string::npos)
         s.erase(pos,1);
      return s;
   }


   class sh_variables : public settingsbash
   {
   public:

      // reading ctor
      sh_variables(const params & p, std::string path)
         : settingsbash(p,path+"/variables.sh")
         {
         bool readok = readSettings();
         if (!readok)
            logmsg(kLERROR,"Broken dService. Couldn't read "+path+"/variables.sh",p);
         }

      // writing ctor
      sh_variables(
         const params & p,
         std::string path,
         const sh_servicecfg & servicecfg,
         std::string servicename,
         std::string imagename,
         std::string hostIP,
         std::string serviceTempDir
         )
         :  settingsbash(p,path+"/variables.sh")
      {
         std::vector<std::string> volumes, extracontainers,dockervols,dockeropts;
         servicecfg.getVolumes(volumes);
         servicecfg.getExtraContainers(extracontainers);

         for (uint i=0;i<volumes.size();++i)
            {
            logmsg(kLDEBUG, "VOLUME:          "+volumes[i],p);
            dockervols.push_back("drunner-"+servicename+"-"+alphanumericfilter(volumes[i]));
            logmsg(kLDEBUG, "Docker Volume:   "+dockervols[i],p);
            dockeropts.push_back("-v");
            dockeropts.push_back(dockervols[i]+":"+volumes[i]);
            }
         for (uint i=0;i<extracontainers.size();++i)
            logmsg(kLDEBUG, "EXTRACONTAINER:  "+extracontainers[i],p);

         setSettingv("VOLUMES",volumes);
         setSettingv("EXTRACONTAINERS",extracontainers);
         setSetting("SERVICENAME",servicename);
         setSetting("IMAGENAME",imagename);
         setSetting("INSTALLTIME",utils::getTime());
         setSetting("HOSTIP",hostIP);
         setSetting("SERVICETEMPDIR",serviceTempDir);
         setSettingv("DOCKERVOLS",dockervols);
         setSettingv("DOCKEROPTS",dockeropts);
         writeSettings();
      }
   }; // sh_variables

#endif
