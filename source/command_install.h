#ifndef __COMMAND_INSTALL_H
#define __COMMAND_INSTALL_H

#include "params.h"
#include "sh_drunnercfg.h"
#include "service.h"

namespace command_install
{
   void validateImage(const params & p,const sh_drunnercfg & settings, std::string imagename);
   void recreateService(const params & p, const sh_drunnercfg & settings, const std::string & imagename, const service & svc, bool updating);
   void installService(const params & p,const sh_drunnercfg & settings,
      const std::string & imagename, service & svc);

}


#endif
