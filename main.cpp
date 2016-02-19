#include <stdio.h>
#include <iostream>

#include "shellutils.h"

//  sudo apt-get install build-essential g++-multilib

int main()
{
	std::cout << "dRunner C, version " << utils::getVersion() << std::endl;
}
