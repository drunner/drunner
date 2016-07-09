#ifndef __UTILS_DOCKER_H
#define __UTILS_DOCKER_H

#include <string>
#include "params.h"
#include "drunnerSettings.h"

namespace utils_docker
{
   void pullImage(const std::string & image);
}

#endif
