#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>

#include "utils.h"
#include "params.h"
#include "setup.h"

//  sudo apt-get install build-essential g++-multilib

using namespace utils;

int main(int argc, char **argv)
{
   uid_t euid=geteuid();
   if (euid == 0)
      utils::dielow("Please run as a standard user, not as root.");

   params p(argc, argv);

   if (p.mOMode != om_silent)
	  std::cout << "dRunner C, version " << p.mVersion << std::endl;

   switch (p.mCmd)
   {
      case c_setup:
         exit( setup(p) );

      default:
         std::cerr << "\e[31m" <<  R"EOF(

          /-------------------------------------------------------------\
          |   That command has not been implemented and I am sad. :,(   |
          \-------------------------------------------------------------/
)EOF" << "\e[0m";
         exit(1);
   }
}
