#ifndef __UTILS_DOCKER_H
#define __UTILS_DOCKER_H

#include <string>
#include "params.h"
#include "drunner_settings.h"

namespace utils_docker
{
   bool dockerVolExists(const std::string & vol);
   bool dockerContainerExists(const std::string & container);

   void createDockerVolume(std::string name);
   void stopContainer(std::string name);
   void pullImage(const std::string & image);

   cResult runBashScriptInContainer(std::string data, std::string imagename, std::string & op);
}

#endif
