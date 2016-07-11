#include <Poco/File.h>

#include "generate.h"
#include "globallogger.h"
#include "utils.h"
#include "chmod.h"

void generate(
   Poco::Path fullpath,
   const mode_t mode,
   const std::string & content
)
{
   // for now we force the generation. In future should check
   // whether any changes have been made.
   poco_assert(fullpath.isFile());
   Poco::File f(fullpath);
   if (f.exists())
      utils::delfile(fullpath.toString());

   std::string op;
   std::ofstream ofs;
   ofs.open(fullpath.toString());
   if (!ofs.is_open())
      logmsg(kLERROR,"Couldn't write to "+fullpath.toString());
   ofs << content;
   ofs.close();

   if (xchmod(fullpath.toString().c_str(), mode)!=0)
      logmsg(kLERROR, "Unable to change permissions on "+fullpath.toString());
   logmsg(kLDEBUG,"Created "+fullpath.toString());
}
