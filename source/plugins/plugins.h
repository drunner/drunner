#ifndef __PLUGINS_H
#define __PLUGINS_H

#include <deque>
#include <vector>
#include <string>
#include <memory>

#include "enums.h"

class plugin
{
public:
   plugin();

   virtual std::string getName() const = 0;
   virtual eResult runCommand() = 0;
};


class plugins
{
public:
   plugins();

   void generate_plugin_scripts() const;
   eResult runcommand() const;

private:
   std::deque< std::shared_ptr <plugin> > mPlugins;
};



#endif
