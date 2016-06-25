#ifndef __DBACKUP_H
#define __DBACKUP_H

#include <string>
#include "enums.h"
#include "plugins.h"

class dbackup : public plugin
{
public:
   dbackup();
   virtual std::string getName() const;
   virtual eResult runCommand() const;

private:
   eResult include(std::string servicename) const;
   eResult exclude(std::string servicename) const;
   eResult run() const;
   eResult info() const;
   eResult configure(std::string path) const;

   eResult showhelp() const;
};

#endif
