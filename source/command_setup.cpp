#include <boost/filesystem.hpp>
#include <sstream>

#include "command_setup.h"
#include "utils.h"
#include "sh_drunnercfg.h"
#include "logmsg.h"
#include "generate_validator_image.h"

namespace command_setup
{

   void pullImage(const params & p,const sh_drunnercfg & s, const std::string & image)
   {
      // -----------------------------------------------------------------------------
      // pull the rootutils image to ensure we have the latest.
      if (s.getPullImages())
      {
         logmsg(kLDEBUG,"Pulling Docker image " + image,p);
         eResult rslt = utils::pullimage( image );
         if (rslt==kRError)
            logmsg(kLERROR,"Couldn't pull "+image ,p);
         if (rslt==kRNoChange)
         {
            if (utils::imageisbranch(image))
               logmsg(kLDEBUG,"No change to Docker image (it's not on the master branch, so assuming dev environment).",p);
            else
               logmsg(kLDEBUG,"No change to Docker image (it's already up to date).",p);
         } else
            logmsg(kLINFO,"Successfully pulled "+image ,p);
      }
   }

   int setup(const params & p)
   {
      if (p.getArgs().size()<1)
         logmsg(kLERROR,"Usage:\n   drunner setup ROOTPATH",p);

      // -----------------------------------------------------------------------------
      // determine rootpath.
      std::string rootpath = utils::getabsolutepath(p.getArgs()[0]);
      if (rootpath.length()==0)
         logmsg(kLERROR,"Couldn't determine path for "+p.getArgs()[0],p);

      // -----------------------------------------------------------------------------
      // create rootpath if it doesn't exist.
      utils::makedirectory(rootpath,p,S_700);

      // -----------------------------------------------------------------------------
      // now that rootpath is created we can get concrete path to it.
      rootpath = utils::getcanonicalpath(rootpath);

      // -----------------------------------------------------------------------------
      // create the settings and write to config.sh
      sh_drunnercfg settings(p,rootpath);
      if (!settings.writeSettings())
         logmsg(kLERROR,"Couldn't write settings file!",p.getLogLevel());

      // -----------------------------------------------------------------------------
      // move this executable to the directory.
      int result = rename( utils::get_exefullpath().c_str(), (rootpath+"/drunner").c_str());
      if (result!=0)
         logmsg(kLERROR,"Couldn't move drunner executable to "+rootpath+".",p);

      // -----------------------------------------------------------------------------
      // create bin directory
      std::string bindir = utils::get_usersbindir();
      utils::makedirectory( bindir, p, S_700 );

      // -----------------------------------------------------------------------------
      // create symlink
	  utils::makesymlink(rootpath + "/drunner", bindir + "/drunner",p);

      // get latest root util image.
      //std::cerr << "ROOTUITILIMAGE = " << settings.getRootUtilImage() << std::endl;
      pullImage(p,settings,settings.getRootUtilImage());

      // -----------------------------------------------------------------------------
      // create services and support directories
      utils::makedirectory(settings.getPath_Services(),p,S_700);
      utils::makedirectory(settings.getPath_Support(),p,S_ALLEXEC);
      generate_validator_image(settings.getPath_Support(),p);

      // -----------------------------------------------------------------------------
      // Finished!
      if (settings.readFromFileOkay())
         logmsg(kLINFO,"Update of drunner to "+p.getVersion()+" completed succesfully.",p);
      else
         logmsg(kLINFO,"Setup of drunner "+p.getVersion()+" completed succesfully.",p);

      return 0;
   }

   int update(const params & p , const sh_drunnercfg & s)
   {
      logmsg(kLDEBUG,"Updating dRunner in "+s.getPath_Root(),p);

      std::string op,url( s.getdrunnerInstallURL() ),trgt( s.getPath_Root() + "/drunner-install" );
      int rval = utils::bashcommand("wget --no-cache -nv -O "+trgt+" "+url+" 2>&1 && chmod 0755 "+trgt, op);
         logmsg(kLDEBUG,op,p);
      if (rval!=0)
         logmsg(kLERROR,"Unable to download updated drunner-install",p);

      logmsg(kLINFO,"Updating...",p);

      std::ostringstream oss;
      oss << trgt.c_str() << " " << p.getLogLevelOption().c_str() << " "
            << "setup" << " " << s.getPath_Root().c_str();
      logmsg(kLDEBUG,oss.str(),p);

      execl(
         trgt.c_str(),
         "drunner-install",
         p.getLogLevelOption().c_str(),
         "setup",
         s.getPath_Root().c_str(),
         (char *)0
      );
      logmsg(kLERROR,"Exec failed.");
      return 1;
   }

}
