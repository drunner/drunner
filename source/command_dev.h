#ifndef __COMMAND_DEV_H
#define __COMMAND_DEV_H

#include "params.h"
#include "sh_drunnercfg.h"

namespace command_dev
{
   void build(const params & p, const sh_drunnercfg & settings,const std::string & thedir="");

}


#endif
