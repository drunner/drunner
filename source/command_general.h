#include "params.h"
#include "sh_drunnercfg.h"

#ifndef __COMMAND_GENERAL_H
#define __COMMAND_GENERAL_H


namespace command_general
{
   void showservices(const params & p, const sh_drunnercfg & settings);
   void clean(const params & p, const sh_drunnercfg & settings);
   void service(const params & p, const sh_drunnercfg & settings);
}

#endif
