#ifndef __COMMAND_SETUP_H
#define __COMMAND_SETUP_H

#include "params.h"
#include "drunnerSettings.h"
#include "cresult.h"

namespace drunner_setup
{
   cResult check_setup(bool forceUpdate);
   int update();

}
#endif
