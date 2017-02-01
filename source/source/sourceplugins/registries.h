#ifndef __REGISTRIES_H
#define __REGISTRIES_H

#include <string>
#include <cereal/access.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/map.hpp>

#include "service_paths.h"
#include "sourceplugins.h"
#include "variables.h"

namespace sourceplugins
{
   class registries : public sourceplugin
   {
   public:
      registries();

      cResult install(std::string imagename, const servicePaths & sp);
      bool pluginmatch(std::string imagename);
      cResult normaliseNames(std::string & imagename, std::string & servicename);

      cResult addregistry(std::string nicename, std::string giturl);
      cResult delregistry(std::string nicename);
      cResult showregistries();

   private:
      cResult copy(std::string nicename, std::string repo, std::string tag, Poco::Path dest) const;
      cResult copy_url(std::string url, std::string repo, std::string tag, Poco::Path dest) const;
      cResult splitImageName(std::string imagename, std::string & registry, std::string & repo, std::string & tag) const;

      cResult load();
      cResult save();

      keyVals mData;
      Poco::Path mPath;
   };
}

#endif