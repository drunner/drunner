#ifndef __SERVICE_DSERVICEDEFINSTALL_H
#define __SERVICE_DSERVICEDEFINSTALL_H

#include <string>
#include "service_paths.h"

namespace sddi
{
   cResult copy_from_container(std::string imagename, const servicePaths & sp);
}


#endif