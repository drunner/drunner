#include <sys/stat.h>

#include "command_dev.h"
#include "utils.h"
#include "logmsg.h"
#include "exceptions.h"
#include "settingsbash.h"
#include "command_setup.h"
#include "command_install.h"
#include "createutils_sh.h"

namespace command_install
{

using namespace utils;

   std::string alphanumericfilter(std::string s)
   {
      std::string validchars="0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
      size_t pos;
      while((pos = s.find_first_not_of(validchars)) != std::string::npos)
         s.erase(pos,1);
      return s;
   }


   class sh_servicecfg : public settingsbash
   {
   public:
      // read ctor
      sh_servicecfg(
         const params & p,
         const drunner_settings & settings,
         std::string path
         )
         :  settingsbash(p,path+"/servicecfg.sh")
      {
         std::vector<std::string> nothing;
         setSettingv("VOLUMES",nothing);
         setSettingv("EXTRACONTAINERS",nothing);

         bool readok = readSettings();
         if (!readok)
            logmsg(kLERROR,"Not a valid dService. Couldn't read "+path+"/servicecfg.sh",p);
      } // ctor

      void getVolumes(std::vector<std::string> & volumes) const
         {getSettingv("VOLUMES",volumes);}
      void getExtraContainers(std::vector<std::string> & extracontainers) const
         {getSettingv("EXTRACONTAINERS",extracontainers);}
   }; //class


   class sh_variables : public settingsbash
   {
   public:

      // reading ctor
      sh_variables(const params & p, std::string path)
         : settingsbash(p,path+"/variables.sh")
         {
         bool readok = readSettings();
         if (!readok)
            logmsg(kLERROR,"Broken dService. Couldn't read "+path+"/variables.sh",p);
         }

      // writing ctor
      sh_variables(
         const params & p,
         const drunner_settings & settings,
         std::string path,
         const sh_servicecfg & servicecfg,
         std::string servicename,
         std::string imagename,
         std::string hostIP,
         std::string serviceTempDir
         )
         :  settingsbash(p,path+"/variables.sh")
      {
         std::vector<std::string> volumes, extracontainers,dockervols,dockeropts;
         servicecfg.getVolumes(volumes);
         servicecfg.getExtraContainers(extracontainers);

         for (uint i=0;i<volumes.size();++i)
            {
            logmsg(kLDEBUG, "VOLUME:          "+volumes[i],p);
            dockervols.push_back("drunner-"+servicename+"-"+alphanumericfilter(volumes[i]));
            logmsg(kLDEBUG, "Docker Volume:   "+dockervols[i],p);
            dockeropts.push_back("-v");
            dockeropts.push_back(dockervols[i]+":"+volumes[i]);
            }
         for (uint i=0;i<extracontainers.size();++i)
            logmsg(kLDEBUG, "EXTRACONTAINER:  "+extracontainers[i],p);

         setSettingv("VOLUMES",volumes);
         setSettingv("EXTRACONTAINERS",extracontainers);
         setSetting("SERVICENAME",servicename);
         setSetting("IMAGENAME",imagename);
         setSetting("INSTALLTIME",utils::getTime());
         setSetting("HOSTIP",hostIP);
         setSetting("SERVICETEMPDIR",serviceTempDir);
         setSettingv("DOCKERVOLS",dockervols);
         setSettingv("DOCKEROPTS",dockeropts);
         writeSettings();
      }
   }; // sh_variables




   void validateImage(const params & p,const drunner_settings & settings, std::string imagename)
   {
      if (!utils::fileexists(settings.getPath_Root())) logmsg(kLERROR,"ROOTPATH not set.",p);

      std::string op;
      int rval = utils::bashcommand("docker run --rm -v \""+settings.getPath_Support()+
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

   void installService(const params & p,const drunner_settings & settings,
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
         utils::makedirectory(drd,p);
         if (0!= chmod(drd.c_str(), S_IRWXU | S_IRWXG	| S_IRWXO )) // http://linux.die.net/include/sys/stat.h
            logmsg(kLERROR,"Unable to set permissions on "+drd);

         // create service's temp directory
         std::string serviceTempDir=settings.getPath_TempServices()+"/"+servicename;
         utils::makedirectory(serviceTempDir,p);

         // copy files to service directory on host.
         std::string op;
         int r=utils::bashcommand("docker run --rm -it -v "+
               drd+":/tempcopy "+imagename+" /bin/bash -c \"cp -r /drunner/* /tempcopy/\"",op);
         if (r!=0)
            logmsg(kLERROR,"Couldn't copy the service files. You will need to reinstall the service.",p);

         // read in servicecfg.sh
         sh_servicecfg servicecfg(p,settings,drd);

         // write out variables.sh
         sh_variables variables(p,settings,drd,servicecfg,servicename,imagename,hostIP,serviceTempDir);

         // make sure we have the latest of all exra containers.
         std::vector<std::string> extracontainers;
         servicecfg.getExtraContainers(extracontainers);
         for (uint i=0;i<extracontainers.size();++i)
            command_setup::pullImage(p,settings,extracontainers[i]);

         // create the utils.sh file for the dService.
         createutils_sh(drd,p);

      }

      catch (const eExit & e) {
         // tidy up.
         if (utils::fileexists(targetdir))
            utils::deltree(targetdir,p);

         throw (e);
      }
   }


}
