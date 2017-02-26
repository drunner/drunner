#include <sstream>

#include "Poco/String.h"

#include "caddy.h"
#include "proxy.h"
#include "utils_docker.h"
#include "drunner_paths.h"

// generate the caddyfile in the shared volume
cResult caddy::generate()
{
   std::ostringstream oss;

   for (auto x : mProxyData)
   {
      std::string ip = utils_docker::getIPAddress(x.container, proxy::networkName());
      if (ip.length() == 0)
      {
         logmsg(kLWARN, "The container " + x.container + " does not appear to be attached to the proxy network '" + proxy::networkName() + "'.");
         logmsg(kLWARN, "Couldn't determine IP address for " + x.container + " - skipping proxy configuration.");
      }
      else
      {
         bool fakemode = (Poco::icompare(x.mode, "fake") == 0);

         oss << "https://" << x.domain << " {" << std::endl;
         oss << "   proxy / " << ip << ":" << x.port << " {" << std::endl;
         oss << "      transparent" << std::endl;
         oss << "      websocket" << std::endl;
         oss << "   }" << std::endl;
         oss << "   gzip" << std::endl;
         oss << "   tls " <<
            (fakemode ? "self_signed" : x.email)
            << std::endl;

         if (!x.timeouts)
            oss << "   timeouts none" << std::endl;

         oss << "}" << std::endl << std::endl;

         oss << "http://" << x.domain << " {" << std::endl;
         oss << "   redir https://" << x.domain << std::endl;
         oss << "}" << std::endl << std::endl;
      }
   }

   std::string encoded_data = utils::base64encodeWithEquals(oss.str());
   logmsg(kLDEBUG, oss.str());

   // generate the caddy file in the drunner-proxy-dataVolume volume.
   CommandLine cl("docker", { "run","--rm",
      "-v",dataVolume() + ":/data",
      drunnerPaths::getdrunnerUtilsImage(),
      "/bin/bash","-c",
      "echo " + encoded_data + " | base64 -d > /data/caddyfile" });

   std::string op;
   int rval = utils::runcommand(cl, op);
   if (rval != 0)
   {
      Poco::trimInPlace(op);
      return cError("Command failed: " + op);
   }

   return kRSuccess;
}

// restart the service
cResult caddy::restart()
{
   std::string op;
   if (utils_docker::dockerContainerRunning(containerName()))
   { // just send signal to restart it.
     //      docker exec <container> kill - SIGUSR1 1
      logmsg(kLINFO, "Reloading dRunner proxy settings (SIGUSR1)");
      CommandLine cl("docker", { "exec",containerName(),"kill","-SIGUSR1","1" });
      int rval = utils::runcommand(cl, op);
      if (rval != 0)
         return cError("Command failed: " + op);
      return kRSuccess;
   }

   if (utils_docker::dockerContainerExists(containerName()))
   { // get rid of old crufty container.
      logmsg(kLWARN, "Removing unexpected stopped proxy container! " + containerName());
      utils_docker::removeContainer(containerName());
   }


   if (mProxyData.size() == 0)
      return kRSuccess; // no need for proxy!

   logmsg(kLDEBUG, "Starting dRunner proxy.");

   // Container is not running. Volume get auto-created if not already present.
   // Note that the /root/.caddy volume mapping is critical to not burn LetsEncrypt certs on
   // container restart and hit their issuing limit!
   CommandLine cl("docker",
   {
      "run",
      "-v",dataVolume() + ":/data",
      "-v",rootVolume() + ":/root/.caddy",
      "--name",containerName(),
      "-p","80:80",
      "-p","443:443",
      "--restart=always",
      "--network=" + proxy::networkName(),
      "-d",
      drunnerPaths::getdrunnerProxyImage()
   });

   int rval = utils::runcommand(cl, op);
   if (rval != 0)
      return cError("Command failed: " + op);

   // wait for the proxy to come up.
   logmsg(kLDEBUG, "Waiting for proxy to come up.");
   if (!utils_docker::dockerContainerWait(containerName(), 80, 120))
      logmsg(kLWARN, "The caddy proxy container didn't come up. O_o");

   return kRSuccess;
}
