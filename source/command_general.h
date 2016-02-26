#include "params.h"
#include "drunner_settings.h"

#ifndef __COMMAND_GENERAL_H
#define __COMMAND_GENERAL_H


namespace command_general
{
   void showservices(const params & p, const drunner_settings & settings);
   void clean(const params & p, const drunner_settings & settings);
   void build(const params & p, const drunner_settings & settings);
   void validateImage(const params & p,const drunner_settings & settings, std::string imagename);
   void installService(const params & p,const drunner_settings & settings,
      const std::string & imagename, std::string servicename);
}

#endif
