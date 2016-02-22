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
#include "drunner_settings.h"
#include "main.h"

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
      logmsg(kLDEBUG,"Username: "+getUSER()+",  Docker OK: "+(canRunDocker ? "YES" : "NO")+", drunner path: "+utils::get_rootpath(), p);

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

   // load settings. We require the basic install to be okay at this point!
   std::string rootpath = utils::get_rootpath();
   drunner_settings settings(rootpath);
   if (!settings.readFromFileOkay())
      throw eExit("Couldn't read settings file. Try running drunner setup.",1);
      
   logmsg(kLDEBUG,"Settings read from "+rootpath+"/"+drunner_settings::getSettingsFileName(),p);
      
      
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
