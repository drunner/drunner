#include <boost/filesystem.hpp>

#include "command_setup.h"
#include "utils.h"
#include "drunner_settings.h"
#include "logmsg.h"

namespace command_setup
{

   void pullImage(const params & p,std::string image)
   {
      // -----------------------------------------------------------------------------
      // pull the rootutils image to ensure we have the latest.
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
      if (!boost::filesystem::exists(rootpath))
      {
         logmsg(kLDEBUG,"Setting up to directory "+rootpath,p.getLogLevel());
         if (utils::mkdirp(rootpath)==kRError)
            logmsg(kLERROR,"Couldn't create directory "+rootpath,p);
      } else
         logmsg(kLDEBUG,"Rootpath "+rootpath+" already exists.",p);

      // -----------------------------------------------------------------------------
      // now that rootpath is created we can get concrete path to it.
      rootpath = utils::getcanonicalpath(rootpath);

      // -----------------------------------------------------------------------------
      // create the settings and write to config.sh
      drunner_settings settings(rootpath);
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
      pullImage(p,settings.getRootUtilImage());

      // -----------------------------------------------------------------------------
      // create services directory
      eResult rslt = utils::mkdirp(settings.getPath_Services());
      if (rslt==kRError)
         logmsg(kLERROR,"Couldn't create "+settings.getPath_Services(),p);
      if (rslt==kRSuccess)
         logmsg(kLINFO,"Created "+settings.getPath_Services(),p);
      if (rslt==kRNoChange)
         logmsg(kLDEBUG,"Services directory exists. Services left unchanged.",p);


      // -----------------------------------------------------------------------------
      // Finished!
      if (settings.readFromFileOkay())
         logmsg(kLINFO,"Update of drunner to "+p.getVersion()+" completed succesfully.",p);
      else
         logmsg(kLINFO,"Setup of drunner "+p.getVersion()+" completed succesfully.",p);

      return 0;
   }

   int update(const params & p , const drunner_settings & s)
   {
      logmsg(kLDEBUG,"Updating dRunner in "+s.getPath_Root(),p);

      std::string op,url( s.getdrunnerInstallURL() ),trgt( utils::get_exefullpath() );
      int rval = utils::bashcommand("wget --no-cache -nv -O "+trgt+" "+url+" && chmod 0755 "+trgt, op);

      if (rval!=0)
         logmsg(kLERROR,"Unable to download updated drunner-install",p);

      // get latest root util image.
      pullImage(p,s.getRootUtilImage());

      logmsg(kLINFO,"Update successful.");
      return 0;
   }

}
