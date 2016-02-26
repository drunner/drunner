#include "params.h"
#include "drunner_settings.h"

#ifndef __COMMAND_DEV_H
#define __COMMAND_DEV_H


namespace command_dev
{
   void build(const params & p, const drunner_settings & settings,const std::string & thedir="");

}


#endif
