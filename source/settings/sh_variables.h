#ifndef __SH_VARIABLES_H
#define __SH_VARIABLES_H

#include "utils.h"
#include "sh_servicecfg.h"
#include "service.h"

class sh_variables : public settingsbash
{
public:

   // reading ctor
   sh_variables(const service & svc)
      : settingsbash(svc.getParams(),svc.getPathVariables())
   {
      load();
   }

   // reading ctor (from other location)
   sh_variables(const params & p, const std::string & path)
      : settingsbash(p, path)
   {
      load();
   }

   // creating ctor - creates variables.sh from sh_servicecfg
   sh_variables(std::string imagename, const sh_servicecfg & sc)
      :  settingsbash(sc.getService().getParams(), sc.getService().getPathVariables())
   {
      populate(imagename, sc);
      writeSettings();
   }

   const std::vector<std::string> & getDockerVols() const { return getVec("DOCKERVOLS"); }
   const std::vector<std::string> & getVolumes()	const { return getVec("VOLUMES"); }
   const std::vector<std::string> & getExtraContainers()	const { return getVec("EXTRACONTAINERS"); }
   const std::string & getImageName() const { return getString("IMAGENAME"); }

private:

   void load()
   {
      bool readok = readSettings();
      if (!readok)
         logmsg(kLERROR, "Broken dService. Couldn't read " + getPath());
   }

   std::string alphanumericfilter(std::string s) const
   {
      std::string validchars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
      size_t pos;
      while ((pos = s.find_first_not_of(validchars)) != std::string::npos)
         s.erase(pos, 1);
      return s;
   }

   void populate(std::string imagename, const sh_servicecfg & servicecfg)
   {
      const service & svc(servicecfg.getService());

      std::vector<std::string> volumes, extracontainers, dockervols, dockeropts;
      volumes = servicecfg.getVolumes();
      extracontainers = servicecfg.getExtraContainers();

      for (uint i = 0; i<volumes.size(); ++i)
      {
         logmsg(kLDEBUG, "VOLUME:          " + volumes[i]);
         dockervols.push_back("drunner-" + svc.getName() + "-" + alphanumericfilter(volumes[i]));
         logmsg(kLDEBUG, "Docker Volume:   " + dockervols[i]);
         dockeropts.push_back("-v");
         dockeropts.push_back(dockervols[i] + ":" + volumes[i]);
      }
      for (uint i = 0; i<extracontainers.size(); ++i)
         logmsg(kLDEBUG, "EXTRACONTAINER:  " + extracontainers[i]);

      setVec("VOLUMES", volumes);
      setVec("EXTRACONTAINERS", extracontainers);
      setString("SERVICENAME", svc.getName());
      setString("IMAGENAME", imagename);
      setString("INSTALLTIME", utils::getTime());
      setString("HOSTIP", utils::getHostIP(svc.getParams()));
      setString("SERVICETEMPDIR", svc.getPathTemp());
      setVec("DOCKERVOLS", dockervols);
      setVec("DOCKEROPTS", dockeropts);
   }


}; // sh_variables

#endif
