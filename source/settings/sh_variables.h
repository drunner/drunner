#ifndef __SH_VARIABLES_H
#define __SH_VARIABLES_H

#include "utils.h"
#include "sh_servicecfg.h"
#include "service.h"

class sh_variables : public settingsbash_reader
{
public:

   // reading ctor
   sh_variables(std::string fullpath) // sets defaults and reads the file if present.
      : settingsbash_reader(fullpath)
   {
      setDefaults();
      read();
   }

   // creates variables.sh from servicecfg.sh
   bool createFromServiceCfg(const service & svc)
   {
      setDefaults();
      return populate(svc);
   }

   bool write() const
   {
      return writeSettings(getPath());
   }

   bool writecopy(const std::string & fullpath) const
   {
      return writeSettings(fullpath);
   }

   const std::vector<std::string> & getDockerVols() const { return getVec("DOCKERVOLS"); }
   const std::vector<std::string> & getVolumes()	const { return getVec("VOLUMES"); }
   const std::vector<std::string> & getExtraContainers()	const { return getVec("EXTRACONTAINERS"); }
   const std::string & getImageName() const { return getString("IMAGENAME"); }

protected:
   void setDefaults()
   {
      std::vector<std::string> nothing;
      setVec("VOLUMES", nothing);
      setVec("EXTRACONTAINERS", nothing);
      setString("SERVICENAME", "not set");
      setString("IMAGENAME", "not set");
      setString("SERVICETEMPDIR", "not set");
      setVec("DOCKERVOLS", nothing);
      setVec("DOCKEROPTS", nothing);

      setString("INSTALLTIME", utils::getTime());
      setString("HOSTIP", utils::getHostIP());
   }

private:

   bool populate(const service & svc)
   {
      sh_servicecfg servicecfg(svc.getPathServiceCfg());
      if (!servicecfg.readOkay())
         return false;

      std::vector<std::string> volumes, extracontainers, dockervols, dockeropts;
      volumes = servicecfg.getVolumes();
      extracontainers = servicecfg.getExtraContainers();

      for (uint i = 0; i<volumes.size(); ++i)
      {
         dockervols.push_back("drunner-" + svc.getName() + "-" + utils::alphanumericfilter(volumes[i],false));
         dockeropts.push_back("-v");
         dockeropts.push_back(dockervols[i] + ":" + volumes[i]);
      }

      setVec("VOLUMES", volumes);
      setVec("EXTRACONTAINERS", extracontainers);
      setVec("DOCKERVOLS", dockervols);
      setVec("DOCKEROPTS", dockeropts);

      setString("SERVICENAME", svc.getName());
      setString("IMAGENAME", svc.getImageName());
      setString("SERVICETEMPDIR", svc.getPathTemp());

      return true;
   }


}; // sh_variables

#endif
