#ifndef __DOCKER_COMPOSE_H
#define __DOCKER_COMPOSE_H

#include <string>

#include "params.h"
#include "service.h"
#include "utils.h"

void InstallDockerCompose(const params & p);

class cVolInfo {
public:
   std::string mName;
   std::string mMountPath;
};

class cServiceInfo {
public:
   std::string mName;
   std::string mImageName;
   std::vector<cVolInfo> mVolumes;
};


class drunnerCompose {
public:
   // reads in servicecfg.sh or docker-compose.yml - whichever is present.
   drunnerCompose(const service & svc, const params & p);

   // writes out variables.sh
   void writeVariables();

   const std::vector<cServiceInfo> & getServices() const;

private:
   void load_docker_compose_yml();
   void load_variables_sh();

   std::vector<cServiceInfo> mServices;

   const params & mParams;
   const service & mService;
};

#endif
