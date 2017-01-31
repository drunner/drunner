#include <memory>

#include "Poco/File.h"


#include "utils.h"
#include "utils_docker.h"
#include "dassert.h"
#include "registries.h"


namespace sourceplugins
{ 


   cResult registries::install(std::string & imagename, const servicePaths & sp)
   {
      Poco::Path dest = sp.getPathdService();
      std::string registry, repo, tag;
      cResult r = splitImageName(imagename, registry, repo, tag);
      if (!r.success())
         fatal(r.what());

      logmsg(kLDEBUG, "registry = " + registry + ", repo = " + repo + ", tag = " + tag);
      return copy(registry, repo, tag, dest);
   }



   cResult registries::addregistry(std::string nicename, std::string giturl)
   {
      return kRNotImplemented;
   }

   cResult registries::delregistry(std::string nicename)
   {
      return kRNotImplemented;
   }

   cResult registries::showregistries()
   {
      return kRNotImplemented;
   }

   cResult registries::copy(std::string nicename, std::string repo, std::string tag, Poco::Path dest) const
   {
      return kRNotImplemented;
   }

   // splitImageName
   // splits the imagename into the registry nice name, the repo and the tag.
   cResult registries::splitImageName(std::string imagename, std::string & registry, std::string & repo, std::string & tag) const
   {
      // imagename is of form:  registry/repo:tag
      repo = imagename;

      // extract registry
      registry = "drunner";
      auto pos = repo.find('/');
      if (pos != std::string::npos)
      {
         if (repo.find(':') != std::string::npos &&
            repo.find(':') < pos)
            return cError("Format must be registry/repo:tag");

         if (pos > 0)
            registry = repo.substr(0, pos);
         repo.erase(0, pos + 1);
      }
      
      // extract tag
      tag = "master";
      pos = repo.find(':');
      if (pos != std::string::npos)
      {
         if (pos == 0)
            return cError("Missing repo name in image " + imagename);
         tag = repo.substr(pos + 1);
         repo.erase(repo.begin() + pos, repo.end());
      }
      return kRSuccess;
   }





}

