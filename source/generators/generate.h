#ifndef __GENERATE_H
#define __GENERATE_H

#include "enums.h"
#include "params.h"

void generate(const std::string & fullpath, const params & p, const std::string & content)
{
   std::string op;
   std::ofstream ofs;
   ofs.open(fullpath);
   if (!ofs.isopen())
      logmsg(kLERROR,"Couldn't write to "+fullpath,p);
   ofs << content;
   ofs.close();
   if (utils::bashcommand("chmod a+r "+fullpath,op) != 0)
      logmsg(kLERROR, "Unable to change permissions on "+fullpath,p);
   logmsg(kLDEBUG,"Created "+fullpath,p);
}

#endif
