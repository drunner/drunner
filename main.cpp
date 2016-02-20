#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>

#include "utils.h"
#include "params.h"
#include "command_setup.h"
#include "main.h"

//  sudo apt-get install build-essential g++-multilib libboost-all-dev

using namespace utils;

int main(int argc, char **argv)
{
   check_basics();

   params p(argc, argv);

   if (p.mOMode != om_silent)
	  std::cout << "dRunner C++, version " << p.mVersion << std::endl;
   if (p.mOMode == om_verbose)
   {
      std::string u=getUSER();
      std::cout << "Username: "+u+",  Docker OK: "+std::string(utils::canrundocker(u) ? "YES" : "NO")<< std::endl;
   }

   switch (p.mCmd)
   {
      case c_setup:
         exit( command_setup(p) );

      default:
         std::cerr << "\e[31m" <<  R"EOF(

          /-------------------------------------------------------------\
          |   That command has not been implemented and I am sad. :,(   |
          \-------------------------------------------------------------/
)EOF" << "\e[0m";
         exit(1);
   }
}

void check_basics()
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
