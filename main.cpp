#include <stdio.h>
#include <iostream>

#include "utils.h"
#include "params.h"

//  sudo apt-get install build-essential g++-multilib

using namespace utils;

int main(int argc, char **argv)
{
   params p(argc, argv);

   if (p.mOMode != om_silent)
	  std::cout << "dRunner C, version " << p.mVersion << std::endl;

   if (p.mOMode == om_verbose)
   std::cout << R"EOF(
      dRunner is a ()!@# wow.
)EOF";
}
