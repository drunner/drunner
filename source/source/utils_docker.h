#ifndef __UTILS_DOCKER_H
#define __UTILS_DOCKER_H

#include <string>
#include "params.h"
#include "drunner_settings.h"

namespace utils_docker
{
   bool dockerVolExists(const std::string & vol);
   bool dockerContainerExists(const std::string & container);
   bool dockerContainerRunning(const std::string & container);
   bool dockerContainerWait(const std::string & containername, int port, int timeout);

   std::string getIPAddress(const std::string & containername, const std::string & network="");

   cResult createDockerVolume(std::string name);
   cResult deleteDockerVolume(std::string name);
   cResult createDockerNetwork(std::string name);
   cResult stopContainer(std::string name);
   cResult removeContainer(std::string name);
   cResult pullImage(const std::string & image);

   cResult runBashScriptInContainer(std::string data, std::string imagename, std::string & op);
   bool dockerContainerRunsAsRoot(std::string container);

   cResult backupDockerVolume(std::string volumename, Poco::Path TempBackupFolder, std::string servicename);
   cResult restoreDockerVolume(std::string volumename, Poco::Path TempBackupFolder, std::string servicename);
}

#endif
