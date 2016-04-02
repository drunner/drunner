#ifndef __COMMAND_SETUP_H
#define __COMMAND_SETUP_H

#include "params.h"
#include "settings/sh_drunnercfg.h"

namespace command_setup
{
   void pullImage(const params & p,const sh_drunnercfg & s, const std::string & image);
   int setup(const params & p);
   int update(const params & p , const sh_drunnercfg & s);

}
#endif
