#include "Poco/String.h"
#include "Poco/File.h"

#include "sourcecopy.h"
#include "registries.h"
#include "globalcontext.h"
#include "buildnum.h"
#include "gitcache.h"

namespace sourcecopy
{



   // -----------------------------------------------------------------------------
   // Install the imagename.
   cResult install(std::string imagename, const servicePaths & sp)
   {
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

      Poco::Path target = sp.getPathdService();

      // try drunner10 subfolder
      Poco::Path drunner10(p.toString() + "drunner" + getVersionNice());
      drunner10.makeDirectory(); // ensures the path is treated as a directory (does not create a directory on disk!!)
      if (Poco::File(drunner10.toString()+"service.lua").exists())
         return gc.recursiveCopyContents(drunner10, target);

      // try drunner subfolder
      Poco::Path drunner(p.toString() + "drunner");
      drunner.makeDirectory();
      if (Poco::File(drunner.toString() + "service.lua").exists())
         return gc.recursiveCopyContents(drunner, target);

      // just copy whole repo
      if (Poco::File(p.toString()+"service.lua").exists())
         return gc.recursiveCopyContents(p, target);

      return cError("Unable to locate service.lua in git repo for " + imagename);
   }

   cResult normaliseNames(std::string & imagename, std::string & servicename)
   {
      std::string registry, repo, tag;
      cResult r = registries::splitImageName(imagename, registry, repo, tag);
      if (!r.success())
         return r;

      imagename = registry + "/" + repo + ":" + tag;
      if (servicename.length() == 0)
         servicename = repo;

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