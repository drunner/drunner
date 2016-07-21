#ifndef __SERVICECLASS_H
#define __SERVICECLASS_H

#include "params.h"
#include "drunner_settings.h"
#include "cresult.h"
#include "service_paths.h"
#include "service_yml.h"
#include "service_variables.h"
#include "service_config.h"

// class to manage the dService.
class service : public servicePaths
{
public:
   // will load imagename from variables.sh unless overridden with parameter.
   service(std::string servicename); 

   cResult servicecmd();

   int status();
   void backup(const std::string & backupfile);
   //void enter(); // uses execl, so never returns.

   const std::string getImageName() const;
   const params & getParams() const;

   cResult serviceRunnerCommand(const std::vector<std::string> & args) const;

   const serviceyml::file & getServiceYml() const { return mServiceYml; }
   const serviceConfig & getServiceCfg() const { return mServiceCfg; }

   //cServiceEnvironment & getEnvironment();
   //const cServiceEnvironment & getEnvironmentConst() const;

private:
   static std::string _loadImageName(const std::string & servicename, std::string imagename);
   cResult _handleconfigure(const std::vector<std::string> & cargs);
   cResult _runserviceRunnerCommand(const serviceyml::CommandLine & x, const std::vector<std::string> & args) const;
   cResult _launchOperation(std::string command, const std::vector<std::string> & args) const;
   cResult _handleStandardCommands(std::string command, const std::vector<std::string> & args, bool & processed) const;
   std::string mImageName;
   serviceConfig mServiceCfg;
   serviceyml::file mServiceYml;
};



#endif

