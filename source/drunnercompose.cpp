#include <sstream>
#include <fstream>

#include "drunnercompose.h"
#include "utils.h"
#include "logmsg.h"
#include "service.h"


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

   //const std::vector<std::string> & getDockerVols() const { return getVec("DOCKERVOLS"); }
   //const std::vector<std::string> & getVolumes()	const { return getVec("VOLUMES"); }
   //const std::vector<std::string> & getExtraContainers()	const { return getVec("EXTRACONTAINERS"); }
   //const std::string & getImageName() const { return getString("IMAGENAME"); }

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


//void DockerCompose_Create(std::string inputfile, std::string outputfile, const service & svc, const params & p)
//{
//   if (!utils::fileexists(inputfile))
//      logmsg(kLERROR, "Couldn't find Docker Compose file " + inputfile, p);
//
//   std::ifstream t(inputfile);
//   std::stringstream buffer;
//   buffer << t.rdbuf();
//   std::string dc(buffer.str());
//   
//   dc = utils::replacestring(dc, "${SERVICENAME}", svc.getName());
//
//   std::ofstream out(outputfile);
//   if (!out.is_open())
//      logmsg(kLERROR, "Couldn't write Docker Compose file to " + outputfile, p);
//   out << dc;
//   out.close();
//}

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
   mReadOkay(false)
{
   if (!load_docker_compose_yml())
      if (!load_docker_compose_yml())
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
   return mServices;
}

void drunnerCompose::getDockerVols(tVecStr & dv) const
{
   for (const auto & entry : mServices)
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

bool drunnerCompose::load_docker_compose_yml()
{
   return false;
}

bool drunnerCompose::load_servicecfg_sh()
{
   return false;
}

// std::string dvol = "drunner-" + drc.getService().getName() + "-" + entry.mServiceName + "-" + utils::alphanumericfilter(vol.mMountPath[i], false);


