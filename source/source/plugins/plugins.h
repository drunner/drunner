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
   virtual std::string getName() const = 0;
   virtual eResult runCommand() const = 0;
};


class plugins
{
public:
   plugins();

   void generate_plugin_scripts() const;
   eResult runcommand() const;

private:
   std::deque< std::unique_ptr<plugin> > mPlugins;
};


constexpr unsigned int str2int(const char* str, int h=0)
{
   return !str[h] ? 5381 : (str2int(str, h + 1) * 33) ^ str[h];
}



#endif
