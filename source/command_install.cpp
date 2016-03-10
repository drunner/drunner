#include <sys/stat.h>

#include "command_dev.h"
#include "utils.h"
#include "logmsg.h"
#include "exceptions.h"
#include "settingsbash.h"
#include "command_setup.h"
#include "command_install.h"
#include "generate_utils_sh.h"
#include "sh_servicecfg.h"
#include "sh_variables.h"
#include "service.h"

namespace command_install
{

	using namespace utils;

	std::string getUserID(const std::string & imagename, const params & p)
	{
		std::string runcmd = "docker run --rm -i " + imagename + R"EOF( /bin/bash -c "id -u | tr -d '\r\n'")EOF";
		std::string op;
		if (0 != utils::bashcommand(runcmd, op))
			logmsg(kLERROR, "Unable to determine the user id in container " + imagename, p);

		logmsg(kLDEBUG, "Container is running under userID " + op + ".", p);
		return op;
	}

	void createLaunchScript(const std::string & servicename, const params & p)
	{
		std::string target = utils::get_usersbindir() + "/" + servicename;

		// remove stale script if present
		if (utils::fileexists(target))
			if (remove(target.c_str()) != 0)
				logmsg(kLERROR, "Couldn't remove stale launch script at " + target, p);

		// write out new one.
		std::ofstream ofs;
		ofs.open(target);
		if (!ofs.is_open())
			logmsg(kLERROR, "Couldn't write launch script at " + target, p);
		ofs << "#!/bin/bash" << std::endl;
		ofs << "drunner servicecmd " << servicename << " \"$@\"" << std::endl;
		ofs.close();

		// fix permissions
		if (chmod(target.c_str(), S_700) != 0)
			logmsg(kLERROR, "Unable to change permissions on " + target, p);

		logmsg(kLDEBUG, "Created launch script at " + target, p);
	}


	void validateImage(const params & p, const sh_drunnercfg & settings, std::string imagename)
	{
		if (!utils::fileexists(settings.getPath_Root())) logmsg(kLERROR, "ROOTPATH not set.", p);

		std::string op;
		int rval = utils::bashcommand(
			"docker run --rm -v \"" + settings.getPath_Support() +
			":/support\" \"" + imagename + "\" /support/validator-image 2>&1", op);

		if (rval != 0)
		{
			if (utils::findStringIC(op, "Unable to find image"))
				logmsg(kLERROR, "Couldn't find image " + imagename, p);
			else
				logmsg(kLERROR, op, p);
		}
		logmsg(kLINFO, "\u2714  " + imagename + " is dRunner compatible.");
	}


	void createVolumes(const std::string & imagename, const sh_variables & variables, const params & p)
	{
		std::string userid = getUserID(imagename, p);
		if (userid == "0")
			logmsg(kLERROR, "Container is running as root user!", p); // should never happen as we've validated the image already.

		const std::vector<std::string> & dockervols = variables.getDockerVols(); //getVec("DOCKERVOLS");
		const std::vector<std::string> & volumes = variables.getVolumes(); // getVec("VOLUMES");
		std::string dname = "dRunner-install-volcreate";

		if (dockervols.size() != volumes.size())
			logmsg(kLERROR, "Error in variables.sh - array sizes not equal for volumes and dockervols.", p);
		for (uint i = 0; i < dockervols.size(); ++i)
		{
			std::string volname = dockervols[i];
			std::string volpath = volumes[i];

			std::string op;
			int rval = utils::bashcommand("docker volume ls | grep \"" + volname + "\"", op);
			if (rval == 0)
				logmsg(kLINFO, "A docker volume already exists for " + volname + ", reusing it.", p);
			else
			{
				rval = utils::bashcommand("docker volume create --name=\"" + volname + "\"", op);
				if (rval != 0)
					logmsg(kLERROR, "Unable to create docker volume " + volname, p);
				// set permissions on volume.
				std::string chowncmd = "docker run --name=\"" + dname + "\" -v \"" + volname + ":" + volpath +
					"\" \"drunner/baseimage-alpine\" /bin/bash -c \"chown " + userid + ":root " + volpath + 
               " && date >> " + volpath + "/install_date";
				logmsg(kLDEBUG, chowncmd, p);
				rval = utils::bashcommand(chowncmd, op);
				if (rval != 0)
					logmsg(kLERROR, "Unable to chown the docker volume to userID " + userid, p);
				rval = utils::bashcommand("docker rm " + dname, op);
				if (rval != 0)
					logmsg(kLERROR, "Unable to remove temp docker container " + dname, p);
			}
		}
	}

   void recreateService(const params & p, const sh_drunnercfg & settings,
      const std::string & imagename, const service & svc, bool updating)
   {
      std::string datestamp = utils::getTime(), hostIP = utils::getHostIP(p);
      logmsg(kLDEBUG, "date = " + datestamp + "  hostIP = " + hostIP, p);

      if (updating)
         command_setup::pullImage(p, settings, imagename);

      try
      {
         // nuke any existing dService files on host (but preserve volume containers!).
         if (utils::fileexists(svc.getPath()))
            utils::deltree(svc.getPath(), p);

         // create the basic directories.
         svc.ensureDirectoriesExist();

         // copy files to service directory on host.
         std::string op;
         int r = utils::bashcommand("docker run --rm -it -v " +
            svc.getPathdRunner() + ":/tempcopy " + imagename + " /bin/bash -c \"cp -r /drunner/* /tempcopy/\"", op);
         if (r != 0)
            logmsg(kLERROR, "Couldn't copy the service files. You will need to reinstall the service.", p);

         // read in servicecfg.sh
         sh_servicecfg servicecfg(p, svc);

         // write out variables.sh
         sh_variables variables(p, svc, servicecfg, imagename, hostIP);

         // make sure we have the latest of all exra containers.
         std::vector<std::string> extracontainers;
         extracontainers = servicecfg.getExtraContainers();
         for (uint i = 0; i < extracontainers.size(); ++i)
            command_setup::pullImage(p, settings, extracontainers[i]);

         // create the utils.sh file for the dService.
         generate_utils_sh(svc.getPathdRunner(), p);

         // create launch script
         createLaunchScript(svc.getName(), p);

         // create volumes
         createVolumes(imagename, variables, p);
      }

      catch (const eExit & e) {
         // tidy up.
         if (utils::fileexists(svc.getPath()))
            utils::deltree(svc.getPath(), p);

         throw (e);
      }
   }

	void installService(const params & p, const sh_drunnercfg & settings,
		const std::string & imagename, service & svc)
	{
		if (svc.getName().length() == 0)
		{
			std::string servicename = imagename;
			size_t found;
			while ((found = servicename.find("/")) != std::string::npos)
				servicename.erase(0, found + 1);
			while ((found = servicename.find(":")) != std::string::npos)
				servicename.erase(found);
         svc.setName(servicename);
		}

      logmsg(kLDEBUG, "Installing " + svc.getName() + " at " + svc.getPath() + ", using image " + imagename, p);
		if (utils::fileexists(svc.getPath()))
			logmsg(kLERROR, "Service already exists. Try:   drunner update " + svc.getName(), p);

		// make sure we have the latest version of the service.
		command_setup::pullImage(p, settings, imagename);

		logmsg(kLDEBUG, "Attempting to validate " + imagename, p);
		validateImage(p, settings, imagename);

      recreateService(p, settings, imagename, svc, false);
	}


}
