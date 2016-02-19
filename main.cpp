#include <stdio.h>
#include <iostream>

#include "utils.h"

//  sudo apt-get install build-essential g++-multilib

using namespace utils;

int main()
{
	std::cout << "dRunner C, version " << getVersion() << std::endl;
}
