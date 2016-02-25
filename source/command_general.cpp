#include <sys/stat.h>

#include "command_general.h"
#include "logmsg.h"
#include "enums.h"
#include "utils.h"
#include "command_setup.h"
#include "exceptions.h"

namespace command_general
{

   void showservices(const params & p, const drunner_settings & settings)
   {
      std::string parent = settings.getPath_Services();
      std::vector<std::string> services;
      if (!utils::getFolders(parent,services))
         logmsg(kLERROR,"Couldn't get subfolders of "+parent,p);
      if (services.size()>0)
      {
         logmsg(kLINFO,"Installed dServices:",p);
         for (auto const & entry : services)
            logmsg(kLINFO,"  "+entry,p);
      } else
         logmsg(kLERROR,"No dServices are installed.",p);
   }


   void clean(const params & p, const drunner_settings & settings)
   {
      std::string op;
      logmsg(kLINFO,"Pulling latest spotify/docker-gc.",p);
      if (utils::pullimage("spotify/docker-gc")==kRError)
         logmsg(kLERROR,"Failed to pull spotify/docker-gc");

      logmsg(kLINFO,"Cleaning.",p);
      if (utils::bashcommand("docker run --rm -v /var/run/docker.sock:/var/run/docker.sock spotify/docker-gc",op) != 0)
         logmsg(kLERROR,"Unable to run spotify/docker-gc to clean docker images.",p);

      logmsg(kLINFO,"Cleaning is complete.",p);
   }

   void update(const params & p, const drunner_settings & settings)
   {

      logmsg(kLERROR,"Soon!",p);
   }


   void asdf(const params & p, const drunner_settings & settings)
   {
      logmsg(kLERROR,R"EOF(

            /-------------------------------------------------------------\
            |    That command isn't fully implemented and I am sad. :,(   |
            \-------------------------------------------------------------/
      )EOF",p);
   }


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

      std::string targetdir=settings.getPath_Services() + "/" + servicename;
      logmsg(kLDEBUG,"Installing "+servicename+" at "+targetdir+", using image "+imagename,p);
      if (utils::fileexists(targetdir))
         logmsg(kLERROR,"Service already exists. Try:   drunner update "+servicename,p);

      command_setup::pullImage(p,settings,imagename);

      logmsg(kLDEBUG,"Attempting to validate "+imagename,p);
      validateImage(p,settings,imagename);

      try
      {
         utils::makedirectory(targetdir,p);
         if (0!= chmod(targetdir.c_str(), S_IRWXU | S_IRWXG	| S_IRWXO )) // http://linux.die.net/include/sys/stat.h
            logmsg(kLERROR,"Unable to set permissions on "+targetdir);
      }

      catch (const eExit & e) {
         // tidy up.
         if (utils::fileexists(targetdir))
            utils::deltree(targetdir,p);

         throw (e);
      }
   }



}
