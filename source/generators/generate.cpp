#include "generate.h"
#include "logmsg.h"
#include "utils.h"

void generate(const std::string & fullpath, const params & p, const std::string & content)
{
   std::string op;
   std::ofstream ofs;
   ofs.open(fullpath);
   if (!ofs.is_open())
      logmsg(kLERROR,"Couldn't write to "+fullpath,p);
   ofs << content;
   ofs.close();
   if (utils::bashcommand("chmod a+r "+fullpath,op) != 0)
      logmsg(kLERROR, "Unable to change permissions on "+fullpath,p);
   logmsg(kLDEBUG,"Created "+fullpath,p);
}
