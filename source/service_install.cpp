#include <sys/stat.h>

#include "command_dev.h"
#include "utils.h"
#include "logmsg.h"
#include "exceptions.h"
#include "settingsbash.h"
#include "command_setup.h"
#include "generate_utils_sh.h"
#include "sh_servicecfg.h"
#include "sh_variables.h"
#include "service.h"


//using namespace utils;


std::string service::getUserID()
{
	std::string runcmd = "docker run --rm -i " + getImageName() + R"EOF( /bin/bash -c "id -u | tr -d '\r\n'")EOF";
	std::string op;
	if (0 != utils::bashcommand(runcmd, op))
		logmsg(kLERROR, "Unable to determine the user id in container " + getImageName());

	logmsg(kLDEBUG, "Container is running under userID " + op + ".");
	return op;
}

void service::logmsg(eLogLevel level, std::string s) const
{
   ::logmsg(level, s, mParams.getLogLevel());
}

void service::createLaunchScript()
{
	std::string target = utils::get_usersbindir() + "/" + getName();

	// remove stale script if present
	if (utils::fileexists(target))
		if (remove(target.c_str()) != 0)
			logmsg(kLERROR, "Couldn't remove stale launch script at " + target);

	// write out new one.
	std::ofstream ofs;
	ofs.open(target);
	if (!ofs.is_open())
		logmsg(kLERROR, "Couldn't write launch script at " + target);
	ofs << "#!/bin/bash" << std::endl;
	ofs << "drunner servicecmd " << getName() << " \"$@\"" << std::endl;
	ofs.close();

	// fix permissions
	if (chmod(target.c_str(), S_700) != 0)
		logmsg(kLERROR, "Unable to change permissions on " + target);

	logmsg(kLDEBUG, "Created launch script at " + target);
}


void service::validateImage()
{
	if (!utils::fileexists(mSettings.getPath_Root())) logmsg(kLERROR, "ROOTPATH not set.");

	std::string op;
	int rval = utils::bashcommand(
		"docker run --rm -v \"" + mSettings.getPath_Support() +
		":/support\" \"" + getImageName() + "\" /support/validator-image 2>&1", op);

	if (rval != 0)
	{
		if (utils::findStringIC(op, "Unable to find image"))
			logmsg(kLERROR, "Couldn't find image " + getImageName());
		else
			logmsg(kLERROR, op);
	}
	logmsg(kLINFO, "\u2714  " + getImageName() + " is dRunner compatible.");
}


void service::createVolumes(const sh_variables * variables)
{
   if (variables == NULL)
      logmsg(kLERROR, "createVolumes passed NULL variables.");

	std::string userid = getUserID();
	if (userid == "0")
		logmsg(kLERROR, "Container is running as root user!"); // should never happen as we've validated the image already.

	const std::vector<std::string> & dockervols = variables->getDockerVols(); //getVec("DOCKERVOLS");
	const std::vector<std::string> & volumes = variables->getVolumes(); // getVec("VOLUMES");
	std::string dname = "dRunner-install-volcreate";

	if (dockervols.size() != volumes.size())
		logmsg(kLERROR, "Error in variables.sh - array sizes not equal for volumes and dockervols.");
	for (uint i = 0; i < dockervols.size(); ++i)
	{
		std::string volname = dockervols[i];
		std::string volpath = volumes[i];

		std::string op;
		int rval = utils::bashcommand("docker volume ls | grep \"" + volname + "\"", op);
		if (rval == 0)
			logmsg(kLINFO, "A docker volume already exists for " + volname + ", reusing it.");
		else
		{
			rval = utils::bashcommand("docker volume create --name=\"" + volname + "\"", op);
			if (rval != 0)
				logmsg(kLERROR, "Unable to create docker volume " + volname);
			// set permissions on volume.
			std::string chowncmd = "docker run --name=\"" + dname + "\" -v \"" + volname + ":" + volpath +
				"\" \"drunner/baseimage-alpine\" /bin/bash -c \"chown " + userid + ":root " + volpath + 
            " && date >> " + volpath + "/install_date";
			logmsg(kLDEBUG, chowncmd);
			rval = utils::bashcommand(chowncmd, op);
			if (rval != 0)
				logmsg(kLERROR, "Unable to chown the docker volume to userID " + userid);
			rval = utils::bashcommand("docker rm " + dname, op);
			if (rval != 0)
				logmsg(kLERROR, "Unable to remove temp docker container " + dname);
		}
	}
}

void service::recreate(bool updating)
{
   if (updating)
      command_setup::pullImage(mParams, mSettings, getImageName());
   
   try
   {
      // nuke any existing dService files on host (but preserve volume containers!).
      if (utils::fileexists(getPath()))
         utils::deltree(getPath(), mParams);

      // create the basic directories.
      ensureDirectoriesExist();

      // copy files to service directory on host.
      std::string op;
      int r = utils::bashcommand("docker run --rm -it -v " +
         getPathdRunner() + ":/tempcopy " + getImageName() + " /bin/bash -c \"cp -r /drunner/* /tempcopy/\"", op);
      if (r != 0)
         logmsg(kLERROR, "Couldn't copy the service files. You will need to reinstall the service.");

      // read in servicecfg.sh and write out variables.sh
      sh_variables variables(getPathVariables()); 
      if (variables.readOkay())
         fatal("Unexpected error - variables.sh already exists.");
      variables.createFromServiceCfg(*this);
      variables.write();

      // make sure we have the latest of all exra containers.
      std::vector<std::string> extracontainers;
      extracontainers = variables.getExtraContainers();
      for (uint i = 0; i < extracontainers.size(); ++i)
         command_setup::pullImage(mParams, mSettings, extracontainers[i]);

      // create the utils.sh file for the dService.
      generate_utils_sh(getPathdRunner(), mParams);

      // create launch script
      createLaunchScript();

      // create volumes
      createVolumes(&variables);
   }

   catch (const eExit & e) {
      // tidy up.
      if (utils::fileexists(getPath()))
         utils::deltree(getPath(), mParams);

      throw (e);
   }
}

void service::install()
{
	if (getName().length() == 0)
	{
		std::string servicename = getImageName();
		size_t found;
		while ((found = servicename.find("/")) != std::string::npos)
			servicename.erase(0, found + 1);
		while ((found = servicename.find(":")) != std::string::npos)
			servicename.erase(found);
      setName(servicename);
	}

   logmsg(kLDEBUG, "Installing " + getName() + " at " + getPath() + ", using image " + getImageName());
	if (utils::fileexists(getPath()))
		logmsg(kLERROR, "Service already exists. Try:   drunner update " + getName());

	// make sure we have the latest version of the service.
	command_setup::pullImage(mParams, mSettings, getImageName());

	logmsg(kLDEBUG, "Attempting to validate " + getImageName());
   validateImage();

   recreate(false);
}

