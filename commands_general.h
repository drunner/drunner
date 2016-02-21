#include "params.h"
#include "drunner_settings.h"

#ifndef __COMMAND_GENERAL_H
#define __COMMAND_GENERAL_H


namespace commands_general
{
   void showservices(const params & p, const drunner_settings settings);
   void clean(const params & p, const drunner_settings settings);
   void update(const params & p, const drunner_settings settings);
}

#endif
