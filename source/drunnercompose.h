#ifndef __DOCKER_COMPOSE_H
#define __DOCKER_COMPOSE_H

#include <string>

#include "params.h"
#include "service.h"
#include "utils.h"

void InstallDockerCompose(const params & p);

class cVolInfo {
public:
   std::string mDockerVolumeName;
   std::string mMountPath;
};

class cServiceInfo {
public:
   std::string mServiceName;
   std::string mImageName;
   std::vector<cVolInfo> mVolumes;
};


class drunnerCompose {
public:
   // reads in servicecfg.sh or docker-compose.yml - whichever is present.
   drunnerCompose(const service & svc, const params & p);

   // writes out variables.sh
   void writeVariables();

   bool readOkay() const;

   const std::vector<cServiceInfo> & getServicesInfo() const;
   void getDockerVols(tVecStr & dv) const;
   std::string getImageName() const;

   const service & getService() const;

   int getVersion() const;

private:
   bool load_docker_compose_yml();
   bool load_servicecfg_sh();

   std::vector<cServiceInfo> mServicesInfo;

   const service & mService;
   const params & mParams;
   bool mReadOkay;
   int mVersion;
};

#endif
