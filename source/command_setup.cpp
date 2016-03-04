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
      std::string ROOTUTILIMAGE="drunner/install-rootutils";
      std::string DRUNNERINSTALLURL="https://raw.githubusercontent.com/drunner/install/master/drunner-install";

      if (p.getArgs().size()<1)
         logmsg(kLERROR,"Usage:\n   drunner setup ROOTPATH",p);

      // -----------------------------------------------------------------------------
      // determine rootpath.
      std::string rootpath = utils::getabsolutepath(p.getArgs()[0]);
      if (rootpath.length()==0)
         logmsg(kLERROR,"Couldn't determine path for "+p.getArgs()[0],p);

      // -----------------------------------------------------------------------------
      // create rootpath if it doesn't exist.
      utils::makedirectory(rootpath,p);
      utils::makedirectory(rootpath+"/temp/services",p);
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
      if (!boost::filesystem::exists(bindir))
      {
         logmsg(kLINFO,"Creating "+bindir,p.getLogLevel());
         if (utils::mkdirp(bindir)==kRError)
            logmsg(kLERROR,"Couldn't create ~/bin.",p);
         if (!utils::fileexists(bindir))
            logmsg(kLERROR,"Failed to create ~/bin.",p);
      } else
         logmsg(kLDEBUG,bindir+" exists and was left unchanged.",p);

      // -----------------------------------------------------------------------------
      // create symlink
      std::string symtarget=bindir+"/drunner";
      if (utils::fileexists(symtarget))
         if (remove(symtarget.c_str())!=0)
            logmsg(kLERROR,"Couldn't remove stale symlink at "+symtarget,p);
      std::string cmd = "ln -s " + rootpath + "/drunner" + " " + bindir + "/drunner";
      std::string op;
      if ( utils::bashcommand(cmd,op) != 0 )
         logmsg(kLERROR,"Failed to create symbolic link for drunner.",p);

      // get latest root util image.
      pullImage(p,settings,settings.getRootUtilImage());

      // -----------------------------------------------------------------------------
      // create services and support directories
      utils::makedirectory(settings.getPath_Services(),p);
      utils::makedirectory(settings.getPath_Support(),p);
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
