#include "params.h"
#include "sh_drunnercfg.h"

#ifndef __COMMAND_INSTALL_H
#define __COMMAND_INSTALL_H


namespace command_install
{
   void validateImage(const params & p,const sh_drunnercfg & settings, std::string imagename);
   void installService(const params & p,const sh_drunnercfg & settings,
      const std::string & imagename, std::string servicename);

}


#endif
