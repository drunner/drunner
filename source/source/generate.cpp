#include <fstream>
#include <iostream>
#include <string>

#include <Poco/File.h>
#include <Poco/TemporaryFile.h>

#include "generate.h"
#include "globallogger.h"
#include "utils.h"

// check if two files are the same.
bool filesameasstring(std::string filename, const std::string & thestring)
{
   bool rval = false;

   std::ifstream t(filename.c_str());
   std::stringstream buffer;
   buffer << t.rdbuf();

   rval = (thestring.compare(buffer.str()) == 0);

   t.close();
   return rval;
}

void generate_low(std::string tfile, const std::string & content)
{
   std::string op;
   std::ofstream ofs;
   ofs.open(tfile);
   if (!ofs.is_open())
      logmsg(kLERROR, "Couldn't write to " + tfile);
   ofs << content;
   ofs.close();
}

cResult generate(
   Poco::Path fullpath,
   const mode_t mode,
   const std::string & content
)
{
   // for now we force the generation. In future should check
   // whether any changes have been made.
   poco_assert(fullpath.isFile());

   if (filesameasstring(fullpath.toString(), content))
   {
      logdbg("File " + fullpath.toString() + " unchanged.");
      return kRNoChange;
   }

   if (utils::fileexists(fullpath))
   {
      logmsg(kLDEBUG, "Updating " + fullpath.toString());
      cResult r = utils::delfile(fullpath.toString());
      if (!r.success())
         logmsg(kLERROR, "Unable to delete file " + fullpath.toString()+"\n"+r.what());
   }
   else
      logmsg(kLDEBUG, "Creating " + fullpath.toString());

   // we re-generate rather than move the temp file, as it may be across filesystems (which won't work for C++ move).
   generate_low(fullpath.toString(), content);

#ifndef _WIN32
   if (chmod(fullpath.toString().c_str(), mode) != 0)
      logmsg(kLERROR, "Unable to change permissions on " + fullpath.toString());
#endif

   return kRSuccess;
}
