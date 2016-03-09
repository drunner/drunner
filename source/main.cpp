#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sstream>

#include "exceptions.h"
#include "utils.h"
#include "logmsg.h"
#include "command_setup.h"
#include "command_general.h"
#include "command_dev.h"
#include "command_install.h"
#include "sh_drunnercfg.h"
#include "showhelp.h"
#include "main.h"
#include "unittests.h"

//  sudo apt-get install build-essential g++-multilib libboost-all-dev

using namespace utils;

// ----------------------------------------------------------------------------------------------------------------------


int main(int argc, char **argv)
{
   try
   {
      mainroutines::check_basics();

      params p(argc, argv);
      logmsg(kLDEBUG,"dRunner C++, version "+p.getVersion(),p);
      bool canRunDocker=utils::canrundocker(getUSER());
      logmsg(kLDEBUG,"Username: "+getUSER()+",  Docker OK: "+(canRunDocker ? "YES" : "NO")+", "+utils::get_exename()+" path: "+utils::get_exepath(), p);

      mainroutines::process(p);
   }

   catch (const eExit & e) {
      return e.exitCode();
   }
}

// ----------------------------------------------------------------------------------------------------------------------


void mainroutines::check_basics()
{
   uid_t euid=geteuid();
   if (euid == 0)
      logmsg(kLERROR,"Please run as a standard user, not as root.");

   std::string user=utils::getUSER();
   if (!utils::isindockergroup(user))
      logmsg(kLERROR,"Please add the current user to the docker group. As root: "+kCODE_S+"adduser "+user+" docker"+kCODE_E);

   if (!utils::canrundocker(user))
      logmsg(kLERROR,user+" hasn't picked up group docker yet. Log out then in again, or run "+kCODE_S+"exec su -l "+user+kCODE_E);

   if (!utils::commandexists("docker"))
      logmsg(kLERROR,"Please install Docker before using dRunner.\n(e.g. use  https://raw.githubusercontent.com/j842/scripts/master/install_docker.sh )");

   if (!utils::commandexists("wget"))
      logmsg(kLERROR,"Please install wget before using dRunner.");

   std::string v;
   if (utils::bashcommand("docker --version",v)!=0)
      logmsg(kLERROR,"Running \"docker --version\" failed! Is docker correctly installed on this machine?");
}

// ----------------------------------------------------------------------------------------------------------------------

void mainroutines::process(const params & p)
{
   // handle setup specially.
   if (p.getCommand()==c_setup)
   {
      int rval=command_setup::setup(p);
      if (rval!=0) throw eExit("Setup failed.",rval);
      return;
   }

   // allow unit tests to be run directly from installer.
   if (p.getCommand()==c_unittest)
   {
      // todo: pass args through to catch.
      // int result = Catch::Session().run( argc, argv );
      int result = UnitTest();
      if (result!=0)
         logmsg(kLERROR,"Unit tests failed.",p);
      logmsg(kLINFO,"All unit tests passed.",p);
      return;
   }

   if (!utils::isInstalled())
      showhelp(p,"Please run "+utils::get_exename()+" setup ROOTPATH");

   // load settings. We require the basic install to be okay at this point!
   std::string rootpath = utils::get_exepath();
   sh_drunnercfg settings(p,rootpath);
   if (!settings.readFromFileOkay())
      throw eExit("Couldn't read settings file. Try running drunner setup.",1);

   logmsg(kLDEBUG,"Settings read from "+settings.getPath(),p);


   // ----------------
   // command handling
   switch (p.getCommand())
   {
      case c_clean:
      {
         command_general::clean(p,settings);
         break;
      }

      case c_list:
      {
         command_general::showservices(p,settings);
         break;
      }

      case c_update:
      {
         if (p.getArgs().size()==0)
            command_setup::update(p,settings); // defined in command_setup
         else
            logmsg(kLERROR,"E_NOTIMPL",p);
         break;
      }

      case c_checkimage:
      {
         if (p.getArgs().size()==0)
            logmsg(kLERROR,"Usage: drunner checkimage IMAGENAME");
         command_install::validateImage(p,settings,p.getArgs()[0]);
         break;
      }

      case c_install:
      {
         if (p.numArgs()<1 || p.numArgs()>2)
            logmsg(kLERROR,"Usage: drunner install IMAGENAME [SERVICENAME]");
         std::string imagename = p.getArgs().at(0);
         std::string servicename;
         if ( p.getArgs().size()==2)
            servicename=p.getArgs()[1];
         command_install::installService(p,settings,imagename,servicename);
         break;
      }

      case c_build:
      {
         if (p.numArgs()<1)
            command_dev::build(p,settings);
         else
            command_dev::build(p,settings,p.getArgs()[0]);
         break;
      }

      case c_servicecmd:
      {
         if (p.numArgs() < 1)
            logmsg(kLERROR, "servicecmd should not be invoked manually.", p);

         std::string servicename = p.getArgs()[0];
         std::string reservedwords = "install backupstart backupend backup restore update enter uninstall obliterate";

         if (p.numArgs() < 2 || utils::stringisame(p.getArgs()[0], "help"))
            logmsg(kLERROR, "E_NOTIMPL", p); // todo: show service help!

         std::string command = p.getArgs()[1];
         if (utils::findStringIC(reservedwords, command))
            logmsg(kLERROR, command + " is a reserved word. You might want  drunner " + command + " " + servicename + "  instead.", p);

         //std::string op;
         bashcommand(settings.getPath_Services() + "/" + servicename + "/drunner/servicerunner " + command); // +" ")

         break;
      }

      default:
         {
            logmsg(kLERROR,R"EOF(

          /-------------------------------------------------------------\
          |   That command has not been implemented and I am sad. :,(   |
          \-------------------------------------------------------------/
)EOF",p);
         }
   }
}


// ----------------------------------------------------------------------------------------------------------------------
