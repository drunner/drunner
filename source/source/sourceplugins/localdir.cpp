#include "localdir.h"

#include "Poco/DirectoryIterator.h"
#include "dassert.h"

namespace sourceplugins
{

   static const std::string localstr = "local:";


   // copy files including service.lua.
   cResult localdir::copydServiceFiles(Poco::Path pfrom, Poco::Path pto)
   {
      Poco::Path servicelua = pfrom;
      servicelua.setFileName("service.lua");
      if (!utils::fileexists(servicelua))
         return cError("Couldn't find " + servicelua.toString() + "\nIs this a dService?!");

      try
      {
         Poco::DirectoryIterator it(pfrom);
         Poco::DirectoryIterator end;
         while (it != end)
         {
            if (it->isFile())
            {
               logmsg(kLDEBUG, "Installing " + it.name());
               Poco::File(it.path()).copyTo(pto.toString());
            }
            ++it;
         }
      }
      catch (const Poco::Exception & e) {
         return cError(std::string("Couldn't copy files - ") + e.what());
      }

      return kRSuccess;
   }

   // ----------------------------

   cResult localdir::install(std::string imagename, const servicePaths & sp)
   {
      Poco::Path dest = sp.getPathdService();
      Poco::Path path(imagename.substr(localstr.length()));

      logmsg(kLINFO, "Installing from local directory "+path.toString());
      return copydServiceFiles(path, dest);
   }
   bool localdir::pluginmatch(std::string imagename)
   {
      if (imagename.compare(".") == 0)
         return true;

      if (imagename.find(localstr) == 0)
         return true;

      return false;
   }
   cResult localdir::normaliseNames(std::string & imagename, std::string & servicename)
   {
      if (imagename.compare(".") == 0)
         imagename = localstr + Poco::Path::current();
     
      drunner_assert(imagename.find(localstr) == 0, "Malformed imagename in localdir plugin");

      if (servicename.length() > 0)
         return kRSuccess;

      // need to determine imagename from path.
      std::string path = imagename;
      path.erase(0, localstr.length());
      Poco::Path p(path);
      drunner_assert(p.depth() >= 1, "Current directory is root. No.");
      servicename = p.directory(p.depth() - 1);
      return kRSuccess;
   }
}