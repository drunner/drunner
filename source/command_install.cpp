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

namespace command_install
{

using namespace utils;


   void validateImage(const params & p,const sh_drunnercfg & settings, std::string imagename)
   {
      if (!utils::fileexists(settings.getPath_Root())) logmsg(kLERROR,"ROOTPATH not set.",p);

      std::string op;
      int rval = utils::bashcommand(
         "docker run --rm -v \""+settings.getPath_Support()+
         ":/support\" \""+imagename+"\" /support/validator-image 2>&1",op);

      if (rval!=0)
         {
         if ( utils::findStringIC(op, "Unable to find image") )
            logmsg(kLERROR,"Couldn't find image "+imagename,p);
         else
            logmsg(kLERROR,op,p);
         }
      logmsg(kLINFO,"\u2714  " + imagename + " is dRunner compatible.");
   }

   void installService(const params & p,const sh_drunnercfg & settings,
      const std::string & imagename, std::string servicename)
   {
      if (servicename.length()==0)
      {
         servicename=imagename;
         size_t found;
         while ((found=servicename.find("/")) != std::string::npos)
            servicename.erase(0,found+1);
         while ((found=servicename.find(":")) != std::string::npos)
            servicename.erase(found);
      }

      std::string datestamp = utils::getTime(),hostIP = utils::getHostIP(p);
      logmsg(kLDEBUG,"date = "+datestamp+"  hostIP = "+hostIP,p);

      std::string targetdir=settings.getPath_Services() + "/" + servicename;
      logmsg(kLDEBUG,"Installing "+servicename+" at "+targetdir+", using image "+imagename,p);
      if (utils::fileexists(targetdir))
         logmsg(kLERROR,"Service already exists. Try:   drunner update "+servicename,p);

      // make sure we have the latest version of the service.
      command_setup::pullImage(p,settings,imagename);

      logmsg(kLDEBUG,"Attempting to validate "+imagename,p);
      validateImage(p,settings,imagename);

      try
      {
         // create service's drunner directory on host.
         std::string drd=targetdir+"/drunner";
         utils::makedirectory(drd,p, S_777 );

         // create service's temp directory
         std::string tmpdir=targetdir+"/temp";
         utils::makedirectory(tmpdir,p,S_777);

         // copy files to service directory on host.
         std::string op;
         int r=utils::bashcommand("docker run --rm -it -v "+
               drd+":/tempcopy "+imagename+" /bin/bash -c \"cp -r /drunner/* /tempcopy/\"",op);
         if (r!=0)
            logmsg(kLERROR,"Couldn't copy the service files. You will need to reinstall the service.",p);

         // read in servicecfg.sh
         sh_servicecfg servicecfg(p,drd);

         // write out variables.sh
         sh_variables variables(p,drd,servicecfg,servicename,imagename,hostIP,tmpdir);

         // make sure we have the latest of all exra containers.
         std::vector<std::string> extracontainers;
         extracontainers = servicecfg.getExtraContainers();
         for (uint i=0;i<extracontainers.size();++i)
            command_setup::pullImage(p,settings,extracontainers[i]);

         // create the utils.sh file for the dService.
         generate_utils_sh(drd,p);

      }

      catch (const eExit & e) {
         // tidy up.
         if (utils::fileexists(targetdir))
            utils::deltree(targetdir,p);

         throw (e);
      }
   }


}
