#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sstream>

#include "exceptions.h"
#include "utils.h"
#include "params.h"
#include "command_setup.h"
#include "main.h"

//  sudo apt-get install build-essential g++-multilib libboost-all-dev

using namespace utils;

int main(int argc, char **argv)
{
   try 
   {
      mainroutines::check_basics();

      params::params p(argc, argv);

      if (! p.drIsSilent())
      std::cout << "dRunner C++, version " << p.getVersion() << std::endl;
      if (p.isVerbose())
      {
         std::string u=getUSER();
         std::cout << "Username: "+u+",  Docker OK: "+std::string(utils::canrundocker(u) ? "YES" : "NO")<< std::endl;
      }

      mainroutines::process(p);
   }
   
   catch (const eExit & e) {
      if (e.hasMsg()) 
         std::cerr << e.what();
      return e.exitCode();
   }
}

void mainroutines::check_basics()
{
   uid_t euid=geteuid();
   if (euid == 0)
      utils::die("Please run as a standard user, not as root.");

   std::string user=utils::getUSER();
   if (!utils::isindockergroup(user))
      utils::die("Please add the current user to the docker group. As root: "+kCODE_S+"adduser "+user+" docker"+kCODE_E);
   if (!utils::canrundocker(user))
      utils::die(user+" hasn't picked up group docker yet. Log out then in again, or run "+kCODE_S+"exec su -l "+user+kCODE_E);

   if (!utils::commandexists("docker"))
      utils::die("Please install Docker before using dRunner.\n(e.g. use  https://raw.githubusercontent.com/j842/scripts/master/install_docker.sh )");

   std::string v;
   if (utils::bashcommand("docker --version",v)!=0)
      utils::die("Running \"docker --version\" failed! Is docker correctly installed on this machine?");
   //std::cerr << v << std::endl;
}

void mainroutines::process(const params::params & p)
{
  
   switch (p.getCommand())
   {
      case params::c_setup:
         {
            int rval=command_setup(p);
            if (rval!=0) throw eExit("Setup failed.",rval);
            if (!p.drIsSilent())
               std::cout << "Setup completed succesfully." << std::endl;
            break;
         }
         
      default:
         {
            std::ostringstream msg;
            msg << "\e[31m" <<  R"EOF(

          /-------------------------------------------------------------\
          |   That command has not been implemented and I am sad. :,(   |
          \-------------------------------------------------------------/
)EOF" << "\e[0m";
            throw eExit(msg.str().c_str());
         }
   } 
}
