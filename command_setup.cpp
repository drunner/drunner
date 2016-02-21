#include <boost/filesystem.hpp>

#include "command_setup.h"
#include "utils.h"
#include "drunner_settings.h"
#include "logmsg.h"

int command_setup(const params::params & p)
{
   //std::string SUPPORTIMAGE="drunner/install-support";
   std::string ROOTUTILIMAGE="drunner/install-rootutils";
   std::string DRUNNERINSTALLURL="https://raw.githubusercontent.com/drunner/install/master/drunner-install";

   if (p.getArgs().size()<1)
      logmsg(kLERROR,"Usage:\n   drunner setup ROOTPATH",p);

   std::string rootpath = utils::getabsolutepath(p.getArgs()[0]);
   if (rootpath.length()==0)
      logmsg(kLERROR,"Couldn't determine path for "+p.getArgs()[0],p);
   logmsg(kLINFO,"Setting up to directory "+rootpath,p.getLogLevel());

   if (utils::mkdirp(rootpath)==kRError)
      logmsg(kLERROR,"Couldn't create directory "+rootpath,p);

   rootpath = utils::getcanonicalpath(rootpath);

   // create the settings and write to config.sh
   drunner_settings settings(rootpath);
   if (!settings.writeSettings())
      logmsg(kLERROR,"Couldn't write settings file!",p.getLogLevel());
   
   // move this executable to the directory.
   int result = rename( utils::get_exefullpath().c_str(), (rootpath+"/drunner").c_str());
   if (result!=0)
      logmsg(kLERROR,"Couldn't move drunner executable to "+rootpath+".",p);
   
   // create bin directory
   std::string bindir = utils::get_usersbindir();
   logmsg(kLINFO,"Creating "+bindir,p.getLogLevel());
   if (utils::mkdirp(bindir)==kRError)
      logmsg(kLERROR,"Couldn't create ~/bin.",p);
   if (!utils::fileexists(bindir))
      logmsg(kLERROR,"Failed to create ~/bin.",p);
   
   // create symlink
   std::string symtarget=bindir+"/drunner";
   if (utils::fileexists(symtarget))
      if (remove(symtarget.c_str())!=0)
         logmsg(kLERROR,"Couldn't remove stale symlink at "+symtarget,p);
   std::string cmd = "ln -s " + rootpath + "/drunner" + " " + bindir + "/drunner";
   std::string op;
   if ( utils::bashcommand(cmd,op) != 0 )
      logmsg(kLERROR,"Failed to create symbolic link for drunner.",p);
   
   // pull the rootutils image to ensure we have the latest.
   logmsg(kLINFO,"Pulling Docker image " + settings.getRootUtilImage(),p);
   eResult rslt = utils::pullimage( settings.getRootUtilImage() );
   if (rslt==kRError) 
      logmsg(kLERROR,"Couldn't pull "+settings.getRootUtilImage() ,p);
      
   if (rslt==kRNoChange)
   {
      if (utils::imageisbranch(settings.getRootUtilImage()))
         logmsg(kLINFO,"No change to Docker image (it's not on the master branch, so assuming dev environment).",p);
      else
         logmsg(kLINFO,"No change to Docker image (it's already up to date).",p);
   }      

   if (settings.readFromFileOkay())
      logmsg(kLINFO,"Update of drunner to "+p.getVersion()+" completed succesfully.",p); 
   else
      logmsg(kLINFO,"Setup of drunner "+p.getVersion()+" completed succesfully.",p);
      
   return 0;
}
