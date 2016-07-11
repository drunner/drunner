#ifndef __GLOBALCONTEXT_H
#define __GLOBALCONTEXT_H

#include <string>
#include <memory>

#include "enums.h"
#include "params.h"
#include "drunner_settings.h"
#include "plugins.h"


class GlobalContext
{
public:
   static std::shared_ptr<const params> getParams();
   static std::shared_ptr<const drunnerSettings> getSettings();
   static std::shared_ptr<const plugins> getPlugins();

   static bool hasParams();
   static bool hasSettings();
   static bool hasPlugins();

   static void init(int argc, char **argv);

   GlobalContext();

   static std::shared_ptr<const params> s_params;
   static std::shared_ptr<const drunnerSettings> s_settings;
   static std::shared_ptr<const plugins> s_plugins;
};

#endif
