#include "params.h"
#include "drunner_settings.h"

#ifndef __COMMAND_INSTALL_H
#define __COMMAND_INSTALL_H


namespace command_install
{
   void validateImage(const params & p,const drunner_settings & settings, std::string imagename);
   void installService(const params & p,const drunner_settings & settings,
      const std::string & imagename, std::string servicename);

}


#endif
