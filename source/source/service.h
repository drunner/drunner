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
   void enter(); // uses execl, so never returns.

   const std::string getImageName() const;
   const params & getParams() const;

   cResult serviceRunnerCommand(const std::vector<std::string> & args) const;

   const serviceyml::file & getServiceYml() const { return mServiceYml; }
   const serviceConfig & getServiceCfg() const { return mServiceCfg; }

   //cServiceEnvironment & getEnvironment();
   //const cServiceEnvironment & getEnvironmentConst() const;

private:
   static std::string loadImageName(const std::string & servicename, std::string imagename);

   std::string mImageName;
   serviceConfig mServiceCfg;
   serviceyml::file mServiceYml;
};



#endif

