#include "Poco/String.h"
#include "Poco/File.h"

#include "sourcecopy.h"
#include "registries.h"
#include "globalcontext.h"
#include "buildnum.h"

namespace sourcecopy
{



   // -----------------------------------------------------------------------------
   // Install the imagename.
   cResult install(std::string imagename, const servicePaths & sp)
   {
      registries regall;

      std::string registry, dService, tag;
      registries::splitImageName(imagename, registry, dService, tag);

      registrydefinition regdef = regall.get(registry,dService);
      
      sourcecopy::registry r(regdef);
      sourcecopy::registryitem regitem;
      r.get(regdef.mNiceName, regitem);

      Poco::Path temppath = sp.getPathdService();
      temppath.pushDirectory("temp_download");
      utils::tempfolder tempf(temppath);

      cResult rslt = CaptainCopy(regitem.getSourceInfo(tag), tempf.getpath(), kCM_Tree);
      if (!rslt.success())
         return rslt;

      Poco::Path target = sp.getPathdService();
      target.pushDirectory(dService);

      // try drunner10 subfolder
      {
         Poco::Path subf = tempf.getpath();
         subf.pushDirectory("drunner" + getVersionNice());
         Poco::File subf2(subf);
         if (subf2.exists())
         {
            subf2.copyTo(target.toString());
            return kRSuccess;
         }
      }

      // try drunner subfolder
      {
         Poco::Path subf = tempf.getpath();
         subf.pushDirectory("drunner");
         Poco::File subf2(subf);
         if (subf2.exists())
         {
            subf2.copyTo(target.toString());
            return kRSuccess;
         }
      }

      Poco::File subf2(tempf.getpath());
      subf2.copyTo(target.toString());
      return kRSuccess;
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

   cResult gitcopy(std::string repoURL, std::string tag, std::vector<std::string> subfoldersearch, Poco::path dest)
   {
      // checkout repo, copy subfolder if present.


      return cResult();
   }


} // namespace 