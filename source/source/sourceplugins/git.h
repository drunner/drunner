#ifndef __GIT_H
#define __GIT_H

#include <string>
#include <cereal/access.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/map.hpp>

#include "service_paths.h"
#include "sourcecopy.h"
#include "variables.h"

namespace sourcecopy
{
   class git : public sourceplugin
   {
   public:
      git();

      cResult install(std::string imagename, const servicePaths & sp);
      bool pluginmatch(std::string imagename);
      cResult normaliseNames(std::string & imagename, std::string & servicename);

   private:
      cResult copy(std::string nicename, std::string repo, std::string tag, Poco::Path dest) const;
      cResult copy_url(std::string url, std::string repo, std::string tag, Poco::Path dest) const;
   };
}

#endif