#ifndef __PLUGINS_H
#define __PLUGINS_H

#include <deque>
#include <vector>
#include <string>
#include <memory>

#include "cresult.h"

class plugin
{
public:
   virtual std::string getName() const = 0;
   virtual cResult runCommand() const = 0;
};


class plugins
{
public:
   plugins();

   void generate_plugin_scripts() const;
   cResult runcommand() const;

private:
   std::deque< std::unique_ptr<plugin> > mPlugins;
};






#endif
