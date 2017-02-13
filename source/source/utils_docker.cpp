#include <algorithm>
#include <iterator>

#include <Poco/String.h>
#include <Poco/Net/Socket.h>
#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Thread.h>

#include "basen.h"
#include "utils.h"
#include "utils_docker.h"
#include "globalcontext.h"
#include "globallogger.h"
#include "compress.h"
#include "dassert.h"

namespace utils_docker
{

   static std::vector<std::string> S_PullList;


   cResult createDockerVolume(std::string name)
   {
      CommandLine cl("docker", { "volume","create","--name=" + name });
      std::string op;
      int rval = utils::runcommand(cl, op);
      if (rval != 0)
      {
         logmsg(kLDEBUG, "Unable to create docker volume " + name);
         return cError("Unable to create docker volume " + name);
      }
      logmsg(kLDEBUG, "Created docker volume " + name);
      return kRSuccess;
   }

   cResult deleteDockerVolume(std::string name)
   {
      logmsg(kLINFO, "Obliterating docker volume " + name);
      std::string op;
      CommandLine cl("docker", { "volume", "rm", name });
      if (0 != utils::runcommand(cl, op))
      {
         logmsg(kLDEBUG, "Failed to remove " + name + ":" + op);
         return cError("Failed to remove " + name + ":" + op);
      }
      return kRSuccess;
   }

   cResult stopContainer(std::string name)
   {
      CommandLine cl("docker", { "stop",name });
      std::string op;
      int rval = utils::runcommand(cl, op);
      if (rval != 0)
      {
         logmsg(kLDEBUG, "Failed to stop docker container " + name + "\n" + op);
         return cError("Failed to stop docker container " + name + "\n" + op);
      }
      logmsg(kLDEBUG, "Stopped docker container " + name);
      return kRSuccess;
   }

   cResult removeContainer(std::string name)
   {
      CommandLine cl("docker", { "rm", name });
      std::string op;
      int rval = utils::runcommand(cl, op);
      if (rval != 0)
      {
         logmsg(kLDEBUG, "Unable to remove docker container " + name + "\n" + op);
         return cError("Unable to remove docker container " + name + "\n" + op);
      }
      logmsg(kLDEBUG, "Removed docker container " + name);
      return kRSuccess;
   }

   cResult pullImage(const std::string & image)
   {
//#ifdef _DEBUG
//      logmsg(kLDEBUG, "DEBUG BUILD - not pulling");
//      return kRSuccess;
//#endif

      if (GlobalContext::getParams()->isDevelopmentMode())
      {
         logmsg(kLDEBUG, "In developer mode - not pulling " + image);
         return kRSuccess;
      }

      if (!GlobalContext::getSettings()->getPullImages())
      {
         logmsg(kLDEBUG, "Pulling images disabled in the global dRunner configuration.");
         return kRSuccess;
      }

      if (std::find(S_PullList.begin(), S_PullList.end(), image) != S_PullList.end())
      { // pulling is slow. Never do it twice in one command.
         logmsg(kLDEBUG, "Already pulled " + image + " so not pulling again.");
         return kRSuccess;
      }


      logmsg(kLINFO, "Pulling Docker image " + image + ".\n This may take some time...");


      // pull the image
      std::string op;
      CommandLine cl("docker", { "pull", image });
      int rval = utils::runcommand_stream(cl, GlobalContext::getParams()->supportCallMode(), "", {},&op);

      if (rval != 0) 
      {
         logmsg(kLINFO, "Couldn't pull " + image);
         return cError("Couldn't pull " + image);
      }

      S_PullList.push_back(image);
      logmsg(kLDEBUG, "Successfully pulled " + image);
      return kRSuccess;
   }

   cResult runBashScriptInContainer(std::string data, std::string imagename, std::string & op)
   {
      std::string encoded_data = utils::base64encodeWithEquals(data);

      CommandLine cl("docker", {"run","--rm",imagename,"/bin/bash","-c",
         "echo " + encoded_data + " | base64 -d > /tmp/_script ; /bin/bash /tmp/_script"});

      int rval=utils::runcommand(cl, op);
      Poco::trimInPlace(op);
      return (rval == 0 ? cResult(kRSuccess) : cError("Command failed: " + op));
   }

   bool dockerVolExists(const std::string & vol)
   {
      CommandLine cl("docker", { "volume","ls","-f","name=" + vol });
      std::string out;
      int rval = utils::runcommand(cl,out);
      return (rval != 0 ? false : utils::wordmatch(out, vol));
   }


   bool dockerContainerExists(const std::string & container)
   {
      CommandLine cl("docker", { "ps","-a","-f","name=" + container });
      std::string out;
      int rval = utils::runcommand(cl, out);
      return (rval != 0 ? false : utils::wordmatch(out, container));
   }

   bool dockerContainerRunning(const std::string & container)
   {
      CommandLine cl("docker", { "ps","-f","name=" + container });
      std::string out;
      int rval = utils::runcommand(cl, out);
      return (rval != 0 ? false : utils::wordmatch(out, container));
   }

   bool dockerContainerWait(const std::string & containername, int port, int timeout)
   {
      // get IP address of container.

      for (int i = 0; i < timeout; ++i)
      {
         CommandLine cl("docker", { "inspect","--format","'{{ .NetworkSettings.IPAddress }}'", containername });
         std::string out;
         int rval = utils::runcommand(cl, out);
         if (rval == 0)
         {
            Poco::trimInPlace(out);
            // test if IP address responds on given port.
            out += ":" + std::to_string(port);
            logmsg(kLDEBUG, "Checking address " + out);
            Poco::Net::SocketAddress sa(out);

            bool socketok = false;
            try {
               Poco::Net::StreamSocket sock(sa);
               sock.shutdown();
               socketok = true;
            }
            catch (Poco::Exception & e)
            {
               logmsg(kLDEBUG, e.what());
            }
            if (socketok)
               return true;
            Poco::Thread::sleep(1000);
         }
      }

      // run script in guest.
      return false;
   }

   bool dockerContainerRunsAsRoot(std::string container)
   {
      std::string script = R"EOF(
#!/bin/sh
[ "$(whoami)" != "root" ] || { echo "Running as root."; exit 0; }
echo "Not running as root."
exit 1
)EOF";
      std::string op;
      cResult r = runBashScriptInContainer(script, container, op);
      logdbg(op);
      return r.success(); // returns 0 if root (success).
   }

   cResult backupDockerVolume(std::string volumename, Poco::Path TempBackupFolder, std::string servicename)
   {
      // -----------------------------------------
      // back up volume container
      std::string password = utils::getenv("PASS");
      std::string backupName = volumename; // todo : make it robust to weird chars etc.

      drunner_assert(TempBackupFolder.isDirectory(), "Coding error: volarchive needs to be directory.");

      // strip out servicename.
      size_t pos = backupName.find(servicename);
      if (pos != std::string::npos)
         backupName.erase(pos, servicename.length());

      if (!utils_docker::dockerVolExists(volumename))
         fatal("Couldn't find docker volume " + volumename + ".");

      TempBackupFolder.setFileName(backupName + ".tar");
      compress::compress_volume(password, volumename, TempBackupFolder);
      logmsg(kLDEBUG, "Backed up docker volume " + volumename + " as " + backupName);

      return kRSuccess;
   }

   cResult restoreDockerVolume(std::string volumename, Poco::Path TempBackupFolder, std::string servicename)
   {
      // -----------------------------------------
      // restore volume container
      std::string password = utils::getenv("PASS");
      std::string backupName = volumename; // todo : make it robust to weird chars etc.

      drunner_assert(TempBackupFolder.isDirectory(), "Coding error: volarchive needs to be directory.");

      // strip out servicename.
      size_t pos = backupName.find(servicename);
      if (pos != std::string::npos)
         backupName.erase(pos, servicename.length());

      if (utils_docker::dockerVolExists(volumename))
         fatal("Volume already exists: " + volumename + " - can't restore.");

      // create the volume to restore into.
      utils_docker::createDockerVolume(volumename);

      TempBackupFolder.setFileName(backupName + ".tar");
      if (!utils::fileexists(TempBackupFolder))
         fatal("Expected archive does not exist: " + TempBackupFolder.toString());

      compress::decompress_volume(password, volumename, TempBackupFolder);
      logmsg(kLDEBUG, "Restored docker volume " + volumename + " as " + backupName);

      return cResult();
   }

} // namespace