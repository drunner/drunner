#include <sstream>
#include <fstream>

#include "drunnercompose.h"
#include "utils.h"
#include "logmsg.h"
#include "service.h"


// ---------------------------------------------------------------------------------------------------
//

class sh_variables : public settingsbash_reader
{
public:

   // creating ctor
   sh_variables(const drunnerCompose & drc) // sets defaults and reads the file if present.
      : settingsbash_reader(drc.getService().getPathVariables())
   {
      setDefaults();
      populate(drc);
      writeSettings(drc.getService().getPathVariables());
   }

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

   bool populate(const drunnerCompose & drc)
   {
      std::vector<std::string> volumes, extracontainers, dockervols, dockeropts;

      for (const auto & entry : drc.getServicesInfo())
      {
         extracontainers.push_back(entry.mImageName);

         for (const auto & vol : entry.mVolumes)
         {
            volumes.push_back(vol.mMountPath);
            dockervols.push_back(vol.mDockerVolumeName);
            dockeropts.push_back("-v");
            dockeropts.push_back(vol.mDockerVolumeName + ":" + vol.mMountPath);
         }
      }

      setVec("VOLUMES", volumes);
      setVec("EXTRACONTAINERS", extracontainers);
      setVec("DOCKERVOLS", dockervols);
      setVec("DOCKEROPTS", dockeropts);

      setString("SERVICENAME", drc.getService().getName());
      setString("IMAGENAME", drc.getService().getImageName());
      setString("SERVICETEMPDIR", drc.getService().getPathTemp());

      return true;
   }
}; // sh_variables



// ---------------------------------------------------------------------------------------------------
//

class sh_legacy_servicecfg : public settingsbash_reader
{
public:
   // read ctor
   sh_legacy_servicecfg(std::string fullpath)
      : settingsbash_reader(fullpath)
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
   {
      return getVec("VOLUMES");
   }

   const std::vector<std::string> & getExtraContainers() const
   {
      return getVec("EXTRACONTAINERS");
   }
};

// ---------------------------------------------------------------------------------------------------
//


void InstallDockerCompose(const params & p)
{
   std::string url("https://github.com/docker/compose/releases/download/1.6.2/docker-compose-Linux-x86_64");
   std::string trgt(utils::get_usersbindir() + "/docker-compose");

   logmsg(kLDEBUG, "Downloading docker-compose...", p);
   utils::downloadexe(url, trgt, p);

   logmsg(kLDEBUG, "docker-compose installed.", p);
}

drunnerCompose::drunnerCompose(const service & svc, const params & p) : 
   mService(svc), 
   mParams(p), 
   mReadOkay(false),
   mVersion(0)
{
   if (!load_docker_compose_yml() && !load_servicecfg_sh())
      logmsg(kLDEBUG, "drunnerCompose - couldn't load either docker-compose.yml or servicecfg.sh for service "+svc.getName(), p);
}

void drunnerCompose::writeVariables()
{
   sh_variables shv(*this);

}

bool drunnerCompose::readOkay() const
{
   return mReadOkay;
}

const std::vector<cServiceInfo>& drunnerCompose::getServicesInfo() const
{
   return mServicesInfo;
}

void drunnerCompose::getDockerVols(tVecStr & dv) const
{
   for (const auto & entry : mServicesInfo)
      for (const auto & vol : entry.mVolumes)
         dv.push_back(vol.mDockerVolumeName);
}

std::string drunnerCompose::getImageName() const
{
   return mService.getImageName();
}

const service & drunnerCompose::getService() const
{
   return mService;
}

int drunnerCompose::getVersion() const
{
   return mVersion;
}

std::string makevolname(
   std::string mainServiceName,
   std::string serviceName,
   std::string mountpath
   )
{
   std::string dvol;

   if (serviceName.length()>0)
      dvol = "drunner-" + mainServiceName + "-" + serviceName + "-" + utils::alphanumericfilter(mountpath, false);
   else
      dvol = "drunner-" + mainServiceName + "-" + utils::alphanumericfilter(mountpath, false);
   return dvol;
}

bool drunnerCompose::load_docker_compose_yml()
{
   //mVersion = 2;
   return false;
}


bool drunnerCompose::load_servicecfg_sh()
{
   sh_legacy_servicecfg svcfg(mService.getPathServiceCfg());
   if (!svcfg.readOkay())
   {
      logmsg(kLDEBUG, "Couldn't read " + mService.getPathServiceCfg(),mParams);
      return false;
   }

   mVersion = 1;
   mReadOkay = true;

   // load from servicecfg.
   const tVecStr & mtpts = svcfg.getVolumes();
   const tVecStr & ectrs = svcfg.getExtraContainers();

   cServiceInfo ci;
   ci.mImageName = mService.getImageName();
   ci.mServiceName = mService.getName();

   for (const auto & mtpt : mtpts)
   {
      cVolInfo vi;
      vi.mDockerVolumeName = makevolname(mService.getName(), "", mtpt);
      vi.mMountPath = mtpt;
      ci.mVolumes.push_back(vi);
   }
   mServicesInfo.push_back(ci);

   logmsg(kLDEBUG, "Successfully read " + mService.getPathServiceCfg(), mParams);
   return true;
}

// std::string dvol = "drunner-" + drc.getService().getName() + "-" + entry.mServiceName + "-" + utils::alphanumericfilter(vol.mMountPath[i], false);


