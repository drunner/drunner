#ifndef __REGISTRIES_H
#define __REGISTRIES_H

#include <string>
#include "service_paths.h"
#include "sourceplugins.h"

namespace sourceplugins
{

   class registry
   {
   public:
      registry() {}
      registry(std::string nn, std::string url) : nicename(nn), giturl(url) {}

      std::string nicename;
      std::string giturl;
   };

   class registries : public sourceplugin
   {
   public:
      registries() {}

      cResult install(std::string & imagename, const servicePaths & sp);

      cResult addregistry(std::string nicename, std::string giturl);
      cResult delregistry(std::string nicename);
      cResult showregistries();

   private:
      cResult copy(std::string nicename, std::string repo, std::string tag, Poco::Path dest) const;
      cResult splitImageName(std::string imagename, std::string & registry, std::string & repo, std::string & tag) const;

      std::vector<registry> mRegistries;
   };


}


#endif