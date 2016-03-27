#include <sstream>
#include <fstream>
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "yaml-cpp/yaml.h"

#include "drunnercompose.h"
#include "utils.h"
#include "logmsg.h"
#include "service.h"

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
   mReadOkay(kRError)
{
   load_docker_compose_yml();
}

void drunnerCompose::setvecenv(const sb_vec & v) const
{
   bashline bl = v.getBashLine();
   std::string key = bl.getkey();
   std::string val = bl.getvalue();
   setenv_log(key, val);
}

void drunnerCompose::setenv_log(std::string key, std::string val) const
{
   setenv(key.c_str(), val.c_str(), 1);
   logmsg(kLDEBUG, "environment: "+key + "=" + val, mParams);
}

void drunnerCompose::setServiceRunnerEnv() const
{
   //std::string envvol = mService.getName() + "-environment";
   //for (const auto & entry : getVolumes() )
   //   if (entry.mDockerVolumeName == envvol)
   //   {
   //      logmsg(kLDEBUG, "Reading stored environment from volume " + envvol,mParams);

   //      std::string op;
   //      utils::bashcommand("docker run --name=\"drunner-env-"+mService.getName()+"\" "+
   //         "drunner/rootutils readstoredenv",op)

   //      break;
   //   }


   setenv_log("SERVICENAME", getService().getName().c_str());
   setenv_log("IMAGENAME", getService().getImageName().c_str());
   setenv_log("SERVICETEMPDIR", getService().getPathTemp().c_str());
   setenv_log("SERVICEHOSTVOL", getService().getPathHostVolume_servicerunner().c_str());
   setenv_log("HOSTIP", utils::getHostIP().c_str());
}

cResult drunnerCompose::readOkay() const
{
   return mReadOkay;
}

const std::vector<cServiceInfo>& drunnerCompose::getServicesInfo() const
{
   return mServicesInfo;
}

const std::vector<cVolInfo>& drunnerCompose::getVolumes() const
{
   return mVolumes;
}

void drunnerCompose::getDockerVols(tVecStr & dv) const
{
   for (const auto & entry : mVolumes)
      dv.push_back(entry.mDockerVolumeName);
}

std::string drunnerCompose::getImageName() const
{
   return mService.getImageName();
}

const service & drunnerCompose::getService() const
{
   return mService;
}

//std::string makevolname(
//   std::string mainServiceName,
//   std::string mountpath
//   )
//{
//   return "drunner-" + mainServiceName + "-" + utils::alphanumericfilter(mountpath, false);
//}

void drunnerCompose::load_docker_compose_yml()
{
   if (!utils::fileexists(mService.getPathDockerCompose()))
   {
      mReadOkay = kRNotImplemented;
      return;
   }

   logmsg(kLDEBUG, "Parsing " + mService.getPathDockerCompose(), mParams);

   YAML::Node config = YAML::LoadFile(mService.getPathDockerCompose());
   if (!config)
      logmsg(kLERROR, "Failed to load the docker-compose.yml file. Parse error?", mParams);

   if (!config["version"] || config["version"].as<int>() != 2)
      logmsg(kLERROR, "dRunner requires the docker-compose.yml file to be Version 2 format.", mParams);

   // parse volumes.
   if (config["volumes"])
   {
      YAML::Node volumes = config["volumes"];
      if (!volumes)
         logmsg(kLERROR, "docker-compose.yml is missing the required volumes section.", mParams);
      if (volumes.Type() != YAML::NodeType::Map)
         logmsg(kLERROR, "docker-compose.yml is malformed - volumes is not a map.\n (Do you have an empty services: section? If so, delete it!).", mParams);
      for (auto it = volumes.begin(); it != volumes.end(); ++it)
      {
         cVolInfo volinfo;
         volinfo.mLabel = it->first.as<std::string>();
         YAML::Node external = it->second["external"];
         if (!external) logmsg(kLDEBUG, "Volume " + volinfo.mLabel + " is not managed by dRunner.", mParams);
         else
         {
            if (!external["name"]) logmsg(kLERROR, "Volume " + volinfo.mLabel + " is missing a required name:", mParams);
            volinfo.mDockerVolumeName = external["name"].as<std::string>();
            // need to var substitute ${SERVICENAME} in it.
            volinfo.mDockerVolumeName = utils::replacestring(volinfo.mDockerVolumeName, "${SERVICENAME}", mService.getName());
            logmsg(kLDEBUG, "dRunner managed volume: " + volinfo.mDockerVolumeName, mParams);
            mVolumes.push_back(volinfo);
         }
      }

      if (mVolumes.size() == 0)
         logmsg(kLDEBUG, "There are no dRunner managed volumes.", mParams);
   }
   else
      logmsg(kLDEBUG, "No volumes are specified in the docker-compose.yml file.\n That means dRunner won't manage any.", mParams);

   // parse services.
   if (config["services"])
   {
      YAML::Node services = config["services"];
      if (services.Type() != YAML::NodeType::Map)
         logmsg(kLERROR, "docker-compose.yml is malformed - services is not a map.", mParams);

      for (auto it = services.begin(); it != services.end(); ++it)
      {
         cServiceInfo sinf;

         sinf.mDockerServiceName = it->first.as<std::string>();
         logmsg(kLDEBUG, "Docker-compose service " + sinf.mDockerServiceName + " found.", mParams);

         if (!it->second["image"])
            logmsg(kLERROR, "Docker-compose service " + sinf.mDockerServiceName + " is missing image definition.", mParams);
         sinf.mImageName = it->second["image"].as<std::string>();
         logmsg(kLDEBUG, sinf.mDockerServiceName + " - Image:  " + sinf.mImageName, mParams);

         YAML::Node volumes = it->second["volumes"];
         if (volumes.IsNull())
            logmsg(kLDEBUG, sinf.mDockerServiceName + " - No volumes defined.", mParams);
         else
            for (auto vol = volumes.begin(); vol != volumes.end(); ++vol)
            {
               cServiceVolInfo volinfo;
               std::string v = vol->as<std::string>();
               size_t pos = v.find(':');
               if (pos == std::string::npos || pos==0 || pos==v.length()-1)
                  logmsg(kLERROR, "Couldn't parse volume info from service " + sinf.mDockerServiceName + " - missing : in " + v, mParams);

               volinfo.mLabel = v.substr(0, pos);
               volinfo.mMountPath = v.substr(pos + 1);
               volinfo.mDockerVolumeName = "";
               for (auto entry : mVolumes)
                  if (entry.mLabel == volinfo.mLabel)
                     volinfo.mDockerVolumeName = entry.mDockerVolumeName;
               if (volinfo.mDockerVolumeName.length() == 0)
                  logmsg(kLERROR, "Volume " + volinfo.mLabel + " is not defined in the volumes: section! docker-compose.yml is broken.", mParams);

               logmsg(kLDEBUG, sinf.mDockerServiceName + " - Volume " + volinfo.mDockerVolumeName + " is to be mounted at "+volinfo.mMountPath, mParams);
               sinf.mVolumes.push_back(volinfo);
            }
         mServicesInfo.push_back(sinf);
      }
   }
   else
      logmsg(kLDEBUG, "No services are specified in the docker-compose.yml file.\n That means dRunner will just use "+mService.getImageName(), mParams);

   mReadOkay = kRSuccess;
}

// std::string dvol = "drunner-" + drc.getService().getName() + "-" + entry.mServiceName + "-" + utils::alphanumericfilter(vol.mMountPath[i], false);


