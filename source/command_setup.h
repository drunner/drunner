#ifndef __SETUP_H
#define __SETUP_H

#include "params.h"
#include "drunner_settings.h"

namespace command_setup
{

   int setup(const params & p);
   int update(const params & p , const drunner_settings & s);

}
#endif
