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


//
//commandlist["__dbackup_include"] = c_dbackup_include;
//commandlist["__dbackup_exclude"] = c_dbackup_exclude;
//commandlist["__dbackup_run"] = c_dbackup_run;
//commandlist["__dbackup_configure"] = c_dbackup_configure;
//commandlist["__dbackup_info"] = c_dbackup_info;


//case c_dbackup_configure:
//{
//   if (p.numArgs() < 1)
//      logmsg(kLERROR, "Usage: dbackup configure BACKUPPATH");
//   return (int)dbackup::configure(p.getArg(0));
//}

//case c_dbackup_exclude:
//{
//   if (p.numArgs() < 1)
//      logmsg(kLERROR, "Usage: dbackup exclude SERVICENAME");
//   return (int)dbackup::exclude(p.getArg(0));
//}

//case c_dbackup_include:
//{
//   if (p.numArgs() < 1)
//      logmsg(kLERROR, "Usage: dbackup include SERVICENAME");
//   return (int)dbackup::include(p.getArg(0));
//}

//case c_dbackup_run:
//   return (int)dbackup::run();

//case c_dbackup_info:
//   return (int)dbackup::info();


#endif
