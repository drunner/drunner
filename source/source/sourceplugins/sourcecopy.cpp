#include "Poco/String.h"

#include "sourcecopy.h"
#include "registries.h"
#include "globalcontext.h"
#include "registry.h"

namespace sourcecopy
{

   // -----------------------------------------------------------------------------
   // Install the imagename.
   cResult install(std::string imagename, const servicePaths & sp)
   {
      registries regall;
      registrydefinition regdef = regall.get(imagename);
      
      registry r(regdef.mURL);


      return kRNotImplemented;
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