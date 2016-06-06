#ifndef __DBACKUP_H
#define __DBACKUP_H

#include <string>
#include "enums.h"

namespace dbackup
{
   eResult include(std::string servicename);
   eResult exclude(std::string servicename);
   eResult run();
   eResult info();
   eResult configure(std::string path);
}


#endif
