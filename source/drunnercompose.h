#ifndef __DOCKER_COMPOSE_H
#define __DOCKER_COMPOSE_H

#include <string>

#include "params.h"
#include "service.h"
#include "utils.h"

void InstallDockerCompose(const params & p);

class cVolInfo {
public:
   std::string mLabel;
   std::string mDockerVolumeName;
};

class cServiceVolInfo : public cVolInfo {
public:
   std::string mMountPath;
};

class cServiceInfo {
public:
   std::string mServiceName;
   std::string mImageName;
   std::vector<cServiceVolInfo> mVolumes;
};


class drunnerCompose {
public:
   // reads in servicecfg.sh or docker-compose.yml - whichever is present.
   drunnerCompose(const service & svc, const params & p);

   // writes out variables.sh
   void writeVariables();

   // was the servicecfg.sh or docker-compose.yml file read okay.
   bool readOkay() const;

   // Services are as defined in the docker-compose.yml. These are not dServices.
   const std::vector<cServiceInfo> & getServicesInfo() const;

   // Volumes are as defined in the docker-compose.yml. These include all the volumes
   // defined in services, but may include extras.
   const std::vector<cVolInfo> & getVolumes() const;

   // get a vector of mDockerVolumeName's (for convenience) - straight from mVolumes.
   void getDockerVols(tVecStr & dv) const;

   // just queries the stored service for conveninece.
   std::string getImageName() const;

   const service & getService() const;

   // 1 = old servicecfg.sh, 2 = new servicecfg.sh, 3 = docker-compose.yml
   int getVersion() const;

private:
   bool load_docker_compose_yml();
   bool load_servicecfg_sh();

   std::vector<cServiceInfo> mServicesInfo;
   std::vector<cVolInfo> mVolumes;

   const service & mService;
   const params & mParams;
   bool mReadOkay;
   int mVersion;
};

#endif
