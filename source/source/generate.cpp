#include <fstream>
#include <iostream>
#include <cstring>

#include <Poco/File.h>
#include <Poco/TemporaryFile.h>

#include "generate.h"
#include "globallogger.h"
#include "utils.h"
#include "chmod.h"

std::ifstream::pos_type _getsize(std::ifstream & in0)
{
   std::ifstream::pos_type size0;
   size0 = in0.seekg(0, std::ifstream::end).tellg();
   in0.seekg(0, std::ifstream::beg);
   return size0;
}

// check if two files are the same.
bool filesame(std::string fn1, std::string fn2)
{
   std::ifstream in1(fn1.c_str(), std::ifstream::in | std::ifstream::binary);
   std::ifstream in2(fn2.c_str(), std::ifstream::in | std::ifstream::binary);

   std::ifstream::pos_type size1 = _getsize(in1);
   std::ifstream::pos_type size2 = _getsize(in2);
   if (size1 != size2)
      return false;

   static const size_t BLOCKSIZE = 4096;
   size_t remaining = (size_t)size1;
   char buffer1[BLOCKSIZE], buffer2[BLOCKSIZE];

   while (remaining)
   {
      size_t size = BLOCKSIZE < remaining ? BLOCKSIZE : remaining;
      
      in1.read(buffer1, size);
      in2.read(buffer2, size);

      if (0 != std::memcmp(buffer1, buffer2, size))
         return false;

      remaining -= size;
   }

   return true;
}

void _generate(std::string tfile, const std::string & content)
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
   std::string tfile = Poco::TemporaryFile::tempName();

   // for now we force the generation. In future should check
   // whether any changes have been made.
   poco_assert(fullpath.isFile());

   _generate(tfile,content);

   Poco::File f(fullpath);
   if (f.exists())
   { // is it the same?
      if (filesame(tfile, fullpath.toString()))
      {
         Poco::File(tfile).remove();
         return kRNoChange;
      }

      logmsg(kLDEBUG, "Updating " + fullpath.toString());
      if (kRSuccess != utils::delfile(fullpath.toString()))
         logmsg(kLERROR, "Unable to delete file " + fullpath.toString());
   }
   else
      logmsg(kLDEBUG, "Creating " + fullpath.toString());

   // delete the temp file.
   Poco::File(tfile).remove();

   // we re-generate rather than move the temp file, as it may be across filesystems (which won't work for C++ move).
   _generate(fullpath.toString(), content);

   if (xchmod(fullpath.toString().c_str(), mode) != 0)
      logmsg(kLERROR, "Unable to change permissions on " + fullpath.toString());

   return kRSuccess;
}
