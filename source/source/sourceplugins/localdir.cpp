#include "localdir.h"

#include "Poco/DirectoryIterator.h"

namespace sourceplugins
{
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

   cResult localdir::install(std::string & imagename, const servicePaths & sp)
   {
      Poco::Path dest = sp.getPathdService();
      static const std::string local = "local|";

      Poco::Path path;
      if (imagename.compare(".") == 0)
      {
         path = Poco::Path::current();
         imagename = local + path.toString();
      }
      else
      {
         if (imagename.find(local) != 0)
            return kRNoChange;
         path = imagename.substr(local.length());
      }

      logmsg(kLDEBUG, "Installing from local directory.");
      return copydServiceFiles(path, dest);
   }
}