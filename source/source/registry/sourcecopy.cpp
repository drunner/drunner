#include "Poco/String.h"
#include "Poco/File.h"

#include "sourcecopy.h"
#include "registries.h"
#include "globalcontext.h"
#include "buildnum.h"
#include "gitcache.h"
#include "dassert.h"

namespace sourcecopy
{
   cResult getServiceLuaParent(Poco::Path & p)
   {
      // try drunner10 subfolder
      Poco::Path drunner10(p.toString() + "drunner" + getVersionNice());
      drunner10.makeDirectory(); // ensures the path is treated as a directory (does not create a directory on disk!!)
      if (Poco::File(drunner10.toString() + "service.lua").exists())
      {
         p = drunner10;
         return kRSuccess;
      }

      // try drunner subfolder
      Poco::Path drunner(p.toString() + "drunner");
      drunner.makeDirectory();
      if (Poco::File(drunner.toString() + "service.lua").exists())
      {
         p = drunner;
         return kRSuccess;
      }

      if (Poco::File(p.toString() + "service.lua").exists())
         return kRSuccess;

      return cError("Could not locate the service.lua file.");
   }



   cResult _installfrom(Poco::Path p, const servicePaths & sp)
   {
      Poco::Path target = sp.getPathdService();

      if (getServiceLuaParent(p).success())
         return gitcache::recursiveCopyContents(p, target);

      return cError("Unable to locate service.lua at " + p.toString());
   }


   // -----------------------------------------------------------------------------
   // Install the imagename.
   cResult install(std::string imagename, const servicePaths & sp)
   {
      std::string localstr = "local:";
      if (Poco::icompare(imagename.substr(0, localstr.length()), localstr) == 0)
      {
         imagename.erase(0, localstr.length());
         drunner_assert(imagename.length() > 0, "Empty folder passed to install_local.");
         return _installfrom(imagename, sp);
      }

      registries regall;

      std::string registry, dService, tag;
      registries::splitImageName(imagename, registry, dService, tag);

      registrydefinition regdef = regall.get(registry);
      
      sourcecopy::registry r(regdef);
      sourcecopy::registryitem regitem;
      cResult getrslt = r.get(dService, regitem);
      if (!getrslt.success())
         return getrslt;

      Poco::Path p;
      gitcache gc(regitem.url, tag);
      cResult rslt = gc.get(p, true);
      if (!rslt.success())
         return rslt;

      return _installfrom(p,sp);
   }

   cResult normaliseNames(std::string & imagename, std::string & servicename)
   {
      if (Poco::icompare(imagename,".")==0)
      {
         imagename = "local:" + utils::getPWD();
         if (servicename.length() == 0)
         {
            Poco::Path p(Poco::Path::current());
            drunner_assert(p.isDirectory(), "coding error: not a directory");
            drunner_assert(p.depth() > 0, "Can't install from /");
            servicename = p.directory(p.depth() - 1);
         }         
      }
      else
      {
         std::string registry, repo, tag;
         cResult r = registries::splitImageName(imagename, registry, repo, tag);
         if (!r.success())
            return r;

         imagename = registry + "/" + repo + ":" + tag;
         if (servicename.length() == 0)
            servicename = repo;
      }

      return kRSuccess;
   }

   cResult registrycommand()
   {
      const params & p(*GlobalContext::getParams());

      registries r;
      if (p.numArgs()<1)
         return r.showregistries();

      if (Poco::icompare(p.getArg(0), "add")==0)
      {
         if (p.numArgs() != 4)
            fatal("drunner registry add NICENAME PROTOCOL URL");
         else
            return r.addregistry(p.getArg(1), p.getArg(2), p.getArg(3));
      }

      if (Poco::icompare(p.getArg(0), "del") == 0)
      {
         if (p.numArgs() != 2)
            fatal("drunner registry del NICENAME");
         else
            return r.delregistry(p.getArg(1));
      }

      return cError("Unknown registry command: " + p.getArg(0));
   }




} // namespace 